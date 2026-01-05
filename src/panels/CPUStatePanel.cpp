#include "panels/CPUStatePanel.h"
#include "imgui.h"

namespace GBDebug {

CPUStatePanel::CPUStatePanel()
    : visible_(true) {
}

void CPUStatePanel::Update(const CPUState& state) {
    state_ = state;
}

void CPUStatePanel::Render() {
    if (!visible_) {
        return;
    }
    
    ImGui::Begin(GetName());
    
    // Cycle count
    ImGui::Text("Cycle: %llu (0x%llX)", 
                (unsigned long long)state_.cycle, 
                (unsigned long long)state_.cycle);
    
    ImGui::Separator();
    
    // Program Counter and Stack Pointer
    ImGui::Text("PC: 0x%04X", state_.pc);
    ImGui::Text("SP: 0x%04X", state_.sp);
    
    ImGui::Separator();
    
    // Register pairs
    ImGui::Text("AF: 0x%04X", state_.af);
    ImGui::Text("BC: 0x%04X", state_.bc);
    ImGui::Text("DE: 0x%04X", state_.de);
    ImGui::Text("HL: 0x%04X", state_.hl);
    
    ImGui::Separator();
    
    // IME flag
    ImGui::Text("IME: %s", state_.ime ? "Enabled" : "Disabled");
    
    ImGui::End();
}

} // namespace GBDebug
