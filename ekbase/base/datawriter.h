#pragma once

#include "base.h"

#include <fstream>

class DataWriter {
public:
    DataWriter(std::string path);
    ~DataWriter();

    void Flush();
    void WriteObook(const Obook &);
    void WriteTrade(const Trade &);
    void WriteSession(const Session &);
    void WriteSecurity(const Security &);

    inline long GetTotal() { return obookNum + tradeNum + sessNum + secNum; }
    inline long GetObookNum() { return obookNum; }
    inline long GetTradeNum() { return tradeNum; }
    inline long GetSessNum() { return sessNum; }
    inline long GetSecNum() { return secNum; }

private:
    std::ofstream file;
    long obookNum;
    long tradeNum;
    long sessNum;
    long secNum;
};
