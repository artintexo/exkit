#include "futinforepl.h"
#include "callback.h"
#include "connection.h"
#include "utils.h"

#include "base/holder.h"

#include <algorithm>

FutInfoRepl::FutInfoRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr)
    : Listener(plz, conn->GetConn(), ReplCB, name, settings, openstr, true)
{
}

CG_RESULT FutInfoRepl::OnMsgOpen(cg_msg_t *)
{
    LOGD("%s MSG_OPEN", name.c_str());
    online = false;
    sessDesc.Deinit();
    secDesc.Deinit();

    cg_scheme_desc_t *sdesc;
    CG_RESULT res = cg_lsn_getscheme(lsn, &sdesc);
    if (res != CG_ERR_OK) {
        LOGE("%s cg_lsn_getscheme res 0x%X", name.c_str(), res);
        return res;
    }

    size_t tableIdx = 0;
    cg_message_desc_t *mdesc = sdesc->messages;
    while (mdesc != nullptr) {
        std::string name = (mdesc->name != nullptr) ? mdesc->name : "";

        if (sessDesc.tableName == name)
            sessDesc.Init(tableIdx, mdesc->fields);
        else if (secDesc.tableName == name)
            secDesc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgClose(cg_msg_t *msg)
{
    LOGD("%s MSG_CLOSE because %s", name.c_str(), GetCloseReason(msg).c_str());
    online = false;
    sessDesc.Deinit();
    secDesc.Deinit();

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgStreamData(cg_msg_t *msg)
{
    cg_msg_streamdata_t *sdata = (cg_msg_streamdata_t *)msg;
    long tableIdx = sdata->msg_index;

    if (sessDesc.tableIdx == tableIdx)
        ReadSess(sdata);
    else if (secDesc.tableIdx == tableIdx)
        ReadSecr(sdata);

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgTnBegin(cg_msg_t *)
{
    //LOGD("%s MSG_TN_BEGIN", name.c_str());
    pendSess.clear();
    pendSec.clear();
    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgTnCommit(cg_msg_t *)
{
    //LOGD("%s MSG_TN_COMMIT", name.c_str());
    //LOGD("pendSess = %ld, pendSecr = %ld", pendSess.size(), pendSecr.size());

    // sessions
    for (SessPair &pair : pendSess) {
        auto it = std::find_if(sessRecs.begin(), sessRecs.end(),
                               [&](SessPair &exist) { return pair.first.replId == exist.first.replId; });
        if (it != sessRecs.end())
            *it = pair;
        else
            sessRecs.push_back(pair);
    }
    std::sort(sessRecs.begin(), sessRecs.end(),
              [&](SessPair &ls, SessPair &rs) { return ls.first.replId < rs.first.replId; });

    // securities
    for (SecrPair &pair : pendSec) {
        auto it = std::find_if(secRecs.begin(), secRecs.end(),
                               [&](SecrPair &exist) { return pair.first.replId == exist.first.replId; });
        if (it != secRecs.end())
            *it = pair;
        else
            secRecs.push_back(pair);
    }
    std::sort(secRecs.begin(), secRecs.end(),
              [&](SecrPair &ls, SecrPair &rs) { return ls.first.replId < rs.first.replId; });

    // send
    if (online) {
        auto timestamp = GetSystemTime();

        if (pendSess.empty() == false) {
            Session newSess = GetBestSession();
            // check if session updated
            if (curSess != newSess) {
                curSess = newSess;

                // send
                NewSession *ns = new NewSession();
                ns->session = curSess;
                ns->session.timestamp = timestamp;
                hld->evman->PushData(ns);
            }
        }

        if (pendSec.empty() == false) {
            std::vector<Security> newSecs = GetSecurities(curSess.sessId);
            std::vector<Security> needSend;

            // update current securities
            auto itCur = curSecs.begin();
            while (itCur++ != curSecs.end()) {
                auto itNew = std::find_if(newSecs.begin(), newSecs.end(),
                                          [&](const Security &newSec) { return newSec.symbol == itCur->symbol; });

                if (itNew != newSecs.end()) {
                    // check if security updated
                    if (*itCur != *itNew) {
                        *itCur = *itNew;
                        needSend.push_back(*itCur);
                    }
                }
            }

            // remove securities
            auto it = std::remove_if(curSecs.begin(), curSecs.end(),
                                     [&](const Security &curSec) { return curSec.sessId != curSess.sessId; });
            curSecs.erase(it, curSecs.end());

            // send
            for (auto sec : needSend) {
                NewSecurity *ns = new NewSecurity();
                ns->security = sec;
                ns->security.timestamp = timestamp;
                hld->evman->PushData(ns);
            }
        }
    }

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgP2ReplLifenum(cg_msg_t *msg)
{
    long newlifenum = -1;

    if (msg->data_size == 8) {
        newlifenum = *((int64_t *)msg->data);
    } else if (msg->data_size == 4) {
        newlifenum = *((int32_t *)msg->data);
    } else {
        LOGW("%s MSG_P2REPL_LIFENUM unknown size %ld", name.c_str(), msg->data_size);
        return CG_ERR_OK;
    }

    if (lifenum != -1 && newlifenum != -1) {
        sessDesc.Deinit();
        secDesc.Deinit();
        ClearSess(CG_MAX_REVISON);
        ClearSecr(CG_MAX_REVISON);
        LOGE("%s ERROR because lifenum", name.c_str());
    }

    lifenum = newlifenum;
    LOGD("%s MSG_P2REPL_LIFENUM changed to %ld", name.c_str(), lifenum);

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgP2ReplClearDeleted(cg_msg_t *msg)
{
    cg_data_cleardeleted_t *cdel = (cg_data_cleardeleted_t *)msg->data;
    long tableIdx = cdel->table_idx;
    long tableRev = cdel->table_rev;

    if (sessDesc.tableIdx == tableIdx)
        ClearSess(tableRev);
    else if (secDesc.tableIdx == tableIdx)
        ClearSecr(tableRev);

    return CG_ERR_OK;
}

CG_RESULT FutInfoRepl::OnMsgP2ReplOnline(cg_msg_t *)
{
    LOGD("%s MSG_P2REPL_ONLINE", name.c_str());
    online = true;

    if (sessRecs.empty()) {
        LOGW("%s has no sessions", name.c_str());
        return CG_ERR_OK;
    } else {
        Session sess = GetBestSession();
        auto secs = GetSecurities(sess.sessId);
        auto timestamp = GetSystemTime();

        NewSession *ns = new NewSession();
        ns->session = sess;
        ns->session.timestamp = timestamp;
        hld->evman->PushData(ns);

        for (auto sec : secs) {
            NewSecurity *ns = new NewSecurity();
            ns->security = sec;
            ns->security.timestamp = timestamp;
            hld->evman->PushData(ns);
        }

        curSess = sess;
        curSecs = secs;
    }

    return CG_ERR_OK;
}

void FutInfoRepl::ReadSess(cg_msg_streamdata_t *sdata)
{
    char *data = (char *)sdata->data;

    ReplHeader rh = {};
    rh.replId = *((int64_t *)(data + sessDesc.replId));
    rh.replRev = *((int64_t *)(data + sessDesc.replRev));
    rh.replAct = *((int64_t *)(data + sessDesc.replAct));

    Session sess = {};
    sess.sessId = *((int32_t *)(data + sessDesc.sessId));
    sess.mainState = *((int32_t *)(data + sessDesc.mainState));
    sess.interState = *((int32_t *)(data + sessDesc.interState));

    /*
    3 Сессия принудительно завершена. Нельзя ставить и удалять заявки.
    4 Сессия завершена по времени. Нельзя ставить и удалять заявки.
    2 Приостановка торгов по всем инструментам. Нельзя ставить заявки, но можно удалять.
    0 Сессия назначена. Нельзя ставить заявки, но можно удалять.
    1 Сессия идет. Можно ставить и удалять заявки.
    */
    if (sess.mainState == 3 || sess.mainState == 4) {
        sess.allowAccept = false;
        sess.allowCancel = false;
    } else if (sess.mainState == 2 || sess.mainState == 0) {
        sess.allowAccept = false;
        sess.allowCancel = true;
    } else if (sess.mainState == 1) {
        sess.allowAccept = true;
        sess.allowCancel = true;
    }

    bool eveOn = *((int8_t *)(data + sessDesc.eveOn)) == 0 ? false : true;
    if (eveOn) {
        sess.evening.begin = Convert((cg_time_t *)(data + sessDesc.eveBegin));
        sess.evening.end = Convert((cg_time_t *)(data + sessDesc.eveEnd));
    } else {
        LOGW("%s no evening for sessId = %d", name.c_str(), sess.sessId);
        return;
    }

    /*
    bool monOn = *((int8_t *)(data + sessDesc.monOn)) == 0 ? false : true;
    if (monOn) {
        sess.morning.begin = Convert((cg_time_t *)(data + sessDesc.monBegin));
        sess.morning.end = Convert((cg_time_t *)(data + sessDesc.monEnd));
    }
    */

    sess.inter.begin = Convert((cg_time_t *)(data + sessDesc.interBegin));
    sess.inter.end = Convert((cg_time_t *)(data + sessDesc.interEnd));

    sess.before.begin = Convert((cg_time_t *)(data + sessDesc.mainBegin));
    sess.before.end = sess.inter.begin;

    sess.after.begin = sess.inter.end;
    sess.after.end = Convert((cg_time_t *)(data + sessDesc.mainEnd));

    pendSess.emplace_back(rh, sess);
}

void FutInfoRepl::ReadSecr(cg_msg_streamdata_t *sdata)
{
    char *data = (char *)sdata->data;

    ReplHeader rh = {};
    rh.replId = *((int64_t *)(data + secDesc.replId));
    rh.replRev = *((int64_t *)(data + secDesc.replRev));
    rh.replAct = *((int64_t *)(data + secDesc.replAct));

    Security sec = {};
    sec.symbol = Readstr("c25", (void *)(data + secDesc.isin));
    sec.sessId = *((int32_t *)(data + secDesc.sessId));
    sec.secId = *((int32_t *)(data + secDesc.isinId));
    sec.mainState = *((int32_t *)(data + secDesc.state));

    /*
    3 Сессия по этому инструменту принудительно завершена. Нельзя ставить и удалять заявки по этому инструменту.
    4 Сессия по этому инструменту завершена по времени. Нельзя ставить и удалять заявки по этому инструменту.
    2 Приостановка торгов по всем инструментам. Нельзя ставить заявки, но можно удалять.
    0 Сессия по этому инструменту назначена. Нельзя ставить заявки, но можно удалять по этому инструменту.
    5 Приостановка торгов по этому инструменту. Нельзя ставить заявки, но можно удалять по этому инструменту.
    1 Сессия по этому инструменту идет. Можно ставить и удалять заявки по этому инструменту.
    */
    if (sec.mainState == 3 || sec.mainState == 4) {
        sec.allowAccept = false;
        sec.allowCancel = false;
    } else if (sec.mainState == 2 || sec.mainState == 0 || sec.mainState == 5) {
        sec.allowAccept = false;
        sec.allowCancel = true;
    } else if (sec.mainState == 1) {
        sec.allowAccept = true;
        sec.allowCancel = true;
    }

    sec.roundTo = *((int32_t *)(data + secDesc.roundto));
    sec.minStep = Readbcd((void *)(data + secDesc.minStep));

    double lastClQuote = Readbcd((void *)(data + secDesc.lastClQuote));

    double limitUp = Readbcd((void *)(data + secDesc.limitUp));
    sec.maxPrice = Round(lastClQuote + limitUp, sec.roundTo);

    double limitDown = Readbcd((void *)(data + secDesc.limitDown));
    sec.minPrice = Round(lastClQuote - limitDown, sec.roundTo);

    //add to pending
    pendSec.emplace_back(rh, sec);
}

void FutInfoRepl::ClearSess(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), sessDesc.tableName.c_str(), tableRev);

    auto it = std::remove_if(sessRecs.begin(), sessRecs.end(),
                             [=](SessPair &pair) { return pair.first.replRev < tableRev; });
    sessRecs.erase(it, sessRecs.end());

    if (tableRev == CG_MAX_REVISON)
        sessDesc.tableRev = 0;
    else
        sessDesc.tableRev = tableRev;
}

void FutInfoRepl::ClearSecr(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), secDesc.tableName.c_str(), tableRev);

    auto it = std::remove_if(secRecs.begin(), secRecs.end(),
                             [=](SecrPair &pair) { return pair.first.replRev < tableRev; });
    secRecs.erase(it, secRecs.end());

    if (tableRev == CG_MAX_REVISON)
        secDesc.tableRev = 0;
    else
        secDesc.tableRev = tableRev;
}

Session FutInfoRepl::GetBestSession()
{
    Session best = {};
    best.sessId = -1;
    best.mainState = 7;

    for (auto &pair : sessRecs) {
        Session next = pair.second;

        if (next.mainState == 1 || next.mainState == 2) {
            best = next;
            break;
        }

        if (best.mainState > 2) {
            // if next session is ready
            if (next.mainState < 3) {
                best = next;
                continue;
            }
            // if has no ready session
            if (next.sessId > best.sessId) {
                best = next;
                continue;
            }
        }
    }

    return best;
}

Session FutInfoRepl::GetSession(int sessId)
{
    for (SessPair &pair : sessRecs) {
        Session sess = pair.second;
        if (sess.sessId == sessId)
            return sess;
    }

    LOGW("%s has no session for sessId = %d", name.c_str(), sessId);
    return Session();
}

std::vector<Security> FutInfoRepl::GetSecurities(int sessId)
{
    std::vector<Security> secs;
    for (auto &pair : secRecs) {
        Security sec = pair.second;

        if (sec.sessId == sessId)
            secs.push_back(sec);
    }

    if (secs.empty())
        LOGW("%s has no securities for sessId = %d", name.c_str(), sessId);

    return secs;
}
