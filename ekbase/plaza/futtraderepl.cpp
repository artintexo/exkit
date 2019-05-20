#include "futtraderepl.h"
#include "callback.h"
#include "connection.h"
#include "utils.h"

#include "base/holder.h"

FutTradeRepl::FutTradeRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr)
    : Listener(plz, conn->GetConn(), ReplCB, name, settings, openstr, true)
{
}

CG_RESULT FutTradeRepl::OnMsgOpen(cg_msg_t *)
{
    LOGD("%s MSG_OPEN", name.c_str());
    online = false;
    execDesc.Deinit();

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

        if (execDesc.tableName == name)
            execDesc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }

    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgClose(cg_msg_t *msg)
{
    LOGD("%s MSG_CLOSE because %s", name.c_str(), GetCloseReason(msg).c_str());
    online = false;
    execDesc.Deinit();

    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgStreamData(cg_msg_t *msg)
{
    cg_msg_streamdata_t *sdata = (cg_msg_streamdata_t *)msg;
    long tableIdx = sdata->msg_index;

    if (execDesc.tableIdx == tableIdx)
        ReadExec(sdata);

    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgTnBegin(cg_msg_t *)
{
    //LOGD("%s MSG_TN_BEGIN", name.c_str());
    pendExec.clear();
    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgTnCommit(cg_msg_t *)
{
    long timestamp = GetSystemTime();

    for (Event *event : pendExec) {
        if (event->type == EventType::NewExecReport) {
            NewExecReport *ne = static_cast<NewExecReport *>(event);
            ne->report.timestamp = timestamp;
            hld->evman->PushExec(ne);
        } else if (event->type == EventType::NewExecTrade) {
            NewExecTrade *ne = static_cast<NewExecTrade *>(event);
            ne->trade.timestamp = timestamp;
            hld->evman->PushExec(ne);
        }
    }

    pendExec.clear();
    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgP2ReplLifenum(cg_msg_t *msg)
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
        execDesc.Deinit();
        ClearExec(CG_MAX_REVISON);
        LOGE("%s ERROR because lifenum", name.c_str());
    }

    lifenum = newlifenum;
    LOGD("%s MSG_P2REPL_LIFENUM changed to %ld", name.c_str(), lifenum);

    return CG_ERR_OK;
}

CG_RESULT FutTradeRepl::OnMsgP2ReplClearDeleted(cg_msg_t *msg)
{
    cg_data_cleardeleted_t *cdel = (cg_data_cleardeleted_t *)msg->data;
    long tableIdx = cdel->table_idx;
    long tableRev = cdel->table_rev;

    if (execDesc.tableIdx == tableIdx)
        ClearExec(tableRev);

    return CG_ERR_OK;
}

void FutTradeRepl::ReadExec(cg_msg_streamdata_t *sdata)
{
    char *data = (char *)sdata->data;

    long replRev = *((int64_t *)(data + execDesc.replRev));
    if (replRev < execDesc.tableRev) {
        LOGE("%s read exec, because replRev", name.c_str());
        return;
    }

    long replAct = *((int64_t *)(data + execDesc.replAct));
    if (replAct != 0) {
        LOGE("%s read exec, because replAct", name.c_str());
        return;
    }

    int clientId = *((int32_t *)(data + execDesc.clientId));
    long orderId = *((int64_t *)(data + execDesc.orderId));
    if (hld->ordman->HasOrder(clientId) == false)
        return;

    int action = *((int8_t *)(data + execDesc.action));
    if (action == 2) {
        NewExecTrade *ne = new NewExecTrade();
        ne->trade.timestamp = 0;
        ne->trade.clientId = clientId;
        ne->trade.orderId = orderId;
        ne->trade.price = Readbcd(data + execDesc.fillPrice);
        ne->trade.qty = *((int64_t *)(data + execDesc.fillQty));
        pendExec.push_back(ne);
    } else if (action == 1) {
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = 0;
        ne->report.clientId = clientId;
        ne->report.orderId = orderId;
        ne->report.status = ExecStatus::Accepted;
        pendExec.push_back(ne);
    } else if (action == 0) {
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = 0;
        ne->report.clientId = clientId;
        ne->report.orderId = orderId;
        ne->report.status = ExecStatus::Cancelled;
        pendExec.push_back(ne);
    } else {
        LOGE("%s read exec, wrong action = %d", name.c_str(), action);
        return;
    }
}

void FutTradeRepl::ClearExec(long tableRev)
{
    LOGD("%s MSG_P2REPL_CLEARDELETED %s got %ld", name.c_str(), execDesc.tableName.c_str(), tableRev);

    if (tableRev == CG_MAX_REVISON)
        execDesc.tableRev = 0;
    else
        execDesc.tableRev = tableRev;
}
