#pragma once

#include "base.h"

#include <map>
#include <mutex>

class PositionManager {
public:
    PositionManager(Holder *hld);
    ~PositionManager();

    int GetPosition(int secId);
    void Update(UserTrade &utrade);

private:
    Holder *hld;
    std::mutex mtx;
    std::map<int, int> positions;
};
