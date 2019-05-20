#include "dataplaza.h"
#include "connection.h"
#include "dealsrepl.h"
#include "futaggrrepl.h"
#include "futinforepl.h"

#include "base/holder.h"

#include <sstream>
#include <thread>

DataPlaza::DataPlaza(Plaza *plz)
    : plz(plz), hld(plz->GetHolder())
{
    std::ostringstream oss;

    oss.str("");
    oss << hld->config.plaza.type << "://" << hld->config.plaza.host << ":" << hld->config.plaza.port << ";";
    oss << "app_name=" << hld->config.main.name << "DataConn;";
    oss << "name=" << hld->config.main.name << "DataConn;";
    LOGD("%s", oss.str().c_str());
    dataConn = new Connection(plz, "DataConn", oss.str());

    oss.str("");
    oss << "p2repl://FORTS_FUTINFO_REPL;";
    oss << "tables=session,fut_sess_contents;";
    oss << "name=" << hld->config.main.name << "FutInfo;";
    LOGD("%s", oss.str().c_str());
    futInfoRepl = new FutInfoRepl(plz, dataConn, "FutInfo", oss.str(), "mode=snapshot+online");

    oss.str("");
    oss << "p2repl://FORTS_FUTAGGR" << EK_OBOOK_DEPTH << "_REPL;";
    oss << "tables=orders_aggr;";
    oss << "name=" << hld->config.main.name << "FutAggr;";
    LOGD("%s", oss.str().c_str());
    futAggrRepl = new FutAggrRepl(plz, dataConn, "FutAggr", oss.str(), "mode=snapshot+online");

    oss.str("");
    oss << "p2repl://FORTS_DEALS_REPL;";
    oss << "tables=deal,heartbeat;";
    oss << "name=" << hld->config.main.name << "Deals;";
    LOGD("%s", oss.str().c_str());
    dealsRepl = new DealsRepl(plz, dataConn, "Deals", oss.str(), "mode=online");

    LOGD("DataPlaza created");
}

DataPlaza::~DataPlaza()
{
    delete dealsRepl;
    delete futAggrRepl;
    delete futInfoRepl;
    delete dataConn;

    LOGD("DataPlaza destroyed");
}

bool DataPlaza::IsOpened()
{
    return dataConn->IsOpened() &&
           futInfoRepl->IsOpened() && futInfoRepl->IsOnline() &&
           futAggrRepl->IsOpened() && futAggrRepl->IsOnline() &&
           dealsRepl->IsOpened() && dealsRepl->IsOnline();
}

bool DataPlaza::IsClosed()
{
    return dataConn->IsClosed() &&
           futInfoRepl->IsClosed() &&
           futAggrRepl->IsClosed() &&
           dealsRepl->IsClosed();
}

void DataPlaza::Open()
{
    dataConn->Open();

    processFlag = true;
    processThread = std::thread(&DataPlaza::Process, this);
}

void DataPlaza::Close()
{
    processFlag = false;
    if (processThread.joinable())
        processThread.join();

    dataConn->Close();
}

void DataPlaza::Subscribe(const Security &security)
{
    LOGD("DataPlaza subscribe %s", security.symbol.c_str());
}

void DataPlaza::Unsubscribe(const Security &security)
{
    LOGD("DataPlaza unsubscribe %s", security.symbol.c_str());
}

void DataPlaza::Process()
{
    // open futInfo
    while (futInfoRepl->IsOpened() == false) {
        futInfoRepl->Open();
        dataConn->Process();
    }

    // open futAggr
    while (futAggrRepl->IsOpened() == false) {
        futAggrRepl->Open();
        dataConn->Process();
    }

    // open deals
    while (dealsRepl->IsOpened() == false) {
        dealsRepl->Open();
        dataConn->Process();
    }

    // process
    while (processFlag) {
        dataConn->Process();
        if (dataConn->IsOpened()) {
            futInfoRepl->Process();
            futAggrRepl->Process();
            dealsRepl->Process();
        }
    }

    // close deals
    while (dealsRepl->IsClosed() == false) {
        dealsRepl->Close();
        dataConn->Process();
    }

    // close futAggr
    while (futAggrRepl->IsClosed() == false) {
        futAggrRepl->Close();
        dataConn->Process();
    }

    // close futInfo
    while (futInfoRepl->IsClosed() == false) {
        futInfoRepl->Close();
        dataConn->Process();
    }
}
