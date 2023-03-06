#include "core/core.h"

Core::Core(UpdateFunction update_fps) : emu_thread([this]() {
    RunFrame();
}, update_fps) {}

void Core::Reset() {
    system.Reset();
}

void Core::SetState(CoreState new_state) {
    switch (new_state) {
    case CoreState::Running:
        emu_thread.Start();
        break;
    case CoreState::Paused:
    case CoreState::Idle:
        emu_thread.Stop();
        break;
    }

    state = new_state;
}

CoreState Core::GetState() {
    return state;
}

void Core::RunFrame() {
    system.RunFrame();
}

void Core::SetBootParameters(BootMode boot_mode, std::string path) {
    system.SetBootParameters(boot_mode, path);
}

void Core::Boot() {
    SetState(CoreState::Idle);
    system.Reset();
    SetState(CoreState::Running);
}