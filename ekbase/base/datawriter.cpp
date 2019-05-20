#include "datawriter.h"

#include <iomanip>

DataWriter::DataWriter(std::string path)
    : obookNum(0), tradeNum(0), sessNum(0), secNum(0)
{
    file.open(path);
}

DataWriter::~DataWriter()
{
    file.flush();
    file.close();
}

void DataWriter::Flush()
{
    file.flush();
}

void DataWriter::WriteObook(const Obook &o)
{
    file << static_cast<int>(DataType::Obook)
         << ";" << o.timestamp
         << ";" << o.secId;

    file << ";" << o.bids.size();
    if (o.bids.empty() == false) {
        for (const ObookItem &item : o.bids) {
            file << ";" << std::fixed << std::setprecision(5) << item.price
                 << ";" << item.qty;
        }
    }

    file << ";" << o.asks.size();
    if (o.asks.empty() == false) {
        for (const ObookItem &item : o.asks) {
            file << ";" << std::fixed << std::setprecision(5) << item.price
                 << ";" << item.qty;
        }
    }

    file << "\n";

    obookNum++;
}

void DataWriter::WriteTrade(const Trade &t)
{
    file << static_cast<int>(DataType::Trade)
         << ";" << t.timestamp
         << ";" << t.moment
         << ";" << t.secId
         << ";" << std::fixed << std::setprecision(5) << t.price
         << ";" << t.qty
         << "\n";

    tradeNum++;
}

void DataWriter::WriteSession(const Session &s)
{
    file << static_cast<int>(DataType::Session)
         << ";" << s.timestamp
         << ";" << s.sessId
         << ";" << s.mainState
         << ";" << s.interState
         << ";" << std::boolalpha << s.allowAccept
         << ";" << std::boolalpha << s.allowCancel
         << ";" << s.evening.begin
         << ";" << s.evening.end
         << ";" << s.before.begin
         << ";" << s.before.end
         << ";" << s.inter.begin
         << ";" << s.inter.end
         << ";" << s.after.begin
         << ";" << s.after.end
         << "\n";

    sessNum++;
}

void DataWriter::WriteSecurity(const Security &s)
{
    file << static_cast<int>(DataType::Security)
         << ";" << s.timestamp
         << ";" << s.sessId
         << ";" << s.secId
         << ";" << s.mainState
         << ";" << std::boolalpha << s.allowAccept
         << ";" << std::boolalpha << s.allowCancel
         << ";" << s.symbol
         << ";" << s.roundTo
         << ";" << std::fixed << std::setprecision(5) << s.minStep
         << ";" << std::fixed << std::setprecision(5) << s.minPrice
         << ";" << std::fixed << std::setprecision(5) << s.maxPrice
         << "\n";

    secNum++;
}
