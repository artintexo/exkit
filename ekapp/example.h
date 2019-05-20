#pragma once

#include "base/strategy.h"

class Example : public Strategy {
public:
    Example();
    void OnStart() override;
    void OnStop() override;
    void OnOrder(Order *order) override;
    void OnObook(Obook &) override;
    void OnTrade(Trade &) override;
    void OnSession(Session &) override;
    void OnSecurity(Security &) override;
    void OnReminder(long) override;

private:
    void OpenLong(double price);
    void OpenShort(double price);
    void ClosePosition();

    Session curses;
    Security cursec;
    bool canTrade = false;
    long startTime = 0;
    long stopTime = 0;
    long lastTime = 0;
    double bestAsk = 0;
    double bestBid = 0;
    std::vector<Trade> trades;

    std::string tradeSymbol = "RTS-6.19";
    int tradeQty = 1;
    long timeBetween = 60'000'000'000;
};
