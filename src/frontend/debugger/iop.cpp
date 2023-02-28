#include "frontend/debugger/iop.h"
#include "core/iop/disassembler.h"

void IOPDebugger::RegistersWindow(iop::Context& iop) {
    ImGui::Begin("IOP Registers");

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("IOPTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("GPR")) {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

            for (int i = 0; i < 32; i++) {
                ImGui::Text("%s", iop::GetRegisterName(i).c_str());
                ImGui::SameLine(90);
                ImGui::Text("%08x", iop.GetReg(i));
            }

            ImGui::Text("pc");
            ImGui::SameLine(90);
            ImGui::Text("%08x", iop.pc);

            ImGui::Text("npc");
            ImGui::SameLine(90);
            ImGui::Text("%08x", iop.npc);

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("COP0")) {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

            ImGui::Text("status: %08x", iop.cop0.status.data);
            ImGui::Text("cause: %08x", iop.cop0.cause.data);
            ImGui::Text("epc: %08x", iop.cop0.epc);
            ImGui::Text("prid: %08x", iop.cop0.prid);

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void IOPDebugger::DisassemblyWindow(Core& core) {
    ImGui::Begin("IOP Disassembly");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    if (core.GetState() != CoreState::Idle) {
        u32 pc = core.system.iop.pc;
        u32 addr = pc - ((disassembly_size - 1) / 2) * 4;

        if (core.system.memory.ValidIOPCodeRegion(addr)) {
            for (int i = 0; i < disassembly_size; i++) {
                iop::Instruction inst = core.system.iop.Read<u32>(addr);

                if (addr == pc) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08x: %08x %s", addr, inst.data, iop::DisassembleInstruction(inst, addr).c_str());
                } else {
                    ImGui::Text("%08x: %08x %s", addr, inst.data, iop::DisassembleInstruction(inst, addr).c_str());
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