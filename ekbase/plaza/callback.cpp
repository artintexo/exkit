#include "callback.h"
#include "listener.h"

CG_RESULT ReplCB(cg_conn_t *, cg_listener_t *, cg_msg_t *msg, void *data)
{
    Listener *listener = static_cast<Listener *>(data);

    switch (msg->type) {
    case CG_MSG_STREAM_DATA:
        return listener->OnMsgStreamData(msg);
    case CG_MSG_TN_BEGIN:
        return listener->OnMsgTnBegin(msg);
    case CG_MSG_TN_COMMIT:
        return listener->OnMsgTnCommit(msg);
    case CG_MSG_OPEN:
        return listener->OnMsgOpen(msg);
    case CG_MSG_CLOSE:
        return listener->OnMsgClose(msg);
    case CG_MSG_P2REPL_LIFENUM:
        return listener->OnMsgP2ReplLifenum(msg);
    case CG_MSG_P2REPL_CLEARDELETED:
        return listener->OnMsgP2ReplClearDeleted(msg);
    case CG_MSG_P2REPL_ONLINE:
        return listener->OnMsgP2ReplOnline(msg);
    case CG_MSG_P2REPL_REPLSTATE:
        return listener->OnMsgP2ReplReplstate(msg);
    default:
        return listener->OnDefault(msg);
    }
}

CG_RESULT MQReplyCB(cg_conn_t *, cg_listener_t *, cg_msg_t *msg, void *data)
{
    Listener *listener = static_cast<Listener *>(data);

    switch (msg->type) {
    case CG_MSG_DATA:
        return listener->OnMsgData(msg);
    case CG_MSG_P2MQ_TIMEOUT:
        return listener->OnMsgP2MQTimeout(msg);
    case CG_MSG_OPEN:
        return listener->OnMsgOpen(msg);
    case CG_MSG_CLOSE:
        return listener->OnMsgClose(msg);
    default:
        return listener->OnDefault(msg);
    }
}
