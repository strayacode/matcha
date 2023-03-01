#include "frontend/debugger/ee.h"
#include "core/ee/disassembler.h"

void EEDebugger::RegistersWindow(ee::Context& ee) {
    ImGui::Begin("EE Registers");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    for (int i = 0; i < 32; i++) {
        ImGui::Text("%s", ee::GetRegisterName(i).c_str());
        ImGui::SameLine(90);
        ImGui::Text("%016llx", ee.GetReg<u64>(i));
    }

    ImGui::Text("pc");
    ImGui::SameLine(90);
    ImGui::Text("%016x", ee.pc);

    ImGui::Text("npc");
    ImGui::SameLine(90);
    ImGui::Text("%016x", ee.npc);

    ImGui::Text("lo");
    ImGui::SameLine(90);
    ImGui::Text("%016llx", ee.lo);

    ImGui::Text("hi");
    ImGui::SameLine(90);
    ImGui::Text("%016llx", ee.hi);

    ImGui::Text("lo1");
    ImGui::SameLine(90);
    ImGui::Text("%016llx", ee.lo1);

    ImGui::Text("hi1");
    ImGui::SameLine(90);
    ImGui::Text("%016llx", ee.hi1);

    ImGui::PopFont();
    ImGui::End();
}

void EEDebugger::DisassemblyWindow(Core& core) {
    ImGui::Begin("EE Disassembly");

    if (ImGui::Button("Single Step")) {
        core.system.SingleStep();
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    if (core.GetState() != CoreState::Idle) {
        u32 pc = core.system.ee.pc;
        u32 addr = pc - ((disassembly_size - 1) / 2) * 4;

        if ((addr >= 0x00000000 && addr < 0x2000000) || (addr >= 0x1fc00000 && addr < 0x20000000)) {
            for (int i = 0; i < disassembly_size; i++) {
                ee::Instruction inst = core.system.ee.Read<u32>(addr);

                if (addr == pc) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08x: %08x %s", addr, inst.data, ee::DisassembleInstruction(inst, addr).c_str());
                } else {
                    ImGui::Text("%08x: %08x %s", addr, inst.data, ee::DisassembleInstruction(inst, addr).c_str());
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