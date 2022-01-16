#include "otterstation-imgui/debugger/ee.h"
#include "core/ee/disassembler.h"

void EEDebugger::RegistersWindow(EECore& ee_core) {
    ImGui::Begin("EE Registers");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    for (int i = 0; i < 32; i++) {
        ImGui::Text("%s", EEGetRegisterName(i).c_str());
        ImGui::SameLine(90);
        ImGui::Text("%016lx", ee_core.GetReg<u64>(i));
    }

    ImGui::Text("pc");
    ImGui::SameLine(90);
    ImGui::Text("%016x", ee_core.pc);

    ImGui::Text("npc");
    ImGui::SameLine(90);
    ImGui::Text("%016x", ee_core.next_pc);

    ImGui::Text("lo");
    ImGui::SameLine(90);
    ImGui::Text("%016lx", ee_core.lo);

    ImGui::Text("hi");
    ImGui::SameLine(90);
    ImGui::Text("%016lx", ee_core.hi);

    ImGui::Text("lo1");
    ImGui::SameLine(90);
    ImGui::Text("%016lx", ee_core.lo1);

    ImGui::Text("hi1");
    ImGui::SameLine(90);
    ImGui::Text("%016lx", ee_core.hi1);

    ImGui::PopFont();
    ImGui::End();
}

void EEDebugger::DisassemblyWindow(Core& core) {
    ImGui::Begin("EE Disassembly");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    if (core.GetState() != CoreState::Idle) {
        u32 pc = core.system.ee_core.pc;
        u32 addr = pc - ((disassembly_size - 1) / 2) * 4;

        if (core.system.memory.ValidEECodeRegion(addr)) {
            for (int i = 0; i < disassembly_size; i++) {
                CPUInstruction inst = core.system.ee_core.ReadWord(addr);

                if (addr == pc) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08x: %08x %s", addr, inst.data, EEDisassembleInstruction(inst, addr).c_str());
                } else {
                    ImGui::Text("%08x: %08x %s", addr, inst.data, EEDisassembleInstruction(inst, addr).c_str());
                }

                addr += 4;
            }
        } else {
            // ignore invalid addresses
            ImGui::Text("");
        }
    }

    ImGui::PopFont();
    ImGui::End();
}