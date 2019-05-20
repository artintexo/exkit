#pragma once

#include "listener.h"

#include "securitydesc.h"
#include "sessiondesc.h"

#include <unordered_map>
#include <utility>
#include <vector>

using SessPair = std::pair<ReplHeader, Session>;
using SecrPair = std::pair<ReplHeader, Security>;

class FutInfoRepl : public Listener {
public:
    FutInfoRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr);

    CG_RESULT OnMsgOpen(cg_msg_t *msg) override;
    CG_RESULT OnMsgClose(cg_msg_t *msg) override;
    CG_RESULT OnMsgStreamData(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnBegin(cg_msg_t *msg) override;
    CG_RESULT OnMsgTnCommit(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplLifenum(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplClearDeleted(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2ReplOnline(cg_msg_t *msg) override;

private:
    void ReadSess(cg_msg_streamdata_t *sdata);
    void ReadSecr(cg_msg_streamdata_t *sdata);
    void ClearSess(long tableRev);
    void ClearSecr(long tableRev);
    Session GetBestSession();
    Session GetSession(int sessId);
    std::vector<Security> GetSecurities(int sessId);

    SessionDesc sessDesc;
    SecurityDesc secDesc;
    std::vector<SessPair> pendSess;
    std::vector<SecrPair> pendSec;
    std::vector<SessPair> sessRecs;
    std::vector<SecrPair> secRecs;

    Session curSess;
    std::vector<Security> curSecs;
};
