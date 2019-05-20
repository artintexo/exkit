#include "holder.h"
#include "strategy.h"

#include "plaza/plaza.h"
#include "simulator/simulator.h"

Holder::Holder(Config &config, IStrategy *strategy)
    : config(config), strat(strategy)
{
    logman = new LogManager(this);
    timeman = new TimeManager(this);
    subsman = new SubsManager(this);
    ordman = new OrderManager(this);
    posman = new PositionManager(this);
    statman = new StatManager(this);
    sessman = new SessionManager(this);
    secman = new SecurityManager(this);
    evman = new EventManager(this);

    plz = new Plaza(this);
    sim = new Simulator(this);

    static_cast<Strategy *>(strat)->hld = this;
}

Holder::~Holder()
{
    delete plz;
    delete sim;

    delete evman;
    delete secman;
    delete sessman;
    delete statman;
    delete posman;
    delete ordman;
    delete subsman;
    delete timeman;
    delete logman;
}
