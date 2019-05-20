#pragma once

#include "listener.h"

#include "heartbeatdesc.h"
#include "tradedesc.h"

#include <vector>

class DealsRepl : public Listener {
public:
    DealsRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr);

    CG_RESULT OnMsgOpen(cg_msg_t *msg) override;
    CG_RESULT OnMsgClose(cg_msg_t *msg) override;
    CG_RESULT OnMsgStreamData(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnBegin(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnCommit(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplLifenum(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplClearDeleted(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplOnline(cg_msg_t *msg) override;

private:
    void ReadTrade(cg_msg_streamdata_t *sdata);
    void ReadHbeat(cg_msg_streamdata_t *sdata);
    void ClearTrade(long tableRev);
    void ClearHbeat(long tableRev);

    TradeDesc tradeDesc;
    HeartbeatDesc hbeatDesc;

    std::vector<NewTrade *> pendTrade;
    std::vector<long> pendHbeat;
};
