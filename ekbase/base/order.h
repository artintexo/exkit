#pragma once

#include "base.h"

class Order {
public:
    int ClientId() const { return clientId; }
    long OrderId() const { return orderId; }
    ExecStatus Status() const { return status; }
    ExecDir Dir() const { return dir; }
    ExecTif Tif() const { return tif; }
    int SecId() const { return secId; }
    std::string Symbol() const { return symbol; }
    double Price() const { return price; }
    double LastPrice() const { return lastPrice; }
    double AvgPrice() const { return avgPrice; }
    int Qty() const { return qty; }
    int LastQty() const { return lastQty; }
    int CumQty() const { return cumQty; }
    int LeavesQty() const { return leavesQty; }

    long FirstTime() const { return timePendingAccept; }
    long LastTime() const { return timeLast; }
    long Roundtrip() const { return timeAccepted - timePendingAccept; }
    bool IsPendingAccept() const { return isPendingAccept; }
    bool IsPendingCancel() const { return isPendingCancel; }
    bool IsCreated() const { return status == ExecStatus::Created; }
    bool IsActive() const { return status == ExecStatus::Accepted; }
    bool IsDone() const { return status > ExecStatus::Accepted; }
    bool IsFill() const { return lastQty > 0 && (status == ExecStatus::Accepted || status == ExecStatus::Filled); }

    void Update(const ExecReport &report);
    void Update(const ExecTrade &utrade);

private:
    friend class OrderManager;
    Order(int clientId, ExecDir dir, ExecTif tif, int secId, std::string symbol, int roundTo, double price, int qty);

    int clientId;
    long orderId;
    ExecStatus status;
    ExecDir dir;
    ExecTif tif;
    int secId;
    std::string symbol;
    int roundTo;
    double price;
    double lastPrice;
    double avgPrice;
    int qty;
    int lastQty;
    int cumQty;
    int leavesQty;

    bool isPendingAccept;
    bool isPendingCancel;
    long timePendingAccept;
    long timeAccepted;
    long timeLast;
};
