#pragma once

#include "plaza.h"

#include "futaddorderdesc.h"
#include "futdelorderdesc.h"
#include "futdeluserordersdesc.h"

#include <chrono>
#include <mutex>
#include <string>

class Publisher {
public:
    Publisher(Plaza *plz, Connection *conn, std::string name, std::string settings);
    ~Publisher();

    inline bool IsOpened() { return state == CG_STATE_ACTIVE; }
    inline bool IsClosed() { return state == CG_STATE_CLOSED; }
    void Open();
    void Close();
    void Process();

    bool Send(Order *order);
    bool Cancel(Order *order);
    bool CancelAll(const Security &security, ExecDir dir);
    void HandleMsg99(int penaltyRemain);

private:
    void ResetScheme();
    void ReadScheme();
    bool CanSendTradeMsg();
    bool CanSendNontradeMsg();
    void TryOpen();
    void TryClose();
    void LogState();

    Plaza *plz;
    Holder *hld;
    std::string name;
    std::string settings;
    cg_publisher_t *pub;
    uint32_t state;

    FutAddOrderDesc addOrdDesc;
    FutDelOrderDesc delOrdDesc;
    FutDelUserOrdersDesc delUserOrdDesc;

    std::mutex mtx;
    char brokerCode[5];
    char clientCode[4];
    int tradeMsgMax;
    int nontradeMsgMax;
    std::pair<long, int> tradeMsgPair;
    std::pair<long, int> nontradeMsgPair;
};
