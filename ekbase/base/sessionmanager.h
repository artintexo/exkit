#pragma once

#include "base.h"

#include <mutex>

class SessionManager
{
public:
    SessionManager(Holder *hld);
    ~SessionManager();
    Session GetSession();
    void Update(Session &session);

private:
    Holder *hld;
    std::mutex mtx;
    Session session;
};
