#include "saver.h"

#include "base/datawriter.h"
#include "base/datetime.h"
#include "base/holder.h"

#include <algorithm>

Saver::Saver(std::string path, std::vector<std::string> symbols)
    : cursyms(symbols)
{
    Datetime dt(GetSystemTime());
    char buf[1024];
    snprintf(buf, 1024, "%s/%04d_%02d_%02d.csv", path.c_str(), dt.year, dt.month, dt.day);
    writer = new DataWriter(buf);
}

Saver::~Saver()
{
    writer->Flush();
    delete writer;
}

void Saver::OnStart()
{
    Log("Saver starting...");

    WriteInitialData();

    cursess = GetSession();
    writer->WriteSession(cursess);
    Log("%s", GetString(cursess).c_str());

    for (std::string symbol : cursyms) {
        Security security = GetSecurity(symbol);
        cursecs.push_back(security);
        writer->WriteSecurity(security);
        Log("%s", GetString(security).c_str());
        Subscribe(security);
    }

    writer->Flush();
}

void Saver::OnStop()
{
    for (Security &security : cursecs)
        Unsubscribe(security);

    writer->Flush();
    Log("Saver stopped");
}

void Saver::OnObook(Obook &obook)
{
    if (cursess.mainState == 1) {
        writer->WriteObook(obook);
        LogState();
    }
}

void Saver::OnTrade(Trade &trade)
{
    if (cursess.mainState == 1) {
        writer->WriteTrade(trade);
        LogState();
    }
}

void Saver::OnSession(Session &session)
{
    cursess = session;
    writer->WriteSession(cursess);
    Log("%s", GetString(cursess).c_str());
    LogState();
}

void Saver::OnSecurity(Security &security)
{
    auto it = std::find_if(cursecs.begin(), cursecs.end(),
                           [&](Security &cursec) { return cursec.symbol == security.symbol; });

    if (it != cursecs.end()) {
        *it = security;
        writer->WriteSecurity(*it);
        Log("%s", GetString(*it).c_str());
        LogState();
    }
}

void Saver::WriteInitialData()
{
    Datetime dt(GetSystemTime());
    dt = Datetime(dt.year, dt.month, dt.day);

    Session session = GetSession();
    session.timestamp = dt.timestamp;
    writer->WriteSession(session);

    for (std::string symbol : cursyms) {
        Security security = GetSecurity(symbol);
        security.timestamp = dt.timestamp;
        writer->WriteSecurity(security);
    }

    writer->Flush();
}

void Saver::LogState()
{
    if (writer->GetTotal() % 10000 == 0) {
        writer->Flush();
        Log("total = %7ld, obook = %7ld, trade = %7ld, sess = %3ld, sec = %3ld",
            writer->GetTotal(),
            writer->GetObookNum(),
            writer->GetTradeNum(),
            writer->GetSessNum(),
            writer->GetSecNum());
    }
}
