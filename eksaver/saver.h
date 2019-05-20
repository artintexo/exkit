#pragma once

#include "base/strategy.h"

#include <string>
#include <vector>

class Saver : public Strategy {
public:
    Saver(std::string path, std::vector<std::string> cursyms);
    ~Saver();
    void OnStart() override;
    void OnStop() override;
    void OnObook(Obook &) override;
    void OnTrade(Trade &) override;
    void OnSession(Session &) override;
    void OnSecurity(Security &) override;

private:
    void WriteInitialData();
    void LogState();
    std::vector<std::string> cursyms;
    Session cursess;
    std::vector<Security> cursecs;
    DataWriter *writer;
};
