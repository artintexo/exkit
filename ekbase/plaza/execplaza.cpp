#include "execplaza.h"
#include "connection.h"
#include "futtraderepl.h"
#include "publisher.h"
#include "pubrepl.h"

#include "base/holder.h"

#include <sstream>

ExecPlaza::ExecPlaza(Plaza *plz)
    : plz(plz), hld(plz->GetHolder())
{
    std::ostringstream oss;

    oss.str("");
    oss << hld->config.plaza.type << "://" << hld->config.plaza.host << ":" << hld->config.plaza.port << ";";
    oss << "app_name=" << hld->config.main.name << "ExecConn;";
    oss << "name=" << hld->config.main.name << "ExecConn;";
    LOGD("%s", oss.str().c_str());
    execConn = new Connection(plz, "ExecConn", oss.str());

    // Pub
    oss.str("");
    oss << "p2mq://FORTS_SRV;";
    oss << "category=FORTS_MSG;";
    oss << "name=" << hld->config.main.name << "Pub;";
    LOGD("%s", oss.str().c_str());
    publisher = new Publisher(plz, execConn, "Pub", oss.str());

    // PubLsn
    oss.str("");
    oss << "p2mqreply://;";
    oss << "ref=" << hld->config.main.name << "Pub;";
    oss << "name=" << hld->config.main.name << "PubRepl;";
    LOGD("%s", oss.str().c_str());
    pubRepl = new PubRepl(plz, execConn, "PubRepl", oss.str(), "mode=snapshot+online");

    // FutTrade
    oss.str("");
    oss << "p2repl://FORTS_FUTTRADE_REPL;";
    oss << "tables=orders_log;";
    oss << "name=" << hld->config.main.name << "FutTrade;";
    LOGD("%s", oss.str().c_str());
    futTradeRepl = new FutTradeRepl(plz, execConn, "FutTrade", oss.str(), "mode=online");

    LOGD("ExecPlaza created");
}

ExecPlaza::~ExecPlaza()
{
    LOGD("ExecPlaza destroyed");
}

bool ExecPlaza::IsOpened()
{
    return execConn->IsOpened() &&
           publisher->IsOpened() &&
           pubRepl->IsOpened() && pubRepl->IsOnline() &&
           futTradeRepl->IsOpened() && futTradeRepl->IsOnline();
}

bool ExecPlaza::IsClosed()
{
    return execConn->IsClosed() &&
           publisher->IsClosed() &&
           pubRepl->IsClosed() &&
           futTradeRepl->IsClosed();
}

void ExecPlaza::Open()
{
    execConn->Open();

    processFlag = true;
    processThread = std::thread(&ExecPlaza::Process, this);
}

void ExecPlaza::Close()
{
    processFlag = false;
    if (processThread.joinable())
        processThread.join();

    execConn->Close();
}

void ExecPlaza::Send(Order *order)
{
    bool sent = publisher->Send(order);
    if (sent == false) {
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = GetSystemTime();
        ne->report.clientId = order->ClientId();
        ne->report.orderId = 0;
        ne->report.status = ExecStatus::AcceptReject;
        hld->evman->PushExec(ne);
    }
}

void ExecPlaza::Cancel(Order *order)
{
    bool sent = publisher->Cancel(order);
    if (sent == false) {
        NewExecReport *ne = new NewExecReport();
        ne->report.timestamp = GetSystemTime();
        ne->report.clientId = order->ClientId();
        ne->report.orderId = order->OrderId();
        ne->report.status = ExecStatus::CancelReject;
        hld->evman->PushExec(ne);
    }
}

void ExecPlaza::CancelAll(const Security &security, ExecDir dir)
{
    publisher->CancelAll(security, dir);
}

void ExecPlaza::Process()
{
    // open publisher
    while (publisher->IsOpened() == false) {
        publisher->Open();
        execConn->Process();
    }

    // open pubRepl
    while (pubRepl->IsOpened() == false) {
        pubRepl->Open();
        execConn->Process();
    }

    // open futTradeRepl
    while (futTradeRepl->IsOpened() == false) {
        futTradeRepl->Open();
        execConn->Process();
    }

    // process
    while (processFlag) {
        execConn->Process();
        if (execConn->IsOpened()) {
            publisher->Process();
            pubRepl->Process();
            futTradeRepl->Process();
        }
    }

    // close futTradeRepl
    while (futTradeRepl->IsClosed() == false) {
        futTradeRepl->Close();
        execConn->Process();
    }

    // close pubRepl
    while (pubRepl->IsClosed() == false) {
        pubRepl->Close();
        execConn->Process();
    }

    // close publisher
    while (publisher->IsClosed() == false) {
        publisher->Close();
        execConn->Process();
    }
}
