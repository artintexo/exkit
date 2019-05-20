#pragma once

#include "base.h"

#include <atomic>
#include <mutex>
#include <set>

class TimeManager {
public:
    TimeManager(Holder *hld);
    ~TimeManager();

    long GetTime() const;
    void SetTime(long value);
    void AddReminder(long timestamp);
    void DelReminder(long timestamp);

private:
    Holder *hld;
    std::atomic<long> time;
    std::mutex mtx;
    std::set<long> reminders;
    std::vector<long> notifiers;
};
