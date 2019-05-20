#pragma once

#include "base.h"
#include "order.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

class OrderManager {
public:
    OrderManager(Holder *hld);
    ~OrderManager();

    bool HasOrder(int clientId);
    Order *GetOrder(int clientId);
    Order *MakeOrder(ExecDir dir, ExecTif tif, int secId, std::string symbol, int roundTo, double price, int qty);
    void Send(Order *order);
    void Cancel(Order *order);
    void CancelAll(const Security &security, ExecDir dir);
    void CancelAll(std::string symbol, ExecDir dir);

private:
    Holder *hld;
    std::atomic<int> clientId;
    std::mutex mtx;
    std::unordered_map<int, Order *> orders;
};
