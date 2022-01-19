#pragma once

#include <functional>
#include <vector>
#include <stdio.h>
#include "common/types.h"
#include "common/log.h"

enum EventId {
    NoneEvent,
    TimerEvent,
};

struct Event {
    u64 start_time;
    int id;
    std::function<void()> callback;
};

class Scheduler {
public:
    void Reset();
    void Tick(int cycles);
    u64 GetCurrentTime();
    u64 GetEventTime();
    void ResetCurrentTime();
    void RunEvents();
    void Add(u64 delay, std::function<void()> callback);
    void AddWithId(u64 delay, int id, std::function<void()> callback);
    void Cancel(int id);
    int CalculateEventIndex(Event& new_event);
    void SchedulerDebug();

private:
    u64 current_time;
    std::vector<Event> events;
};