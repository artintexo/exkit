#include "order.h"

#include <cassert>

Order::Order(int clientId, ExecDir dir, ExecTif tif, int secId, std::string symbol, int roundTo, double price, int qty)
{
    this->clientId = clientId;
    this->orderId = 0;
    this->status = ExecStatus::Created;

    this->dir = dir;
    this->tif = tif;
    this->secId = secId;
    this->symbol = symbol;
    this->roundTo = roundTo;
    this->price = price;
    this->lastPrice = 0;
    this->avgPrice = 0;
    this->qty = qty;
    this->lastQty = 0;
    this->cumQty = 0;
    this->leavesQty = qty;

    this->isPendingAccept = false;
    this->isPendingCancel = false;
    this->timePendingAccept = 0;
    this->timeAccepted = 0;
    this->timeLast = 0;
}

void Order::Update(const ExecReport &report)
{
    switch (report.status) {

    case ExecStatus::AcceptReject:
        isPendingAccept = false;
        timeAccepted = report.timestamp;
        status = ExecStatus::Rejected;
        break;

    case ExecStatus::CancelReject:
        isPendingCancel = false;
        break;

    case ExecStatus::Accepted:
        isPendingAccept = false;
        timeAccepted = report.timestamp;
        orderId = report.orderId;
        status = report.status;
        break;

    case ExecStatus::Cancelled:
        isPendingCancel = false;
        status = report.status;
        break;

    case ExecStatus::Rejected:
        isPendingAccept = false;
        timeAccepted = report.timestamp;
        status = report.status;
        break;

    default:
        break;
    }

    timeLast = report.timestamp;
}

void Order::Update(const ExecTrade &trade)
{
    lastPrice = trade.price;
    lastQty = trade.qty;

    avgPrice = (avgPrice * cumQty + lastPrice * lastQty) / (cumQty + lastQty);
    avgPrice = Round(avgPrice, roundTo);

    cumQty += lastQty;
    leavesQty -= lastQty;
    assert(cumQty <= qty);
    assert(leavesQty >= 0);

    if (leavesQty == 0)
        status = ExecStatus::Filled;

    timeLast = trade.timestamp;
}
