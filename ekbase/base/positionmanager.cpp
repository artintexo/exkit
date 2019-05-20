#include "positionmanager.h"

PositionManager::PositionManager(Holder *hld)
    : hld(hld)
{
}

PositionManager::~PositionManager()
{
}

int PositionManager::GetPosition(int secId)
{
    std::lock_guard<std::mutex> lock(mtx);

    auto it = positions.find(secId);
    if (it != positions.end())
        return it->second;
    else
        return 0;
}

void PositionManager::Update(UserTrade &utrade)
{
    std::lock_guard<std::mutex> lock(mtx);

    int value = 0;
    if (utrade.dir == ExecDir::Long)
        value = utrade.qty;
    else if (utrade.dir == ExecDir::Short)
        value = -utrade.qty;

    auto it = positions.find(utrade.secId);
    if (it != positions.end())
        it->second += value;
    else
        positions[utrade.secId] = value;
}
