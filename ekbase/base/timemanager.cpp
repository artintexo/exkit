#include "timemanager.h"
#include "holder.h"

TimeManager::TimeManager(Holder *hld)
    : hld(hld)
{
}

TimeManager::~TimeManager()
{
}

long TimeManager::GetTime() const
{
    if (hld->config.main.mode != Mode::Backtest)
        return GetSystemTime();
    else
        return time.load();
}

void TimeManager::SetTime(long value)
{
    time.store(value);

    mtx.lock();
    auto it = reminders.begin();
    while (it != reminders.end()) {
        if (*it <= value) {
            notifiers.push_back(*it);
            it = reminders.erase(it);
        } else {
            break;
        }
    }
    mtx.unlock();

    if (notifiers.empty() == false) {
        for (long timestamp : notifiers)
            hld->strat->OnReminder(timestamp);
        notifiers.clear();
    }
}

void TimeManager::AddReminder(long timestamp)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = reminders.find(timestamp);
    if (it == reminders.end())
        reminders.insert(timestamp);
}

void TimeManager::DelReminder(long timestamp)
{
    std::lock_guard<std::mutex> lock(mtx);
    auto it = reminders.find(timestamp);
    if (it != reminders.end())
        reminders.erase(it);
}
