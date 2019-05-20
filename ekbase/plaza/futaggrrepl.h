#pragma once

#include "listener.h"

#include "aggrdesc.h"

#include <unordered_map>
#include <vector>

class FutAggrRepl : public Listener {
public:
    FutAggrRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr);

    CG_RESULT OnMsgOpen(cg_msg_t *msg) override;
    CG_RESULT OnMsgClose(cg_msg_t *msg) override;
    CG_RESULT OnMsgStreamData(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnBegin(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnCommit(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplLifenum(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplClearDeleted(cg_msg_t *msg) override;

private:
    struct AggrRec {
        long replId;
        long replRev;
        long replAct;
        int secId;
        double price;
        int qty;
        int dir;
    };

private:
    void ReadAggr(cg_msg_streamdata_t *sdata);
    void ClearAggr(long tableRev);
    NewObook *GetNewObook(int secId);
    void RemoveRecord(std::vector<AggrRec *> &vec, long replId);

    AggrDesc aggrDesc;
    std::vector<AggrRec> pendAggr;
    std::vector<AggrRec *> aggrRecs;
    std::unordered_map<int, std::vector<AggrRec *>> aggrMap;
};
