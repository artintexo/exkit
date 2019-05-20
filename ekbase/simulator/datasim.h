#pragma once

#include "base/base.h"

#include <filesystem>
#include <thread>
#include <vector>

class DataSim : public IDataGate {
public:
    DataSim(Holder *hld);
    ~DataSim();

    bool IsOpened() override;
    bool IsClosed() override;
    void Open() override;
    void Close() override;

    void Subscribe(const Security &security) override;
    void Unsubscribe(const Security &security) override;

    void Start();
    void Stop();

private:
    void PrepareReaders();
    void ReadInitialData();
    void ReadData();
    std::vector<long> GetDates();
    std::vector<std::string> GetFiles(std::vector<long> dates);

    Holder *hld;
    bool opened;
    long startTime;
    long stopTime;
    std::vector<DataReader *> readers;
    bool readFlag;
    std::thread readThread;
};
