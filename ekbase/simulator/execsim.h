#pragma once

#include "base/base.h"

class ExecSim : public IExecGate {
public:
    ExecSim(Holder *hld);
    ~ExecSim();

    bool IsOpened() override;
    bool IsClosed() override;
    void Open() override;
    void Close() override;

    void Send(Order *order) override;
    void Cancel(Order *order) override;
    void CancelAll(const Security &, ExecDir) override;

    void OnObook(const Obook &obook);
    void OnTrade(const Trade &trade);

private:
    void TryFillOrder(Order *order, const Obook &obook);
    void TryFillOrder(Order *order, const Trade &trade);
    void RemoveDoneOrders();
    Holder *hld;
    bool opened;
    long counter;
    std::vector<Order *> actives;
};
