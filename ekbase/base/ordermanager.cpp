#include "ordermanager.h"
#include "holder.h"
#include "order.h"

#include "plaza/execplaza.h"

OrderManager::OrderManager(Holder *hld)
    : hld(hld)
{
    clientId = hld->config.main.startId;
}

OrderManager::~OrderManager()
{
}

bool OrderManager::HasOrder(int clientId)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = orders.find(clientId);
    if (it != orders.end())
        return true;
    else
        return false;
}

Order *OrderManager::GetOrder(int clientId)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = orders.find(clientId);
    if (it != orders.end())
        return it->second;
    else
        return nullptr;
}

Order *OrderManager::MakeOrder(ExecDir dir, ExecTif tif, int secId, std::string symbol, int roundTo, double price, int qty)
{
    Order *order = new Order(clientId.fetch_add(1), dir, tif, secId, symbol, roundTo, price, qty);
    return order;
}

void OrderManager::Send(Order *order)
{
    if (order->IsCreated() == false || order->IsPendingAccept() == true) {
        LOGW("OrdMan send clientId = %d, status = %s", order->clientId, GetString(order->status).c_str());
        return;
    }

    mtx.lock();
    orders[order->clientId] = order;
    mtx.unlock();

    order->timePendingAccept = hld->timeman->GetTime();
    order->isPendingAccept = true;
    hld->egate->Send(order);
}

void OrderManager::Cancel(Order *order)
{
    if (order->IsActive() == false || order->IsPendingCancel() == true) {
        LOGW("OrdMan cancel clientId = %d, status = %s", order->clientId, GetString(order->status).c_str());
        return;
    }

    order->isPendingCancel = true;
    hld->egate->Cancel(order);
}

void OrderManager::CancelAll(const Security &security, ExecDir dir)
{
    hld->egate->CancelAll(security, dir);
}
