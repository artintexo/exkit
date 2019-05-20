#include "base.h"
#include "datetime.h"
#include "holder.h"
#include "order.h"
#include "timemanager.h"

#include <chrono>
#include <iomanip>
#include <sstream>

long GetSystemTime()
{
    auto tse = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(tse).count();
}

long GetContextTime(Holder *hld)
{
    return hld->timeman->GetTime();
}

std::vector<std::string> Split(const std::string &str, char delim)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
    while (std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool CheckObook(const Obook &o)
{
    double price = 0;
    for (auto it = o.bids.rbegin(); it != o.bids.rend(); it++) {
        if (it->price > price)
            price = it->price;
        else
            return false;
    }

    price = 0;
    for (auto it = o.asks.begin(); it != o.asks.end(); it++) {
        if (it->price > price)
            price = it->price;
        else
            return false;
    }

    return true;
}

double Round(double value, int roundTo)
{
    switch (roundTo) {
    case 0: {
        int tmp = value * 1;
        return tmp / 1.0;
    }
    case 1: {
        int tmp = value * 10;
        return tmp / 10.0;
    }
    case 2: {
        int tmp = value * 100;
        return tmp / 100.0;
    }
    case 3: {
        int tmp = value * 1000;
        return tmp / 1000.0;
    }
    case 4: {
        int tmp = value * 10000;
        return tmp / 10000.0;
    }
    case 5: {
        int tmp = value * 100000;
        return tmp / 100000.0;
    }
    default:
        return value;
    }
}

std::string GetString(const Datetime &dt)
{
    char buf[32];
    snprintf(buf, 32, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec, dt.ns / 1000000);
    return buf;
}

std::string GetString(const Statistics &s)
{
    std::ostringstream oss;

    oss << "GAll = " << std::fixed << std::setprecision(2) << s.grossAll << ", ";
    oss << "GWin = " << std::fixed << std::setprecision(2) << s.grossWin << ", ";
    oss << "GLoss = " << std::fixed << std::setprecision(2) << s.grossLoss << ", ";
    oss << "PF = " << std::fixed << std::setprecision(2) << s.profitFactor << ", ";
    oss << "NAll = " << s.numAll << ", ";
    oss << "WRate = " << s.winRate << "%, ";
    oss << "AProfit = " << std::fixed << std::setprecision(2) << s.avgProfit << ", ";
    oss << "ATime = " << std::fixed << std::setprecision(2) << s.avgTime / 1000000000 << " sec";

    return oss.str();
}

std::string GetString(const Order &o)
{
    std::ostringstream oss;

    oss << GetString(o.Status()) << ", ";

    double rt = o.Roundtrip() / 1000000.0;
    oss << std::fixed << std::setprecision(2) << rt << " ms, ";
    oss << o.ClientId() << ", ";
    oss << o.OrderId() << ", ";
    oss << GetString(o.Dir()) << ", ";
    oss << o.Symbol() << ", ";
    oss << o.Price() << ", ";
    oss << o.Qty() << ", ";
    if (o.LastQty() != 0)
        oss << "(" << o.LastPrice() << " - " << o.LastQty() << "; " << o.CumQty() << " - " << o.LeavesQty() << ") ";

    /*
    oss << "rt = " << std::fixed << std::setprecision(2) << rt << " ms, ";
    oss << "cli = " << o.ClientId() << ", ";
    oss << "ord = " << o.OrderId() << ", ";
    oss << GetString(o.Dir()) << ", ";
    oss << o.Symbol() << ", ";
    oss << o.Price() << ", ";
    oss << "qty = " << o.Qty() << " (" << o.LastPrice() << " - " << o.LastQty() << "; " << o.CumQty() << " - " << o.LeavesQty() << ")";
    */

    return oss.str();
}

std::string GetString(const Obook &o)
{
    std::ostringstream oss;
    oss << "secId = " << o.secId;

    if (o.bids.empty() == false)
        oss << ", bestBid = " << o.bids[0].price << " - " << o.bids[0].qty;

    if (o.asks.empty() == false)
        oss << ", bestAsk = " << o.asks[0].price << " - " << o.asks[0].qty;

    return oss.str();
}

std::string GetString(const Trade &t)
{
    std::ostringstream oss;
    oss << "isinId = " << t.secId << ", ";
    oss << "price = " << t.price << ", ";
    oss << "qty = " << t.qty;

    return oss.str();
}

std::string GetString(const Session &s)
{
    std::ostringstream oss;
    oss << "sessId = " << s.sessId << ", ";
    oss << "mainState = " << s.mainState << ", ";
    oss << "interState = 0x" << std::hex << s.interState << ", ";
    oss << "allowAccept = " << std::boolalpha << s.allowAccept << ", ";
    oss << "allowCancel = " << std::boolalpha << s.allowCancel;

    if (s.evening.begin != 0) {
        oss << "\n\t\t\t evening: ";
        oss << GetString(Datetime(s.evening.begin)) << " - " << GetString(Datetime(s.evening.end));
    }

    if (s.before.begin != 0) {
        oss << "\n\t\t\t before:  ";
        oss << GetString(Datetime(s.before.begin)) << " - " << GetString(Datetime(s.before.end));
    }

    if (s.inter.begin != 0) {
        oss << "\n\t\t\t inter:   ";
        oss << GetString(Datetime(s.inter.begin)) << " - " << GetString(Datetime(s.inter.end));
    }

    if (s.after.begin != 0) {
        oss << "\n\t\t\t after:   ";
        oss << GetString(Datetime(s.after.begin)) << " - " << GetString(Datetime(s.after.end));
    }

    return oss.str();
}

std::string GetString(const Security &s)
{
    std::ostringstream oss;
    oss << "sessId = " << s.sessId << ", ";
    oss << "secId = " << s.secId << ", ";
    oss << "mainState = " << s.mainState << ", ";
    oss << "allowAccept = " << std::boolalpha << s.allowAccept << ", ";
    oss << "allowCancel = " << std::boolalpha << s.allowCancel << ", ";
    oss << "symbol = " << s.symbol << ", ";
    oss << "roundTo = " << s.roundTo << ", ";
    oss << "minStep = " << s.minStep << ", ";
    oss << "maxPrice = " << s.maxPrice << ", ";
    oss << "minPrice = " << s.minPrice;

    return oss.str();
}

std::string GetString(const ExecStatus &s)
{
    switch (s) {
    case ExecStatus::AcceptReject:
        return "AcceptReject";
    case ExecStatus::CancelReject:
        return "CancelReject";
    case ExecStatus::Created:
        return "Created";
    case ExecStatus::Accepted:
        return "Accepted";
    case ExecStatus::Filled:
        return "Filled";
    case ExecStatus::Cancelled:
        return "Cancelled";
    case ExecStatus::Rejected:
        return "Rejected";
    default:
        return "Unknown";
    }
}

std::string GetString(const ExecDir &d)
{
    switch (d) {
    case ExecDir::Long:
        return "Long";
    case ExecDir::Short:
        return "Short";
    case ExecDir::Both:
        return "Both";
    default:
        return "Unknown";
    }
}

std::string GetString(const ExecTif &t)
{
    switch (t) {
    case ExecTif::Day:
        return "Day";
    case ExecTif::IOC:
        return "IOC";
    case ExecTif::FOK:
        return "FOK";
    default:
        return "Unknown";
    }
}
