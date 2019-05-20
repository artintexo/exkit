#include "futaggrrepl.h"
#include "callback.h"
#include "connection.h"
#include "utils.h"

#include "base/holder.h"

#include <algorithm>
#include <set>

FutAggrRepl::FutAggrRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr)
    : Listener(plz, conn->GetConn(), ReplCB, name, settings, openstr, false)
{
}

CG_RESULT FutAggrRepl::OnMsgOpen(cg_msg_t *)
{
    LOGD("%s MSG_OPEN", name.c_str());
    online = false;
    aggrDesc.Deinit();

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

        if (aggrDesc.tableName == name)
            aggrDesc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }

    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgClose(cg_msg_t *msg)
{
    LOGD("%s MSG_CLOSE because %s", name.c_str(), GetCloseReason(msg).c_str());
    online = false;
    aggrDesc.Deinit();

    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgStreamData(cg_msg_t *msg)
{
    cg_msg_streamdata_t *sdata = (cg_msg_streamdata_t *)msg;
    long tableIdx = sdata->msg_index;

    if (aggrDesc.tableIdx == tableIdx)
        ReadAggr(sdata);

    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgTnBegin(cg_msg_t *)
{
    pendAggr.clear();
    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgTnCommit(cg_msg_t *)
{
    std::set<int> secIds;
    bool needSort = false;

    // update
    for (AggrRec &newRec : pendAggr) {
        secIds.insert(newRec.secId);

        auto it = std::find_if(aggrRecs.begin(), aggrRecs.end(),
                               [&](AggrRec *curRec) { return curRec->replId == newRec.replId; });

        if (it != aggrRecs.end()) {
            AggrRec *curRec = *it;

            if (newRec.replAct != 0) {
                RemoveRecord(aggrMap[curRec->secId], curRec->replId);
                delete curRec;
                aggrRecs.erase(it);
                continue;
            }

            if (newRec.secId == curRec->secId) {
                *curRec = newRec;
            } else {
                int secId = curRec->secId;
                RemoveRecord(aggrMap[curRec->secId], curRec->replId);
                *curRec = newRec;
                aggrMap[curRec->secId].push_back(curRec);
                LOGD("%s replId %ld, secId %d -> %d", name.c_str(), curRec->replId, secId, curRec->secId);
            }
        } else {
            AggrRec *rec = new AggrRec(newRec);
            aggrRecs.push_back(rec);
            aggrMap[rec->secId].push_back(rec);
            needSort = true;
        }
    }

    // sort
    if (needSort)
        std::sort(aggrRecs.begin(), aggrRecs.end(), [](AggrRec *ls, AggrRec *rs) { return ls->replId < rs->replId; });

    // send
    if (online) {
        long timestamp = GetSystemTime();
        for (int secId : secIds) {
            if (hld->subsman->HasSecId(secId)) {
                NewObook *ne = GetNewObook(secId);
                ne->obook.timestamp = timestamp;
                hld->evman->PushData(ne);
            }
        }
    }

    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgP2ReplLifenum(cg_msg_t *msg)
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
        aggrDesc.Deinit();
        ClearAggr(CG_MAX_REVISON);
        LOGE("%s ERROR because lifenum", name.c_str());
    }

    lifenum = newlifenum;
    LOGD("%s MSG_P2REPL_LIFENUM changed to %ld", name.c_str(), lifenum);

    return CG_ERR_OK;
}

CG_RESULT FutAggrRepl::OnMsgP2ReplClearDeleted(cg_msg_t *msg)
{
    cg_data_cleardeleted_t *cdel = (cg_data_cleardeleted_t *)msg->data;
    long tableIdx = cdel->table_idx;
    long tableRev = cdel->table_rev;

    if (aggrDesc.tableIdx == tableIdx)
        ClearAggr(tableRev);

    return CG_ERR_OK;
}

void FutAggrRepl::ReadAggr(cg_msg_streamdata_t *sdata)
{
    char *data = (char *)sdata->data;

    AggrRec rec;
    rec.replId = *((int64_t *)(data + aggrDesc.replId));
    rec.replRev = *((int64_t *)(data + aggrDesc.replRev));
    rec.replAct = *((int64_t *)(data + aggrDesc.replAct));

    rec.secId = *((int32_t *)(data + aggrDesc.isinId));
    rec.price = Readbcd(data + aggrDesc.price);
    rec.qty = *((int64_t *)(data + aggrDesc.qty));
    rec.dir = *((int8_t *)(data + aggrDesc.dir));

    pendAggr.push_back(rec);
}

void FutAggrRepl::ClearAggr(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), aggrDesc.tableName.c_str(), tableRev);

    auto it = aggrRecs.begin();
    while (it != aggrRecs.end()) {
        AggrRec *rec = *it;
        if (rec->replRev < tableRev) {
            RemoveRecord(aggrMap[rec->secId], rec->replId);
            delete rec;
            it = aggrRecs.erase(it);
        } else {
            it++;
        }
    }
    std::sort(aggrRecs.begin(), aggrRecs.end(), [](AggrRec *ls, AggrRec *rs) { return ls->replId < rs->replId; });

    if (tableRev == CG_MAX_REVISON)
        aggrDesc.tableRev = 0;
    else
        aggrDesc.tableRev = tableRev;
}

NewObook *FutAggrRepl::GetNewObook(int secId)
{
    NewObook *ne = new NewObook();
    ne->obook.secId = secId;
    ne->obook.bids.reserve(EK_OBOOK_DEPTH);
    ne->obook.asks.reserve(EK_OBOOK_DEPTH);

    auto &vec = aggrMap[secId];
    for (AggrRec *rec : vec) {
        if (rec->qty == 0)
            continue;
        if (rec->dir == 1)
            ne->obook.bids.emplace_back(rec->price, rec->qty);
        else if (rec->dir == 2)
            ne->obook.asks.emplace_back(rec->price, rec->qty);
    }

    std::sort(ne->obook.bids.begin(), ne->obook.bids.end(), [](auto &x, auto &y) { return x.price > y.price; });
    std::sort(ne->obook.asks.begin(), ne->obook.asks.end(), [](auto &x, auto &y) { return x.price < y.price; });

    return ne;
}

void FutAggrRepl::RemoveRecord(std::vector<FutAggrRepl::AggrRec *> &vec, long replId)
{
    auto it = std::remove_if(vec.begin(), vec.end(), [&](AggrRec *rec) { return rec->replId == replId; });
    vec.erase(it, vec.end());
}
