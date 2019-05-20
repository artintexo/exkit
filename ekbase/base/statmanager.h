#pragma once

#include "base.h"

#include <map>
#include <mutex>
#include <stack>

class StatManager {
public:
    StatManager(Holder *hld);
    ~StatManager();

    Statistics GetStatistcs();
    void Update(UserTrade &utrade);

private:
    void Update(UserDeal &udeal);

    Holder *hld;
    std::mutex mtx;
    std::map<int, std::stack<UserTrade>> trades;
    Statistics stats;
};
