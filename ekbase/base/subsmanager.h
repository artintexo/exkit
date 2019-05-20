#pragma once

#include "base.h"

#include <mutex>
#include <vector>

class SubsManager {
public:
    SubsManager(Holder *hld);
    ~SubsManager();

    inline bool HasSymbol(std::string symbol)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (Security &sec : subs) {
            if (sec.symbol == symbol)
                return true;
        }
        return false;
    }

    inline bool HasSecId(int secId)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (Security &sec : subs) {
            if (sec.secId == secId)
                return true;
        }
        return false;
    }

    void Subscribe(Security &security);
    void Unsubscribe(Security &security);
    void Update(Security &security);

private:
    Holder *hld;
    std::mutex mtx;
    std::vector<Security> subs;
};
