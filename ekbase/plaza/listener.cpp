#include "listener.h"
#include "utils.h"

#include "base/context.h"
#include "base/holder.h"

Listener::Listener(Plaza *plz, cg_conn_t *conn, CG_LISTENER_CB callback,
                   std::string name, std::string settings, std::string openstr, bool useReplstate)
    : plz(plz), hld(plz->GetHolder()),
      name(name), settings(settings), openstr(openstr), useReplstate(useReplstate), state(0)
{
    P2ERR(cg_lsn_new(conn, settings.c_str(), callback, this, &lsn));
}

Listener::~Listener()
{
    P2DBG(cg_lsn_destroy(lsn));
}

void Listener::Open()
{
    if (state != CG_STATE_ACTIVE && Application::GetMainFlag())
        TryOpen();
}

void Listener::Close()
{
    if (state != CG_STATE_CLOSED)
        TryClose();
}

void Listener::Process()
{
    P2ERR(cg_lsn_getstate(lsn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_lsn_close(lsn));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_lsn_open(lsn, GetOpenstr().c_str()));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

CG_RESULT Listener::OnMsgOpen(cg_msg_t *)
{
    //LOGD("%s MSG_OPEN", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgClose(cg_msg_t *)
{
    //LOGD("%s MSG_CLOSE", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgData(cg_msg_t *)
{
    //LOGD("%s MSG_DATA", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgP2MQTimeout(cg_msg_t *)
{
    //LOGD("%s MSG_P2MQ_TIMEOUT", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgStreamData(cg_msg_t *)
{
    //LOGD("%s MSG_STREAM_DATA", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgTnBegin(cg_msg_t *)
{
    //LOGD("%s MSG_TN_BEGIN", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgTnCommit(cg_msg_t *)
{
    //LOGD("%s MSG_TN_COMMIT", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgP2ReplLifenum(cg_msg_t *)
{
    //LOGD("%s MSG_P2REPL_LIFENUM", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgP2ReplClearDeleted(cg_msg_t *)
{
    //LOGD("%s MSG_P2REPL_CLEARDELETED", name.c_str());
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgP2ReplOnline(cg_msg_t *)
{
    LOGD("%s MSG_P2REPL_ONLINE", name.c_str());
    online = true;
    return CG_ERR_OK;
}

CG_RESULT Listener::OnMsgP2ReplReplstate(cg_msg_t *msg)
{
    LOGD("%s MSG_P2REPL_REPLSTATE", name.c_str());
    replstate = std::string((char *)msg->data);
    return CG_ERR_OK;
}

CG_RESULT Listener::OnDefault(cg_msg_t *msg)
{
    LOGW("%s DEFAULT 0x%X", name.c_str(), msg->type);
    return CG_ERR_OK;
}

std::string Listener::GetOpenstr()
{
    if (useReplstate && replstate.empty() == false)
        return std::string("replstate=") + replstate;
    else
        return openstr;
}

std::string Listener::GetCloseReason(cg_msg_t *msg)
{
    int reason = *((int *)(msg->data));

    switch (reason) {
    case CG_REASON_UNDEFINED:
        return "CG_REASON_UNDEFINED";
    case CG_REASON_USER:
        return "CG_REASON_USER";
    case CG_REASON_ERROR:
        return "CG_REASON_ERROR";
    case CG_REASON_DONE:
        return "CG_REASON_DONE";
    case CG_REASON_SNAPSHOT_DONE:
        return "CG_REASON_SNAPSHOT_DONE";
    default:
        return "UNKNOWN";
    }
}

void Listener::TryOpen()
{
    P2ERR(cg_lsn_getstate(lsn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        LogState();
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_lsn_close(lsn));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_lsn_open(lsn, GetOpenstr().c_str()));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Listener::TryClose()
{
    P2ERR(cg_lsn_getstate(lsn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        P2DBG(cg_lsn_close(lsn));
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_lsn_close(lsn));
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

void Listener::LogState()
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
