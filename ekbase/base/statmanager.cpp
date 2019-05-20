#include "statmanager.h"
#include "holder.h"

#include <cassert>

StatManager::StatManager(Holder *hld)
    : hld(hld), stats()
{
}

StatManager::~StatManager()
{
}

Statistics StatManager::GetStatistcs()
{
    std::lock_guard<std::mutex> lock(mtx);
    return stats;
}

void StatManager::Update(UserTrade &utrade)
{
    if (utrade.dir == ExecDir::Both) {
        LOGW("StatMan dir = %d", GetString(utrade.dir).c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(mtx);

    auto it = trades.find(utrade.secId);
    if (it != trades.end()) {
        auto &stack = it->second;

        if (stack.empty() == false) {
            while (stack.empty() == false) {
                auto &exist = stack.top();

                if (exist.dir != utrade.dir) {
                    int value = exist.qty - utrade.qty;

                    UserDeal udeal;
                    udeal.secId = exist.secId;
                    udeal.dir = exist.dir;
                    udeal.qty = 0;
                    udeal.open.timestamp = exist.timestamp;
                    udeal.open.price = exist.price;
                    udeal.close.timestamp = utrade.timestamp;
                    udeal.close.price = utrade.price;

                    if (value == 0) {
                        udeal.qty = exist.qty;
                        stack.pop();
                        Update(udeal);
                        break;
                    } else if (value > 0) {
                        udeal.qty = utrade.qty;
                        exist.qty -= udeal.qty;
                        Update(udeal);
                        break;
                    } else if (value < 0) {
                        udeal.qty = exist.qty;
                        utrade.qty -= udeal.qty;
                        stack.pop();
                        Update(udeal);
                        if (stack.empty()) {
                            stack.push(utrade);
                            break;
                        } else {
                            continue;
                        }
                    }
                } else {
                    stack.push(utrade);
                    break;
                }
            }
        } else {
            stack.push(utrade);
        }
    } else {
        trades[utrade.secId].push(utrade);
    }
}

void StatManager::Update(UserDeal &udeal)
{
    double profit = 0;
    if (udeal.dir == ExecDir::Long)
        profit = (udeal.close.price - udeal.open.price) * udeal.qty;
    else if (udeal.dir == ExecDir::Short)
        profit = (udeal.open.price - udeal.close.price) * udeal.qty;

    if (profit > 0) {
        stats.grossWin += profit;
        stats.numWin += 1;
    } else {
        stats.grossLoss += -profit;
        stats.numLoss += 1;
    }

    stats.grossAll = stats.grossWin - stats.grossLoss;
    stats.profitFactor = (stats.grossLoss == 0) ? 0 : stats.grossWin / stats.grossLoss;
    stats.numAll = stats.numWin + stats.numLoss;
    stats.winRate = (stats.numWin * 1.0) / stats.numAll * 100;
    stats.avgProfit = ((stats.avgProfit * (stats.numAll - 1)) + profit) / stats.numAll;

    long time = udeal.close.timestamp - udeal.open.timestamp;
    assert(time >= 0);
    stats.avgTime = ((stats.avgTime * (stats.numAll - 1)) + time) / stats.numAll;
}
