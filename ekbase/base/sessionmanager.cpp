#include "sessionmanager.h"
#include "holder.h"

SessionManager::SessionManager(Holder *hld)
    : hld(hld), session()
{
}

SessionManager::~SessionManager()
{
}

Session SessionManager::GetSession()
{
    std::lock_guard<std::mutex> lock(mtx);

    if (session.sessId == 0)
        LOGW("SessMan has no session");

    return session;
}

void SessionManager::Update(Session &session)
{
    std::lock_guard<std::mutex> lock(mtx);
    this->session = session;
}
