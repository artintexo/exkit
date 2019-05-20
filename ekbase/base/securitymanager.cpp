#include "securitymanager.h"
#include "holder.h"

#include <algorithm>

SecurityManager::SecurityManager(Holder *hld)
    : hld(hld)
{
}

SecurityManager::~SecurityManager()
{
}

Security SecurityManager::GetSecurity(std::string symbol)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(secs.begin(), secs.end(),
                           [&](Security &sec) { return sec.symbol == symbol; });

    if (it != secs.end())
        return *it;

    LOGW("SecrMan has no security with symbol = %s", symbol.c_str());
    return Security();
}

void SecurityManager::Update(Security &security)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::find_if(secs.begin(), secs.end(),
                           [&](Security &sec) { return sec.symbol == security.symbol; });

    if (it != secs.end())
        *it = security;
    else
        secs.push_back(security);
}
