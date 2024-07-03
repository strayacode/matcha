#include "host_interface.h"
#include "core/core.h"
#include "core/ee/context.h"
#include "core/ee/disassembler.h"
#include "core/iop/context.h"
#include "core/iop/disassembler.h"

void HostInterface::render_debugger_window() {
    ImGui::Begin("Debugger");

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("DebuggerTabs", tab_bar_flags)) {
        if (ImGui::BeginTabItem("EE")) {
            render_ee_debugger();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("IOP")) {
            render_iop_debugger();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void HostInterface::render_ee_debugger() {
    ee::Context& ee = core.system.ee;

    for (int i = 0; i < 16; i++) {
        int reg = i * 2;

        ImGui::Text("%s", ee::GetRegisterName(reg).c_str());
        ImGui::SameLine(90);

        u128 first = ee.GetReg<u128>(reg);

        ImGui::Text("%016llx%016llx", first.hi, first.lo);
        ImGui::SameLine(350);

        ImGui::Text("%s", ee::GetRegisterName(reg + 1).c_str());
        ImGui::SameLine(440);

        u128 second = ee.GetReg<u128>(reg + 1);

        ImGui::Text("%016llx%016llx", second.hi, second.lo);
    }

    ImGui::Text("pc");
    ImGui::SameLine(90);
    ImGui::Text("%08x", ee.pc);

    ImGui::Text("npc");
    ImGui::SameLine(90);
    ImGui::Text("%08x", ee.npc);

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

    ImGui::Text("sa");
    ImGui::SameLine(90);
    ImGui::Text("%016llx", ee.sa);

    ImGui::Separator();

    u32 pc = ee.pc;
    u32 addr = pc - ((m_ee_disassembly_size - 1) / 2) * 4;

    for (int i = 0; i < m_ee_disassembly_size; i++) {
        ee::Instruction inst = ee.read<u32>(addr);

        // Address must be a code address.
        if ((addr >= 0x00000000 && addr < 0x2000000) || (addr >= 0x1fc00000 && addr < 0x20000000)) {
            if (addr == pc) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08x: %08x %s", addr, inst.data, ee::DisassembleInstruction(inst, addr).c_str());
            } else {
                ImGui::Text("%08x: %08x %s", addr, inst.data, ee::DisassembleInstruction(inst, addr).c_str());
            } 
        } else {
            ImGui::Text("%08x: n/a n/a", addr);
        }

        addr += 4;
    }
}

void HostInterface::render_iop_debugger() {
    iop::Context& iop = core.system.iop;

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

    ImGui::Separator();

    ImGui::Text("status: %08x", iop.cop0.status.data);
    ImGui::Text("cause: %08x", iop.cop0.cause.data);
    ImGui::Text("epc: %08x", iop.cop0.epc);
    ImGui::Text("prid: %08x", iop.cop0.prid);

    ImGui::Separator();

    u32 pc = iop.pc;
    u32 addr = pc - ((m_iop_disassembly_size - 1) / 2) * 4;

    for (int i = 0; i < m_iop_disassembly_size; i++) {
        iop::Instruction inst = iop.Read<u32>(addr);

        // Address must be a code address.
        if ((addr >= 0x00000000 && addr < 0x200000) || (addr >= 0x1fc00000 && addr < 0x20000000)) {
            if (addr == pc) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "%08x: %08x %s", addr, inst.data, iop::DisassembleInstruction(inst, addr).c_str());
            } else {
                ImGui::Text("%08x: %08x %s", addr, inst.data, iop::DisassembleInstruction(inst, addr).c_str());
            }
        } else {
            ImGui::Text("%08x: n/a n/a", addr);
        }
        
        

        addr += 4;
    }
}