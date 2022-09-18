#pragma once

#include "frontend/imgui/imgui.h"
#include "core/core.h"

class EEDebugger {
public:
    void RegistersWindow(EECore& ee_core);
    void DisassemblyWindow(Core& core);

    bool show_registers_window = false;
    bool show_disassembly_window = false;
    int disassembly_size = 15;
private:
};