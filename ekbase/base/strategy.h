#pragma once

#include "base.h"
#include "datetime.h"
#include "order.h"

class Strategy : public IStrategy {
public:
    virtual ~Strategy() {}
    virtual void OnStart() override {}
    virtual void OnStop() override {}
    virtual void OnOrder(Order *) override {}
    virtual void OnObook(Obook &) override {}
    virtual void OnTrade(Trade &) override {}
    virtual void OnSession(Session &) override {}
    virtual void OnSecurity(Security &) override {}
    virtual void OnReminder(long) override {}

protected:
    void Log(const char *fmt, ...);

    long GetTime();
    void AddReminder(long timestamp);
    void DelReminder(long timestamp);

    Session GetSession();
    Security GetSecurity(std::string symbol);
    void Subscribe(Security security);
    void Unsubscribe(Security security);

    Order *BuyOrder(const Security &security, double price, int qty);
    Order *SellOrder(const Security &security, double price, int qty);
    void Send(Order *order);
    void Cancel(Order *order);
    void CancelAll(const Security &security, ExecDir dir);

    int GetPosition(const Security &security);
    Statistics GetStatistics();

private:
    friend class Holder;
    Holder *hld;
};
