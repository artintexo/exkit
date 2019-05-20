#include "datasim.h"

#include "base/context.h"
#include "base/datareader.h"
#include "base/datetime.h"
#include "base/holder.h"

#include <algorithm>
#include <filesystem>

constexpr long NS_PER_DAY = 86400000000000;

DataSim::DataSim(Holder *hld)
    : hld(hld), opened(false)
{
    startTime = hld->config.simulator.start;
    stopTime = hld->config.simulator.stop;

    LOGD("DataSim created");
}

DataSim::~DataSim()
{
    LOGD("DataSim destroyed");
}

bool DataSim::IsOpened()
{
    return opened == true;
}

bool DataSim::IsClosed()
{
    return opened == false;
}

void DataSim::Open()
{
    PrepareReaders();
    ReadInitialData();
    opened = true;
    LOGD("DataSim opened");
}

void DataSim::Close()
{
    opened = false;
    LOGD("DataSim closed");
}

void DataSim::Subscribe(const Security &security)
{
    LOGD("DataSim subscribe %s", security.symbol.c_str());
}

void DataSim::Unsubscribe(const Security &security)
{
    LOGD("DataSim unsubscribe %s", security.symbol.c_str());
}

void DataSim::Start()
{
    LOGD("DataSim starting...");
    readFlag = true;
    readThread = std::thread(&DataSim::ReadData, this);
}

void DataSim::Stop()
{
    readFlag = false;
    if (readThread.joinable())
        readThread.join();

    LOGD("DataSim stopped");
}

void DataSim::PrepareReaders()
{
    std::vector<long> dates = GetDates();
    std::vector<std::string> files = GetFiles(dates);

    /*
    for (auto date : dates)
        LOGD("DataSim has date %s", GetString(Datetime(date)).c_str());
    for (auto file : files)
        LOGD("DataSim has file %s", file.c_str());
    */

    for (auto file : files)
        readers.push_back(new DataReader(file));

    if (readers.empty()) {
        LOGE("DataSim has no readers");
        throw std::runtime_error("DataSim has no readers");
    }
}

void DataSim::ReadInitialData()
{
    DataReader *reader = readers[0];

    while (true) {
        Event *event = reader->PeekEvent();

        if (event->type == EventType::NewObook) {
            NewObook *ne = static_cast<NewObook *>(event);
            if (ne->obook.timestamp >= startTime)
                break;
        }

        else if (event->type == EventType::NewTrade) {
            NewTrade *ne = static_cast<NewTrade *>(event);
            if (ne->trade.timestamp >= startTime)
                break;
        }

        else if (event->type == EventType::NewSession) {
            NewSession *ne = static_cast<NewSession *>(event);
            if (ne->session.timestamp >= startTime)
                break;
            hld->evman->PushData(ne);
        }

        else if (event->type == EventType::NewSecurity) {
            NewSecurity *ne = static_cast<NewSecurity *>(event);
            if (ne->security.timestamp >= startTime)
                break;
            hld->evman->PushData(ne);
        }

        NewHeartbeat *ne = new NewHeartbeat();
        ne->heartbeat.timestamp = startTime;
        hld->evman->PushData(ne);

        reader->ReadNext();
    }
}

void DataSim::ReadData()
{
    long timestamp = startTime;
    bool complete = false;

    size_t i = 0;
    while (i < readers.size() && readFlag) {
        DataReader *reader = readers[i];
        Event *event = reader->PeekEvent();

        if (event->type == EventType::NewObook) {
            NewObook *ne = static_cast<NewObook *>(event);
            timestamp = ne->obook.timestamp;
            if (startTime <= timestamp && timestamp < stopTime) {
                NewHeartbeat *nh = new NewHeartbeat();
                nh->heartbeat.timestamp = timestamp;
                hld->evman->PushData(nh);
                if (hld->subsman->HasSecId(ne->obook.secId))
                    hld->evman->PushData(ne);
                reader->ReadNext();
            } else {
                complete = true;
            }
        }

        else if (event->type == EventType::NewTrade) {
            NewTrade *ne = static_cast<NewTrade *>(event);
            timestamp = ne->trade.timestamp;
            if (startTime <= timestamp && timestamp < stopTime) {
                NewHeartbeat *nh = new NewHeartbeat();
                nh->heartbeat.timestamp = timestamp;
                hld->evman->PushData(nh);
                if (hld->subsman->HasSecId(ne->trade.secId))
                    hld->evman->PushData(ne);
                reader->ReadNext();
            } else {
                complete = true;
            }
        }

        else if (event->type == EventType::NewSession) {
            NewSession *ne = static_cast<NewSession *>(event);
            timestamp = ne->session.timestamp;
            if (startTime <= timestamp && timestamp < stopTime) {
                NewHeartbeat *nh = new NewHeartbeat();
                nh->heartbeat.timestamp = timestamp;
                hld->evman->PushData(nh);
                hld->evman->PushData(ne);
                reader->ReadNext();
            } else {
                complete = true;
            }
        }

        else if (event->type == EventType::NewSecurity) {
            NewSecurity *ne = static_cast<NewSecurity *>(event);
            timestamp = ne->security.timestamp;
            if (startTime <= timestamp && timestamp < stopTime) {
                NewHeartbeat *nh = new NewHeartbeat();
                nh->heartbeat.timestamp = timestamp;
                hld->evman->PushData(nh);
                if (hld->subsman->HasSymbol(ne->security.symbol))
                    hld->evman->PushData(ne);
                reader->ReadNext();
            } else {
                complete = true;
            }
        }

        else if (event->type == EventType::NewEndHistFile) {
            hld->evman->PushData(event);
            i++;
        }

        if (complete)
            break;
    }

    NewEndHistData *ne = new NewEndHistData();
    hld->evman->PushData(ne);
}

std::vector<long> DataSim::GetDates()
{
    Datetime st(startTime);
    Datetime start = Datetime(st.year, st.month, st.day);

    Datetime sp(stopTime);
    Datetime stop = Datetime(sp.year, sp.month, sp.day);

    std::vector<long> dates;
    for (long i = start.timestamp; i <= stop.timestamp; i += NS_PER_DAY)
        dates.push_back(i);

    std::sort(dates.begin(), dates.end());
    return dates;
}

std::vector<std::string> DataSim::GetFiles(std::vector<long> dates)
{
    std::vector<std::string> files;

    for (long date : dates) {
        Datetime dt(date);
        char buf[1024];
        snprintf(buf, 1024, "%s/%04d_%02d_%02d.csv", hld->config.simulator.histpath.c_str(), dt.year, dt.month, dt.day);

        if (std::filesystem::exists(buf))
            files.push_back(buf);
        else
            LOGW("DataSim has no %s", buf);
    }

    return files;
}
