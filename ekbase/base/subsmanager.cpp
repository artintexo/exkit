#include "subsmanager.h"
#include "holder.h"

#include <algorithm>

SubsManager::SubsManager(Holder *hld)
    : hld(hld)
{
}

SubsManager::~SubsManager()
{
}

void SubsManager::Subscribe(Security &security)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(subs.begin(), subs.end(), [&](Security &sec) { return sec.symbol == security.symbol; });
    if (it == subs.end()) {
        subs.push_back(security);
        hld->dgate->Subscribe(security);
    }
}

void SubsManager::Unsubscribe(Security &security)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(subs.begin(), subs.end(), [&](Security &sec) { return sec.symbol == security.symbol; });
    if (it != subs.end()) {
        subs.erase(it);
        hld->dgate->Unsubscribe(security);
    }
}

void SubsManager::Update(Security &security)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(subs.begin(), subs.end(), [&](Security &sec) { return sec.symbol == security.symbol; });
    if (it != subs.end())
        it->secId = security.secId;
}
