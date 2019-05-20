#pragma once

#include "base.h"

#include <deque>
#include <mutex>

class EventQueue {
public:
    inline int GetSize()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return events.size();
    }

    inline void Enqueue(Event *event)
    {
        std::lock_guard<std::mutex> lock(mtx);
        events.push_back(event);
    }

    inline Event *Dequeue()
    {
        std::lock_guard<std::mutex> lock(mtx);
        Event *event = events.front();
        events.pop_front();
        return event;
    }

private:
    std::mutex mtx;
    std::deque<Event *> events;
};

class EventManager {
public:
    EventManager(Holder *hld);
    ~EventManager();

    inline void SetStarted(bool value) { started = value; }
    inline void PushExec(Event *event) { execQueue.Enqueue(event); }
    inline void PushData(Event *event) { dataQueue.Enqueue(event); }
    void Process();

private:
    friend class ExecSim;
    void OnNewExecReport(Event *event);
    void OnNewExecTrade(Event *event);
    void OnNewObook(Event *event);
    void OnNewTrade(Event *event);
    void OnNewSession(Event *event);
    void OnNewSecurity(Event *event);
    void OnNewHeartbeat(Event *event);
    void OnNewEndHistFile(Event *event);
    void OnNewEndHistData(Event *event);

    Holder *hld;
    bool started;
    EventQueue execQueue;
    EventQueue dataQueue;
};
