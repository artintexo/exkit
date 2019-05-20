#include "pubrepl.h"
#include "callback.h"
#include "connection.h"
#include "execplaza.h"
#include "publisher.h"
#include "utils.h"

#include "base/holder.h"

PubRepl::PubRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr)
    : Listener(plz, conn->GetConn(), MQReplyCB, name, settings, openstr, false)
{
}

CG_RESULT PubRepl::OnMsgOpen(cg_msg_t *)
{
    LOGD("%s MSG_OPEN", name.c_str());
    online = false;
    msg99Desc.Deinit();
    msg100Desc.Deinit();
    msg101Desc.Deinit();
    msg102Desc.Deinit();
    msg103Desc.Deinit();

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

        if (msg99Desc.tableName == name)
            msg99Desc.Init(tableIdx, mdesc->fields);
        else if (msg100Desc.tableName == name)
            msg100Desc.Init(tableIdx, mdesc->fields);
        else if (msg101Desc.tableName == name)
            msg101Desc.Init(tableIdx, mdesc->fields);
        else if (msg102Desc.tableName == name)
            msg102Desc.Init(tableIdx, mdesc->fields);
        else if (msg103Desc.tableName == name)
            msg103Desc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }

    online = true;
    return CG_ERR_OK;
}

CG_RESULT PubRepl::OnMsgClose(cg_msg_t *msg)
{
    LOGD("%s MSG_CLOSE because %s", name.c_str(), GetCloseReason(msg).c_str());
    online = false;
    msg99Desc.Deinit();
    msg100Desc.Deinit();
    msg101Desc.Deinit();
    msg102Desc.Deinit();
    msg103Desc.Deinit();

    return CG_ERR_OK;
}

CG_RESULT PubRepl::OnMsgData(cg_msg_t *msg)
{
    cg_msg_data_t *mdata = (cg_msg_data_t *)msg;
    long tableIdx = mdata->msg_index;

    if (msg101Desc.tableIdx == tableIdx)
        ReadMsg101(mdata);
    else if (msg102Desc.tableIdx == tableIdx)
        ReadMsg102(mdata);
    else if (msg103Desc.tableIdx == tableIdx)
        ReadMsg103(mdata);
    else if (msg100Desc.tableIdx == tableIdx)
        ReadMsg100(mdata);
    else if (msg99Desc.tableIdx == tableIdx)
        ReadMsg99(mdata);

    return CG_ERR_OK;
}

CG_RESULT PubRepl::OnMsgP2MQTimeout(cg_msg_t *)
{
    LOGW("%s MSG_P2MQ_TIMEOUT", name.c_str());
    return CG_ERR_OK;
}

void PubRepl::ReadMsg99(cg_msg_data_t *mdata)
{
    char *data = (char *)mdata->data;
    int queueSize = *((int32_t *)(data + msg99Desc.queueSize));
    int penaltyRemain = *((int32_t *)(data + msg99Desc.penaltyRemain));

    LOGW("%s read msg 99, queueSize = %d, penaltyRemain = %d", name.c_str(), queueSize, penaltyRemain);
    Publisher *publisher = static_cast<ExecPlaza *>(hld->egate)->GetPublisher();
    publisher->HandleMsg99(penaltyRemain);
}

void PubRepl::ReadMsg100(cg_msg_data_t *mdata)
{
    char *data = (char *)mdata->data;
    int code = *((int32_t *)(data + msg100Desc.code));
    int clientId = mdata->user_id;

    NewExecReport *ne = new NewExecReport();
    ne->report.timestamp = GetSystemTime();
    ne->report.clientId = clientId;
    ne->report.orderId = 0;
    ne->report.status = ExecStatus::Rejected;

    LOGW("%s read msg 100, code = %d, clientId = %d", name.c_str(), code, clientId);
    hld->evman->PushExec(ne);
}

void PubRepl::ReadMsg101(cg_msg_data_t *mdata)
{
    char *data = (char *)mdata->data;
    int code = *((int32_t *)(data + msg101Desc.code));
    //long orderId = *((int64_t *)(data + msg101Desc.orderId));
    int clientId = mdata->user_id;

    if (code == 0) {
        // get accepted from FutTrade orders_log
        return;
    } else {
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = GetSystemTime();
        ne->report.clientId = clientId;
        ne->report.orderId = 0;
        ne->report.status = ExecStatus::Rejected;

        LOGW("%s read msg 101, code = %d, clientId = %d", name.c_str(), code, clientId);
        hld->evman->PushExec(ne);
    }
}

void PubRepl::ReadMsg102(cg_msg_data_t *mdata)
{
    char *data = (char *)mdata->data;
    int code = *((int32_t *)(data + msg102Desc.code));
    //int amount = *((int32_t *)(data + msg102Desc.amount));
    int clientId = mdata->user_id;

    if (code == 0) {
        // get cancelled from FutTrade orders_log
        return;
    } else if (code == 14) {
        // use code == 14 as cancelled
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = GetSystemTime();
        ne->report.clientId = clientId;
        ne->report.orderId = 0;
        ne->report.status = ExecStatus::Cancelled;

        LOGW("%s read msg 102, code = %d, clientId = %d", name.c_str(), code, clientId);
        hld->evman->PushExec(ne);
    } else {
        LOGW("%s read msg 102, code = %d, clientId = %d", name.c_str(), code, clientId);
    }
}

void PubRepl::ReadMsg103(cg_msg_data_t *mdata)
{
    char *data = (char *)mdata->data;
    int code = *((int32_t *)(data + msg103Desc.code));
    int numOrders = *((int32_t *)(data + msg103Desc.numOrders));

    if (code == 0)
        LOGD("%s read msg 103, code = %d, numOrders = %d", name.c_str(), code, numOrders);
    else
        LOGW("%s read msg 103, code = %d, numOrders = %d", name.c_str(), code, numOrders);
}
