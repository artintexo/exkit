#include "eventmanager.h"
#include "context.h"
#include "datetime.h"
#include "holder.h"

#include "simulator/execsim.h"

#include <thread>

EventManager::EventManager(Holder *hld)
    : hld(hld), started(false)
{
}

EventManager::~EventManager()
{
}

void EventManager::Process()
{
    int esize = execQueue.GetSize();
    int dsize = dataQueue.GetSize();

    if (esize == 0 && dsize == 0) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        return;
    }

    while (esize-- > 0) {
        Event *event = execQueue.Dequeue();
        switch (event->type) {
        case EventType::NewExecReport:
            OnNewExecReport(event);
            break;
        case EventType::NewExecTrade:
            OnNewExecTrade(event);
            break;
        default:
            LOGW("Found non exec event in execQueue");
            break;
        }
    }

    while (dsize-- > 0) {
        Event *event = dataQueue.Dequeue();
        switch (event->type) {
        case EventType::NewObook:
            OnNewObook(event);
            break;
        case EventType::NewTrade:
            OnNewTrade(event);
            break;
        case EventType::NewSession:
            OnNewSession(event);
            break;
        case EventType::NewSecurity:
            OnNewSecurity(event);
            break;
        case EventType::NewHeartbeat:
            OnNewHeartbeat(event);
            break;
        case EventType::NewEndHistFile:
            OnNewEndHistFile(event);
            break;
        case EventType::NewEndHistData:
            OnNewEndHistData(event);
            break;
        default:
            LOGW("Found non data event in dataQueue");
            break;
        }
    }
}

void EventManager::OnNewExecReport(Event *event)
{
    auto ne = static_cast<NewExecReport *>(event);
    Order *order = hld->ordman->GetOrder(ne->report.clientId);

    if (order != nullptr) {
        order->Update(ne->report);
        if (started)
            hld->strat->OnOrder(order);
    } else {
        LOGW("can't handle report, clientId = %d", ne->report.clientId);
    }

    delete ne;
}

void EventManager::OnNewExecTrade(Event *event)
{
    auto ne = static_cast<NewExecTrade *>(event);
    Order *order = hld->ordman->GetOrder(ne->trade.clientId);

    if (order != nullptr) {
        order->Update(ne->trade);

        UserTrade utrade;
        utrade.timestamp = ne->trade.timestamp;
        utrade.secId = order->SecId();
        utrade.dir = order->Dir();
        utrade.price = ne->trade.price;
        utrade.qty = ne->trade.qty;
        //LOGD("%ld %s %.f %d", utrade.timestamp, GetString(utrade.dir).c_str(), utrade.price, utrade.qty);

        hld->posman->Update(utrade);
        hld->statman->Update(utrade);

        if (started)
            hld->strat->OnOrder(order);
    } else {
        LOGW("can't handle trade, clientId = %d", ne->trade.clientId);
    }

    delete ne;
}

void EventManager::OnNewObook(Event *event)
{
    auto ne = static_cast<NewObook *>(event);

    if (hld->config.main.mode != Mode::Realtime && hld->config.simulator.fillOnObook == true) {
        ExecSim *esim = static_cast<ExecSim *>(hld->egate);
        esim->OnObook(ne->obook);
    }

    if (started)
        hld->strat->OnObook(ne->obook);

    delete ne;
}

void EventManager::OnNewTrade(Event *event)
{
    auto ne = static_cast<NewTrade *>(event);

    if (hld->config.main.mode != Mode::Realtime && hld->config.simulator.fillOnTrade == true) {
        ExecSim *esim = static_cast<ExecSim *>(hld->egate);
        esim->OnTrade(ne->trade);
    }

    if (started)
        hld->strat->OnTrade(ne->trade);

    delete ne;
}

void EventManager::OnNewSession(Event *event)
{
    auto ne = static_cast<NewSession *>(event);
    hld->sessman->Update(ne->session);

    if (started)
        hld->strat->OnSession(ne->session);

    delete ne;
}

void EventManager::OnNewSecurity(Event *event)
{
    auto ne = static_cast<NewSecurity *>(event);
    hld->secman->Update(ne->security);

    if (hld->subsman->HasSymbol(ne->security.symbol)) {
        hld->subsman->Update(ne->security);
        if (started)
            hld->strat->OnSecurity(ne->security);
    }

    delete ne;
}

void EventManager::OnNewHeartbeat(Event *event)
{
    auto ne = static_cast<NewHeartbeat *>(event);
    hld->timeman->SetTime(ne->heartbeat.timestamp);
    //LOGD("heartbeat %s", GetString(Datetime(ne->heartbeat.timestamp)).c_str());
    delete ne;
}

void EventManager::OnNewEndHistFile(Event *event)
{
    auto ne = static_cast<NewEndHistFile *>(event);
    LOGD("end of history file %s", ne->file.c_str());
    delete ne;
}

void EventManager::OnNewEndHistData(Event *event)
{
    auto ne = static_cast<NewEndHistData *>(event);
    LOGD("end of history data");
    Application::SetProcessFlag(false);
    delete ne;
}
