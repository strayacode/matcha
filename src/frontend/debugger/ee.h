#pragma once

#include "frontend/imgui/imgui.h"
#include "core/core.h"
#include "core/ee/context.h"

class EEDebugger {
public:
    void RegistersWindow(ee::Context& ee);
    void DisassemblyWindow(Core& core);

    bool show_registers_window = false;
    bool show_disassembly_window = false;
    int disassembly_size = 15;
private:
};