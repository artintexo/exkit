#pragma once

#include "listener.h"

#include "execdesc.h"

#include <vector>

class FutTradeRepl : public Listener {
public:
    FutTradeRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr);

    CG_RESULT OnMsgOpen(cg_msg_t *msg) override;
    CG_RESULT OnMsgClose(cg_msg_t *msg) override;
    CG_RESULT OnMsgStreamData(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnBegin(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnCommit(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplLifenum(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplClearDeleted(cg_msg_t *msg) override;

private:
    void ReadExec(cg_msg_streamdata_t *sdata);
    void ClearExec(long tableRev);

    ExecDesc execDesc;
    std::vector<Event *> pendExec;
};
