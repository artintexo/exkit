#include "datareader.h"

#include <cassert>
#include <iostream>

DataReader::DataReader(std::string filename)
    : filename(filename), event(nullptr)
{
    file.open(filename);
    ReadNext();
}

DataReader::~DataReader()
{
    file.close();
}

Event *DataReader::PeekEvent()
{
    assert(event != nullptr);
    return event;
}

void DataReader::ReadNext()
{
    std::string row;
    if (std::getline(file, row, '\n')) {
        std::vector<std::string> data = Split(row, ';');

        if (data.empty() == false) {
            DataType type = static_cast<DataType>(std::atoi(data[0].c_str()));
            if (type == DataType::Obook)
                event = ReadObook(data);
            else if (type == DataType::Trade)
                event = ReadTrade(data);
            else if (type == DataType::Session)
                event = ReadSession(data);
            else if (type == DataType::Security)
                event = ReadSecurity(data);
        }
    } else {
        auto ne = new NewEndHistFile();
        ne->file = filename;
        event = ne;
    }
}

Event *DataReader::ReadObook(std::vector<std::string> data)
{
    assert(data.size() >= 5);
    NewObook *ne = new NewObook();
    ne->obook.timestamp = std::atol(data[1].c_str());
    ne->obook.secId = std::atoi(data[2].c_str());

    int bidSizeIdx = 3;
    int bidSize = std::atoi(data[bidSizeIdx].c_str());
    for (int i = bidSizeIdx + 1; i < bidSizeIdx + 1 + bidSize * 2; i += 2) {
        double price = std::atof(data[i].c_str());
        int qty = std::atoi(data[i + 1].c_str());
        ne->obook.bids.push_back(ObookItem(price, qty));
    }

    int askSizeIdx = bidSizeIdx + bidSize * 2 + 1;
    int askSize = std::atoi(data[askSizeIdx].c_str());
    for (int i = askSizeIdx + 1; i < askSizeIdx + 1 + askSize * 2; i += 2) {
        double price = std::atof(data[i].c_str());
        int qty = std::atoi(data[i + 1].c_str());
        ne->obook.asks.push_back(ObookItem(price, qty));
    }

    return ne;
}

Event *DataReader::ReadTrade(std::vector<std::string> data)
{
    assert(data.size() == 6);
    NewTrade *ne = new NewTrade();
    ne->trade.timestamp = std::atol(data[1].c_str());
    ne->trade.moment = std::atol(data[2].c_str());
    ne->trade.secId = std::atoi(data[3].c_str());
    ne->trade.price = std::atof(data[4].c_str());
    ne->trade.qty = std::atoi(data[5].c_str());
    return ne;
}

Event *DataReader::ReadSession(std::vector<std::string> data)
{
    assert(data.size() == 15);
    NewSession *ne = new NewSession();

    ne->session.timestamp = std::atol(data[1].c_str());
    ne->session.sessId = std::atoi(data[2].c_str());
    ne->session.mainState = std::atoi(data[3].c_str());
    ne->session.interState = std::atoi(data[4].c_str());

    ne->session.allowAccept = (data[5] == "true") ? true : false;
    ne->session.allowCancel = (data[6] == "true") ? true : false;

    ne->session.evening.begin = std::atol(data[7].c_str());
    ne->session.evening.end = std::atol(data[8].c_str());

    ne->session.before.begin = std::atol(data[9].c_str());
    ne->session.before.end = std::atol(data[10].c_str());

    ne->session.inter.begin = std::atol(data[11].c_str());
    ne->session.inter.end = std::atol(data[12].c_str());

    ne->session.after.begin = std::atol(data[13].c_str());
    ne->session.after.end = std::atol(data[14].c_str());

    return ne;
}

Event *DataReader::ReadSecurity(std::vector<std::string> data)
{
    assert(data.size() == 12);
    NewSecurity *ne = new NewSecurity();

    ne->security.timestamp = std::atol(data[1].c_str());
    ne->security.sessId = std::atoi(data[2].c_str());
    ne->security.secId = std::atoi(data[3].c_str());
    ne->security.mainState = std::atoi(data[4].c_str());

    ne->security.allowAccept = (data[5] == "true") ? true : false;
    ne->security.allowCancel = (data[6] == "true") ? true : false;

    ne->security.symbol = data[7];
    ne->security.roundTo = std::atoi(data[8].c_str());

    ne->security.minStep = std::atof(data[9].c_str());
    ne->security.minPrice = std::atof(data[10].c_str());
    ne->security.maxPrice = std::atof(data[11].c_str());

    return ne;
}
