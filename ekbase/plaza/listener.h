#pragma once

#include "plaza.h"

#include <atomic>
#include <chrono>
#include <string>

class Listener {
public:
    Listener(Plaza *plz, cg_conn_t *conn, CG_LISTENER_CB callback,
             std::string name, std::string settings, std::string openstr, bool useReplstate);
    virtual ~Listener();

    inline bool IsOpened() { return state == CG_STATE_ACTIVE; }
    inline bool IsClosed() { return state == CG_STATE_CLOSED; }
    void Open();
    void Close();
    void Process();
    inline bool IsOnline() { return online; }

    virtual CG_RESULT OnMsgOpen(cg_msg_t *msg);
    virtual CG_RESULT OnMsgClose(cg_msg_t *msg);
    virtual CG_RESULT OnMsgData(cg_msg_t *msg);
    virtual CG_RESULT OnMsgP2MQTimeout(cg_msg_t *msg);
    virtual CG_RESULT OnMsgStreamData(cg_msg_t *msg);
    virtual CG_RESULT OnMsgTnBegin(cg_msg_t *msg);
    virtual CG_RESULT OnMsgTnCommit(cg_msg_t *msg);
    virtual CG_RESULT OnMsgP2ReplLifenum(cg_msg_t *msg);
    virtual CG_RESULT OnMsgP2ReplClearDeleted(cg_msg_t *msg);
    virtual CG_RESULT OnMsgP2ReplOnline(cg_msg_t *msg);
    virtual CG_RESULT OnMsgP2ReplReplstate(cg_msg_t *msg);
    virtual CG_RESULT OnDefault(cg_msg_t *msg);

protected:
    std::string GetOpenstr();
    std::string GetCloseReason(cg_msg_t *msg);

    Plaza *plz;
    Holder *hld;
    std::string name;
    std::string settings;
    std::string openstr;
    bool useReplstate;
    cg_listener_t *lsn;
    uint32_t state;

    long lifenum = -1;
    bool online = false;
    std::string replstate = "";

private:
    void TryOpen();
    void TryClose();
    void LogState();
};
