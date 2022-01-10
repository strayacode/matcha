#pragma once

#include <functional>
#include <vector>
#include <stdio.h>
#include <common/types.h>
#include <common/log.h>

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
    auto GetCurrentTime() -> u64;
    auto GetEventTime() -> u64;
    void ResetCurrentTime();
    void RunEvents();
    void Add(u64 delay, std::function<void()> callback);
    void AddWithId(u64 delay, int id, std::function<void()> callback);
    void Cancel(int id);
    auto CalculateEventIndex(Event& new_event) -> int;
    void SchedulerDebug();
    int CalculateEECycles();
    int CalculateBusCycles();
    int CalculateIOPCycles();

    constexpr static u64 ee_clock = 294912000;
    constexpr static u64 bus_clock = 147456000;
    constexpr static u64 iop_clock = 36864000;

private:
    u64 current_time;
    std::vector<Event> events;
};