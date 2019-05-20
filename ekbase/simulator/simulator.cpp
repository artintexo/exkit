#include "simulator.h"
#include "datasim.h"
#include "execsim.h"

#include "base/holder.h"

Simulator::Simulator(Holder *hld)
    : hld(hld), data(nullptr), exec(nullptr)
{
    if (hld->config.main.mode == Mode::Backtest) {
        data = new DataSim(hld);
        exec = new ExecSim(hld);
        hld->dgate = data;
        hld->egate = exec;
    }

    if (hld->config.main.mode == Mode::Paper) {
        exec = new ExecSim(hld);
        hld->egate = exec;
    }
}

Simulator::~Simulator()
{
    if (exec != nullptr)
        delete exec;

    if (data != nullptr)
        delete data;
}
