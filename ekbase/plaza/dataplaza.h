#pragma once

#include "plaza.h"

#include <thread>

class DataPlaza : public IDataGate {
public:
    DataPlaza(Plaza *plz);
    ~DataPlaza();

    bool IsOpened() override;
    bool IsClosed() override;
    void Open() override;
    void Close() override;

    void Subscribe(const Security &security) override;
    void Unsubscribe(const Security &security) override;

private:
    void Process();

    Plaza *plz;
    Holder *hld;
    bool processFlag;
    std::thread processThread;
    Connection *dataConn;
    FutInfoRepl *futInfoRepl;
    FutAggrRepl *futAggrRepl;
    DealsRepl *dealsRepl;
};
