#include "strategy.h"
#include "holder.h"
#include "logmanager.h"
#include "ordermanager.h"
#include "securitymanager.h"
#include "sessionmanager.h"
#include "subsmanager.h"
#include "timemanager.h"

#include <cstdarg>

void Strategy::Log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    hld->logman->Log(LogLevel::Info, GetTime(), fmt, args);
    va_end(args);
}

long Strategy::GetTime()
{
    return hld->timeman->GetTime();
}

void Strategy::AddReminder(long timestamp)
{
    hld->timeman->AddReminder(timestamp);
}

void Strategy::DelReminder(long timestamp)
{
    hld->timeman->DelReminder(timestamp);
}

Session Strategy::GetSession()
{
    return hld->sessman->GetSession();
}

Security Strategy::GetSecurity(std::string symbol)
{
    return hld->secman->GetSecurity(symbol);
}

void Strategy::Subscribe(Security security)
{
    hld->subsman->Subscribe(security);
}

void Strategy::Unsubscribe(Security security)
{
    hld->subsman->Unsubscribe(security);
}

Order *Strategy::BuyOrder(const Security &security, double price, int qty)
{
    return hld->ordman->MakeOrder(ExecDir::Long, ExecTif::Day, security.secId, security.symbol, security.roundTo, price, qty);
}

Order *Strategy::SellOrder(const Security &security, double price, int qty)
{
    return hld->ordman->MakeOrder(ExecDir::Short, ExecTif::Day, security.secId, security.symbol, security.roundTo, price, qty);
}

void Strategy::Send(Order *order)
{
    hld->ordman->Send(order);
}

void Strategy::Cancel(Order *order)
{
    hld->ordman->Cancel(order);
}

void Strategy::CancelAll(const Security &security, ExecDir dir)
{
    hld->ordman->CancelAll(security, dir);
}

int Strategy::GetPosition(const Security &security)
{
    return hld->posman->GetPosition(security.secId);
}

Statistics Strategy::GetStatistics()
{
    return hld->statman->GetStatistcs();
}
