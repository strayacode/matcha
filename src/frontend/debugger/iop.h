#pragma once

#include "frontend/imgui/imgui.h"
#include "core/core.h"
#include "core/iop/context.h"

class IOPDebugger {
public:
    void RegistersWindow(iop::Context& iop);
    void DisassemblyWindow(Core& core);

    bool show_registers_window = false;
    bool show_disassembly_window = false;
    int disassembly_size = 15;
private:
};