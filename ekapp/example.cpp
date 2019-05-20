#include "example.h"

#include <algorithm>
#include <cassert>

Example::Example()
{
}

void Example::OnStart()
{
    Log("Example starting...");

    Datetime dt = GetTime();
    startTime = Datetime(dt.year, dt.month, dt.day, 10, 10).timestamp;
    stopTime = Datetime(dt.year, dt.month, dt.day, 18, 40).timestamp;
    AddReminder(startTime);
    AddReminder(stopTime);
    Log("Open time = %s", GetString(Datetime(startTime)).c_str());
    Log("Close time = %s", GetString(Datetime(stopTime)).c_str());

    curses = GetSession();
    cursec = GetSecurity(tradeSymbol);
    Subscribe(cursec);
}

void Example::OnStop()
{
    CancelAll(cursec, ExecDir::Both);
    Unsubscribe(cursec);
    Log("Example stopped");
}

void Example::OnOrder(Order *order)
{
    Log("%s", GetString(*order).c_str());
    if (order->IsFill())
        Log("Position = %d, %s", GetPosition(cursec), GetString(GetStatistics()).c_str());
}

void Example::OnObook(Obook &obook)
{
    if (obook.asks.empty() == false)
        bestAsk = obook.asks[0].price;

    if (obook.bids.empty() == false)
        bestBid = obook.bids[0].price;
}

void Example::OnTrade(Trade &trade)
{
    trades.push_back(trade);

    if (trade.timestamp - lastTime > timeBetween)
        lastTime = trade.timestamp;
    else
        return;

    if (canTrade == false || trades.size() < 2)
        return;

    Trade curTrade = trades[trades.size() - 1];
    Trade prvTrade = trades[trades.size() - 2];

    if (curTrade.price > prvTrade.price)
        OpenLong(bestBid);
    else if (curTrade.price < prvTrade.price)
        OpenShort(bestAsk);
}

void Example::OnSession(Session &session)
{
    curses = session;
    //Log("%s", GetString(curses).c_str());
}

void Example::OnSecurity(Security &security)
{
    cursec = security;
    //Log("%s", GetString(cursec).c_str());
}

void Example::OnReminder(long timestamp)
{
    if (timestamp == startTime) {
        Log("Start trading...");
        canTrade = true;
    } else if (timestamp == stopTime) {
        Log("Stop trading...");
        canTrade = true;
        ClosePosition();
    }
}

void Example::OpenLong(double price)
{
    CancelAll(cursec, ExecDir::Both);
    int pos = GetPosition(cursec);
    int qty = (pos < 0) ? std::abs(pos) + tradeQty : tradeQty;
    if (pos <= 0) {
        Log("open long");
        Order *order = BuyOrder(cursec, price, qty);
        Send(order);
    }
}

void Example::OpenShort(double price)
{
    CancelAll(cursec, ExecDir::Both);
    int pos = GetPosition(cursec);
    int qty = (pos > 0) ? std::abs(pos) + tradeQty : tradeQty;
    if (pos >= 0) {
        Log("open short");
        Order *order = SellOrder(cursec, price, qty);
        Send(order);
    }
}

void Example::ClosePosition()
{
    CancelAll(cursec, ExecDir::Both);
    int pos = GetPosition(cursec);
    if (pos > 0) {
        Log("close long");
        Order *order = SellOrder(cursec, cursec.minPrice, std::abs(pos));
        Send(order);
    } else if (pos < 0) {
        Log("close short");
        Order *order = BuyOrder(cursec, cursec.maxPrice, std::abs(pos));
        Send(order);
    }
}
