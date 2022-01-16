#pragma once

#include "otterstation-imgui/imgui/imgui.h"
#include "core/core.h"

class IOPDebugger {
public:
    void RegistersWindow(IOPCore& iop_core);
    void DisassemblyWindow(Core& core);

    bool show_registers_window = false;
    bool show_disassembly_window = false;
    int disassembly_size = 15;
private:
};