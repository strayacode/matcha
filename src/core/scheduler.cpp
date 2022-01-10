#include <core/scheduler.h>

void Scheduler::Reset() {
    events.clear();

    current_time = 0;
}

void Scheduler::Tick(int cycles) {
    current_time += cycles;
}

void Scheduler::ResetCurrentTime() {
    // this will be ran at the start of each frame
    current_time = 0;
}

auto Scheduler::GetCurrentTime() -> u64 {
    return current_time;
}

auto Scheduler::GetEventTime() -> u64 {
    return events[0].start_time;
}

auto Scheduler::CalculateEventIndex(Event& new_event) -> int {
    int lower_bound = 0;
    int upper_bound = events.size() - 1;
   
    while (lower_bound <= upper_bound) {
        int mid = (lower_bound + upper_bound) / 2;
        
        if (new_event.start_time > events[mid].start_time) {
            lower_bound = mid + 1;
        } else {
            upper_bound = mid - 1;
        }
    }

    return lower_bound;
}

void Scheduler::RunEvents() {
    // do any scheduler events that are meant to happen at the current moment
    while (events[0].start_time <= GetCurrentTime() && events.size() > 0) {
        // do the callback associated with that scheduler event
        events[0].callback();
        
        // remove the event from the priority queue
        events.erase(events.begin());
    }
}

void Scheduler::Add(u64 delay, std::function<void()> callback) {
    Event new_event;
    new_event.callback = callback;
    new_event.id = NoneEvent;
    new_event.start_time = GetCurrentTime() + delay;
    int index = CalculateEventIndex(new_event);

    events.insert(events.begin() + index, new_event);
}

void Scheduler::AddWithId(u64 delay, int id, std::function<void()> callback) {
    Event new_event;
    new_event.callback = callback;
    new_event.id = id;
    new_event.start_time = GetCurrentTime() + delay;
    int index = CalculateEventIndex(new_event);

    events.insert(events.begin() + index, new_event);
}

void Scheduler::Cancel(int id) {
    for (u64 i = 0; i < events.size(); i++) {
        if (events[i].id == id) {
            events.erase(events.begin() + i);
        }
    }
}

void Scheduler::SchedulerDebug() {
    for (Event event : events) {
        printf("start time: %ld, id: %d\n", event.start_time, event.id);
    }
}

int Scheduler::CalculateEECycles() {
    // by default just do 1 cycle
    // however later we can optimise this by running more cycles at once if there are no events queued
    return 1;
}

int Scheduler::CalculateBusCycles() {
    // return 1 if we are on an odd cycle (meaning 2 cycles of the ee have occured)
    // other 0
    return GetCurrentTime() & 0x1;
}

int Scheduler::CalculateIOPCycles() {
    // the iop runs 1 cycle for each ee cycle
    // TODO: make this more efficient
    if ((GetCurrentTime() & 0x7) == 0 && GetCurrentTime()) {
        return 1;
    } else {
        return 0;
    }

    // if ((GetCurrentTime() % 8) == 0 && GetCurrentTime()) {
    //     return 1;
    // } else {
    //     return 0;
    // }
}