#include "execsim.h"

#include "base/holder.h"
#include "base/order.h"

#include <algorithm>
#include <cassert>

ExecSim::ExecSim(Holder *hld)
    : hld(hld), opened(false)
{
    counter = 0;
    LOGD("ExecSim created");
}

ExecSim::~ExecSim()
{
    LOGD("ExecSim destroyed");
}

bool ExecSim::IsOpened()
{
    return opened == true;
}

bool ExecSim::IsClosed()
{
    return opened == false;
}

void ExecSim::Open()
{
    opened = true;
    LOGD("ExecSim opened");
}

void ExecSim::Close()
{
    opened = false;
    LOGD("ExecSim closed");
}

void ExecSim::Send(Order *order)
{
    NewExecReport *ne = new NewExecReport();
    ne->report.timestamp = hld->timeman->GetTime();
    ne->report.clientId = order->ClientId();
    ne->report.orderId = order->ClientId();
    ne->report.status = ExecStatus::Accepted;

    actives.push_back(new Order(*order));
    actives[actives.size() - 1]->Update(ne->report);

    hld->evman->OnNewExecReport(ne);
}

void ExecSim::Cancel(Order *order)
{
    NewExecReport *ne = new NewExecReport();
    ne->report.timestamp = hld->timeman->GetTime();
    ne->report.clientId = order->ClientId();
    ne->report.orderId = order->OrderId();
    ne->report.status = ExecStatus::Cancelled;

    auto it = std::remove_if(actives.begin(), actives.end(), [=](Order *exist) { return order->ClientId() == exist->ClientId(); });
    actives.erase(it, actives.end());

    hld->evman->OnNewExecReport(ne);
}

void ExecSim::CancelAll(const Security &, ExecDir)
{
    for (size_t i = 0; i < actives.size(); i++) {
        Order *order = actives[i];
        if (order->IsActive() == true) {
            NewExecReport *ne = new NewExecReport();
            ne->report.timestamp = hld->timeman->GetTime();
            ne->report.clientId = order->ClientId();
            ne->report.orderId = order->OrderId();
            ne->report.status = ExecStatus::Cancelled;

            order->Update(ne->report);
            hld->evman->OnNewExecReport(ne);
        }
    }
}

void ExecSim::OnObook(const Obook &obook)
{
    for (size_t i = 0; i < actives.size(); i++) {
        Order *order = actives[i];
        if (order->IsActive() == true && order->SecId() == obook.secId)
            TryFillOrder(order, obook);
    }

    if (counter++ % 5000 == 0)
        RemoveDoneOrders();
}

void ExecSim::OnTrade(const Trade &trade)
{
    for (size_t i = 0; i < actives.size(); i++) {
        Order *order = actives[i];
        if (order->IsActive() == true && order->SecId() == trade.secId)
            TryFillOrder(order, trade);
    }

    if (counter++ % 5000 == 0)
        RemoveDoneOrders();
}

void ExecSim::TryFillOrder(Order *order, const Obook &obook)
{
    assert(order->LeavesQty() > 0);
    assert(order->CumQty() <= order->Qty());

    if (order->Tif() != ExecTif::Day)
        return;

    if (order->Dir() == ExecDir::Long) {
        if (obook.asks.empty())
            return;

        const ObookItem &item = obook.asks[0];
        if (item.price > order->Price() || order->IsActive() == false)
            return;

        NewExecTrade *ne = new NewExecTrade();
        ne->trade.timestamp = obook.timestamp;
        ne->trade.clientId = order->ClientId();
        ne->trade.orderId = order->OrderId();
        ne->trade.price = item.price;
        ne->trade.qty = order->LeavesQty();

        order->Update(ne->trade);
        hld->evman->OnNewExecTrade(ne);

    }

    else if (order->Dir() == ExecDir::Short) {
        if (obook.bids.empty())
            return;

        const ObookItem &item = obook.bids[0];
        if (item.price < order->Price() || order->IsActive() == false)
            return;

        NewExecTrade *ne = new NewExecTrade();
        ne->trade.timestamp = obook.timestamp;
        ne->trade.clientId = order->ClientId();
        ne->trade.orderId = order->OrderId();
        ne->trade.price = item.price;
        ne->trade.qty = order->LeavesQty();

        order->Update(ne->trade);
        hld->evman->OnNewExecTrade(ne);
    }
}

void ExecSim::TryFillOrder(Order *order, const Trade &trade)
{
    assert(order->LeavesQty() > 0);
    assert(order->CumQty() <= order->Qty());

    if (order->Tif() != ExecTif::Day)
        return;

    if (order->Dir() == ExecDir::Long) {
        if (trade.price < order->Price()) {
            NewExecTrade *ne = new NewExecTrade();
            ne->trade.timestamp = trade.timestamp;
            ne->trade.clientId = order->ClientId();
            ne->trade.orderId = order->OrderId();
            ne->trade.price = trade.price;
            ne->trade.qty = order->LeavesQty();

            order->Update(ne->trade);
            hld->evman->OnNewExecTrade(ne);
        }
    }

    else if (order->Dir() == ExecDir::Short) {
        if (trade.price > order->Price()) {
            NewExecTrade *ne = new NewExecTrade();
            ne->trade.timestamp = trade.timestamp;
            ne->trade.clientId = order->ClientId();
            ne->trade.orderId = order->OrderId();
            ne->trade.price = trade.price;
            ne->trade.qty = order->LeavesQty();

            order->Update(ne->trade);
            hld->evman->OnNewExecTrade(ne);
        }
    }
}

void ExecSim::RemoveDoneOrders()
{
    for (auto it = actives.begin(); it != actives.end(); it++) {
        if ((*it)->IsDone()) {
            delete *it;
            *it = nullptr;
        }
    }

    auto it = std::remove_if(actives.begin(), actives.end(), [](Order *order) { return order == nullptr; });
    actives.erase(it, actives.end());
}
