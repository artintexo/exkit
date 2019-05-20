#pragma once

#include "listener.h"
#include "publisher.h"

#include "msg99desc.h"
#include "msg100desc.h"
#include "msg101desc.h"
#include "msg102desc.h"
#include "msg103desc.h"

class PubRepl : public Listener {
public:
    PubRepl(Plaza *plz, Connection *conn, std::string name, std::string settings, std::string openstr);

    CG_RESULT OnMsgOpen(cg_msg_t *msg) override;
    CG_RESULT OnMsgClose(cg_msg_t *msg) override;
    CG_RESULT OnMsgData(cg_msg_t *msg) override;
    CG_RESULT OnMsgP2MQTimeout(cg_msg_t *msg) override;

private:
    void ReadMsg99(cg_msg_data_t *mdata);
    void ReadMsg100(cg_msg_data_t *mdata);
    void ReadMsg101(cg_msg_data_t *mdata);
    void ReadMsg102(cg_msg_data_t *mdata);
    void ReadMsg103(cg_msg_data_t *mdata);

    Msg99Desc msg99Desc;
    Msg100Desc msg100Desc;
    Msg101Desc msg101Desc;
    Msg102Desc msg102Desc;
    Msg103Desc msg103Desc;
};
