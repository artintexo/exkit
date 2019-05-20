#include "publisher.h"
#include "connection.h"
#include "utils.h"

#include "base/context.h"
#include "base/datetime.h"
#include "base/holder.h"
#include "base/order.h"

#include <cassert>
#include <cstring>

static const long NS_PER_SEC = 1000000000;
static const long NS_PER_MS = 1000000;

Publisher::Publisher(Plaza *plz, Connection *conn, std::string name, std::string settings)
    : plz(plz), hld(plz->GetHolder()), name(name), settings(settings)
{
    ResetScheme();

    snprintf(brokerCode, 5, "%s", hld->config.plaza.broker.c_str());
    snprintf(clientCode, 4, "%s", hld->config.plaza.client.c_str());

    tradeMsgMax = hld->config.plaza.tradeMsgMax;
    nontradeMsgMax = hld->config.plaza.nontradeMsgMax;

    tradeMsgPair.first = 0;
    tradeMsgPair.second = 0;

    nontradeMsgPair.first = 0;
    nontradeMsgPair.second = 0;

    P2ERR(cg_pub_new(conn->GetConn(), settings.c_str(), &pub));
}

Publisher::~Publisher()
{
    P2DBG(cg_pub_destroy(pub));
}

void Publisher::Open()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (state != CG_STATE_ACTIVE && Application::GetMainFlag())
        TryOpen();
}

void Publisher::Close()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (state != CG_STATE_CLOSED)
        TryClose();
}

void Publisher::Process()
{
    std::lock_guard<std::mutex> lock(mtx);
    P2ERR(cg_pub_getstate(pub, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_pub_close(pub));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_pub_open(pub, 0));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

bool Publisher::Send(Order *order)
{
    std::lock_guard<std::mutex> lock(mtx);

    // check spam
    bool canSend = CanSendTradeMsg();
    if (canSend == false) {
        LOGW("%s send order, can't send trade message", name.c_str());
        return false;
    }

    // create message
    cg_msg_t *msg = nullptr;
    assert(addOrdDesc.tableIdx != -1);
    CG_RESULT res = cg_pub_msgnew(pub, CG_KEY_INDEX, &(addOrdDesc.tableIdx), &msg);
    if (res != CG_ERR_OK) {
        LOGW("%s send order, cg_pub_msgnew res 0x%X", name.c_str(), res);
        return false;
    }

    // set response id
    cg_msg_data_t *mdata = (cg_msg_data_t *)msg;
    mdata->user_id = order->ClientId();

    // set values
    char *data = (char *)msg->data;

    *((int32_t *)(data + addOrdDesc.clientId)) = order->ClientId();
    snprintf(data + addOrdDesc.brokerCode, 4 + 1, "%s", brokerCode);
    snprintf(data + addOrdDesc.clientCode, 3 + 1, "%s", clientCode);

    *((int32_t *)(data + addOrdDesc.dir)) = (int32_t)(order->Dir());
    *((int32_t *)(data + addOrdDesc.type)) = (int32_t)(order->Tif());
    snprintf(data + addOrdDesc.isin, 25 + 1, "%s", order->Symbol().c_str());
    snprintf(data + addOrdDesc.price, 17 + 1, "%.5f", order->Price());
    *((int32_t *)(data + addOrdDesc.qty)) = order->Qty();

    // post command
    P2DBG(cg_pub_post(pub, msg, CG_PUB_NEEDREPLY));

    // destroy message
    P2DBG(cg_pub_msgfree(pub, msg));

    //
    return true;
}

bool Publisher::Cancel(Order *order)
{
    std::lock_guard<std::mutex> lock(mtx);

    // check spam
    bool canSend = CanSendTradeMsg();
    if (canSend == false) {
        LOGW("%s cancel order, can't send trade message", name.c_str());
        return false;
    }

    // create message
    cg_msg_t *msg = nullptr;
    assert(delOrdDesc.tableIdx != -1);
    CG_RESULT res = cg_pub_msgnew(pub, CG_KEY_INDEX, &(delOrdDesc.tableIdx), &msg);
    if (res != CG_ERR_OK) {
        LOGW("%s cancel order, cg_pub_msgnew res 0x%X", name.c_str(), res);
        return false;
    }

    // set response id
    cg_msg_data_t *mdata = (cg_msg_data_t *)msg;
    mdata->user_id = order->ClientId();

    // set values
    char *data = (char *)msg->data;

    snprintf(data + delOrdDesc.brokerCode, 4 + 1, "%s", brokerCode);
    *((int64_t *)(data + delOrdDesc.orderId)) = order->OrderId();

    // post command
    P2DBG(cg_pub_post(pub, msg, CG_PUB_NEEDREPLY));

    // destroy message
    P2DBG(cg_pub_msgfree(pub, msg));

    //
    return true;
}

bool Publisher::CancelAll(const Security &security, ExecDir dir)
{
    std::lock_guard<std::mutex> lock(mtx);

    // check spam
    bool canSend = CanSendTradeMsg();
    if (canSend == false) {
        LOGW("%s cancel all orders, can't send trade message", name.c_str());
        return false;
    }

    // create message
    cg_msg_t *msg = nullptr;
    assert(delUserOrdDesc.tableIdx != -1);
    CG_RESULT res = cg_pub_msgnew(pub, CG_KEY_INDEX, &(delUserOrdDesc.tableIdx), &msg);
    if (res != CG_ERR_OK) {
        LOGW("%s cancel all orders, cg_pub_msgnew res 0x%X", name.c_str(), res);
        return false;
    }

    // set values
    char *data = (char *)msg->data;

    snprintf(data + delUserOrdDesc.brokerCode, 4 + 1, "%s", brokerCode);
    snprintf(data + delUserOrdDesc.clientCode, 3 + 1, "%s", clientCode);
    snprintf(data + delUserOrdDesc.isin, 25 + 1, "%s", security.symbol.c_str());

    if (dir == ExecDir::Long) {
        *((int32_t *)(data + delUserOrdDesc.buySell)) = 1;
    } else if (dir == ExecDir::Short) {
        *((int32_t *)(data + delUserOrdDesc.buySell)) = 2;
    } else if (dir == ExecDir::Both) {
        *((int32_t *)(data + delUserOrdDesc.buySell)) = 3;
    } else {
        LOGW("%s cancel all orders, wrong exec dir", name.c_str());
        return false;
    }

    // post command
    P2DBG(cg_pub_post(pub, msg, CG_PUB_NEEDREPLY));

    // destroy message
    P2DBG(cg_pub_msgfree(pub, msg));

    //
    return true;
}

void Publisher::HandleMsg99(int penaltyRemain)
{
    std::lock_guard<std::mutex> lock(mtx);

    // calculate next timestamp
    long nextNsTimestamp = GetSystemTime() + (penaltyRemain * NS_PER_MS);
    long nextSecTimestamp = nextNsTimestamp / NS_PER_SEC + 1;

    // update trade counter
    tradeMsgPair.first = nextSecTimestamp;
    tradeMsgPair.second = 1;

    // update non trade counter
    nontradeMsgPair.first = nextSecTimestamp;
    nontradeMsgPair.second = 1;

    LOGW("%s handle msg 99, next time is %s", name.c_str(), GetString(Datetime(nextSecTimestamp * NS_PER_SEC)).c_str());
}

void Publisher::ResetScheme()
{
    addOrdDesc.Deinit();
    delOrdDesc.Deinit();
    delUserOrdDesc.Deinit();
}

void Publisher::ReadScheme()
{
    cg_scheme_desc_t *sdesc;
    CG_RESULT res = cg_pub_getscheme(pub, &sdesc);
    if (res != CG_ERR_OK) {
        ResetScheme();
        LOGE("%s cg_lsn_getscheme res 0x%X", name.c_str(), res);
        return;
    }

    size_t tableIdx = 0;
    cg_message_desc_t *mdesc = sdesc->messages;
    while (mdesc != nullptr) {
        std::string name = (mdesc->name != nullptr) ? mdesc->name : "";

        if (addOrdDesc.tableName == name)
            addOrdDesc.Init(tableIdx, mdesc->fields);
        else if (delOrdDesc.tableName == name)
            delOrdDesc.Init(tableIdx, mdesc->fields);
        else if (delUserOrdDesc.tableName == name)
            delUserOrdDesc.Init(tableIdx, mdesc->fields);

        tableIdx++;
        mdesc = mdesc->next;
    }
}

bool Publisher::CanSendTradeMsg()
{
    long curSecTimestamp = GetSystemTime() / NS_PER_SEC;
    long oldSecTimestamp = tradeMsgPair.first;

    if (curSecTimestamp > oldSecTimestamp) {
        tradeMsgPair.first = curSecTimestamp;
        tradeMsgPair.second = 1;
        return true;
    } else if (curSecTimestamp == oldSecTimestamp) {
        tradeMsgPair.second += 1;
        if (tradeMsgPair.second < tradeMsgMax)
            return true;
        else
            return false;
    } else {
        LOGW("can send trade msg, wrong condition");
        return false;
    }
}

bool Publisher::CanSendNontradeMsg()
{
    long curSecTimestamp = GetSystemTime() / NS_PER_SEC;
    long oldSecTimestamp = nontradeMsgPair.first;

    if (curSecTimestamp > oldSecTimestamp) {
        nontradeMsgPair.first = curSecTimestamp;
        nontradeMsgPair.second = 1;
        return true;
    } else if (curSecTimestamp == oldSecTimestamp) {
        nontradeMsgPair.second += 1;
        if (nontradeMsgPair.second < nontradeMsgMax)
            return true;
        else
            return false;
    } else {
        LOGW("can send non trade msg, wrong condition");
        return false;
    }
}

void Publisher::TryOpen()
{
    P2ERR(cg_pub_getstate(pub, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        ReadScheme();
        LogState();
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_pub_close(pub));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_pub_open(pub, 0));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Publisher::TryClose()
{
    P2ERR(cg_pub_getstate(pub, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        P2DBG(cg_pub_close(pub));
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_pub_close(pub));
        break;
    case CG_STATE_CLOSED:
        LogState();
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Publisher::LogState()
{
    switch (state) {
    case CG_STATE_ACTIVE:
        LOGD("%s ACTIVE", name.c_str());
        break;
    case CG_STATE_ERROR:
        LOGD("%s ERROR", name.c_str());
        break;
    case CG_STATE_CLOSED:
        LOGD("%s CLOSED", name.c_str());
        break;
    case CG_STATE_OPENING:
        LOGD("%s OPENING", name.c_str());
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}
