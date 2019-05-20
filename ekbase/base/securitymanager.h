#pragma once

#include "base.h"

#include <mutex>
#include <vector>

class SecurityManager {
public:
    SecurityManager(Holder *hld);
    ~SecurityManager();
    Security GetSecurity(std::string symbol);
    void Update(Security &security);

private:
    Holder *hld;
    std::mutex mtx;
    std::vector<Security> secs;
};
