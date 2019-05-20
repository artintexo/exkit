#include "dealsrepl.h"
#include "callback.h"
#include "connection.h"
#include "utils.h"

#include "base/holder.h"

constexpr long HbeatAdditional = 10000000;

DealsRepl::DealsRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr)
    : Listener(plz, conn->GetConn(), ReplCB, name, settings, openstr, false)
{
}

CG_RESULT DealsRepl::OnMsgOpen(cg_msg_t *)
{
    LOGD("%s MSG_OPEN", name.c_str());
    online = false;
    tradeDesc.Deinit();
    hbeatDesc.Deinit();

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

        if (tradeDesc.tableName == name)
            tradeDesc.Init(tableIdx, mdesc->fields);
        else if (hbeatDesc.tableName == name)
            hbeatDesc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgClose(cg_msg_t *msg)
{
    LOGD("%s MSG_CLOSE because %s", name.c_str(), GetCloseReason(msg).c_str());
    online = false;
    tradeDesc.Deinit();
    hbeatDesc.Deinit();

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgStreamData(cg_msg_t *msg)
{
    cg_msg_streamdata_t *sdata = (cg_msg_streamdata_t *)msg;
    long tableIdx = sdata->msg_index;

    if (tradeDesc.tableIdx == tableIdx && online)
        ReadTrade(sdata);
    else if (hbeatDesc.tableIdx == tableIdx && online)
        ReadHbeat(sdata);

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgTnBegin(cg_msg_t *)
{
    //LOGD("%s MSG_TN_BEGIN", name.c_str());
    pendTrade.clear();
    pendHbeat.clear();

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgTnCommit(cg_msg_t *)
{
    if (online == true) {
        long timestamp = GetSystemTime();

        for (NewTrade *ne : pendTrade) {
            ne->trade.timestamp = timestamp;
            hld->evman->PushData(ne);
        }

        if (pendHbeat.empty() == false) {
            NewHeartbeat *ne = new NewHeartbeat();
            ne->heartbeat.timestamp = timestamp + HbeatAdditional;
            hld->evman->PushData(ne);
        }
    } else {
        for (NewTrade *ne : pendTrade)
            delete ne;
    }

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgP2ReplLifenum(cg_msg_t *msg)
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
        tradeDesc.Deinit();
        hbeatDesc.Deinit();
        ClearTrade(CG_MAX_REVISON);
        ClearHbeat(CG_MAX_REVISON);
        LOGE("%s ERROR because lifenum", name.c_str());
    }

    lifenum = newlifenum;
    LOGD("%s MSG_P2REPL_LIFENUM changed to %ld", name.c_str(), lifenum);

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgP2ReplClearDeleted(cg_msg_t *msg)
{
    cg_data_cleardeleted_t *cdel = (cg_data_cleardeleted_t *)msg->data;
    long tableIdx = cdel->table_idx;
    long tableRev = cdel->table_rev;

    if (tradeDesc.tableIdx == tableIdx)
        ClearTrade(tableRev);
    else if (hbeatDesc.tableIdx == tableIdx)
        ClearHbeat(tableRev);

    return CG_ERR_OK;
}

CG_RESULT DealsRepl::OnMsgP2ReplOnline(cg_msg_t *)
{
    LOGD("%s MSG_P2REPL_ONLINE", name.c_str());
    online = true;

    NewHeartbeat *ne = new NewHeartbeat();
    ne->heartbeat.timestamp = GetSystemTime() + HbeatAdditional;
    hld->evman->PushData(ne);

    return CG_ERR_OK;
}

void DealsRepl::ReadTrade(cg_msg_streamdata_t *sdata)
{
    char *data = (char *)sdata->data;

    long replRev = *((int64_t *)(data + tradeDesc.replRev));
    if (replRev < tradeDesc.tableRev)
        return;

    long replAct = *((int64_t *)(data + tradeDesc.replAct));
    if (replAct != 0)
        return;

    bool nosystem = *((int8_t *)(data + tradeDesc.nosystem)) == 0 ? false : true;
    if (nosystem)
        return;

    int isinId = *((int32_t *)(data + tradeDesc.isinId));
    if (hld->subsman->HasSecId(isinId) == false)
        return;

    double price = Readbcd((void *)(data + tradeDesc.price));
    int qty = *((int64_t *)(data + tradeDesc.qty));
    long moment = *((uint64_t *)(data + tradeDesc.moment));

    NewTrade *ne = new NewTrade();
    ne->trade.timestamp = 0;
    ne->trade.moment = moment;
    ne->trade.secId = isinId;
    ne->trade.price = price;
    ne->trade.qty = qty;

    pendTrade.push_back(ne);
}

void DealsRepl::ReadHbeat(cg_msg_streamdata_t *)
{
    pendHbeat.push_back(0);
}

void DealsRepl::ClearTrade(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), tradeDesc.tableName.c_str(), tableRev);

    if (tableRev == CG_MAX_REVISON)
        tradeDesc.tableRev = 0;
    else
        tradeDesc.tableRev = tableRev;
}

void DealsRepl::ClearHbeat(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), hbeatDesc.tableName.c_str(), tableRev);

    if (tableRev == CG_MAX_REVISON)
        hbeatDesc.tableRev = 0;
    else
        hbeatDesc.tableRev = tableRev;
}
