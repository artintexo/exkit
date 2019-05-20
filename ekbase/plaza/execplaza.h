#pragma once

#include "plaza.h"

#include <thread>

class ExecPlaza : public IExecGate {
public:
    ExecPlaza(Plaza *plz);
    ~ExecPlaza();

    bool IsOpened() override;
    bool IsClosed() override;
    void Open() override;
    void Close() override;

    void Send(Order *order) override;
    void Cancel(Order *order) override;
    void CancelAll(const Security &security, ExecDir dir) override;

    Publisher *GetPublisher() { return publisher; }

private:
    void Process();

    Plaza *plz;
    Holder *hld;
    bool processFlag;
    std::thread processThread;
    Connection *execConn;
    Publisher *publisher;
    PubRepl *pubRepl;
    FutTradeRepl *futTradeRepl;
};
