#pragma once

#include <string>
#include <common/emu_thread.h>
#include <core/system.h>

enum class CoreState {
    Running,
    Paused,
    Idle,
};

class Core {
public:
    Core(UpdateFunction update_fps);

    void Reset();
    void SetState(CoreState new_state);
    CoreState GetState();
    void RunFrame();
private:
    CoreState state = CoreState::Idle;

    EmuThread emu_thread;
    System system;
};