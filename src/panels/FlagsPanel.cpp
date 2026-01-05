#include "panels/FlagsPanel.h"
#include "imgui.h"

namespace GBDebug {

FlagsPanel::FlagsPanel()
    : visible_(true) {
}

void FlagsPanel::Update(const CPUState& state) {
    state_ = state;
}

void FlagsPanel::Render() {
    if (!visible_) {
        return;
    }
    
    ImGui::Begin(GetName());
    
    // Get flag values
    bool z_flag = state_.GetZFlag();
    bool n_flag = state_.GetNFlag();
    bool h_flag = state_.GetHFlag();
    bool c_flag = state_.GetCFlag();
    
    // Colors for set/clear states
    const ImVec4 set_color(0.0f, 1.0f, 0.0f, 1.0f);   // Green
    const ImVec4 clear_color(1.0f, 0.0f, 0.0f, 1.0f); // Red
    
    // Zero flag
    ImGui::Text("Z (Zero):      ");
    ImGui::SameLine();
    ImGui::TextColored(z_flag ? set_color : clear_color, z_flag ? "SET" : "CLEAR");
    
    // Subtract flag
    ImGui::Text("N (Subtract):  ");
    ImGui::SameLine();
    ImGui::TextColored(n_flag ? set_color : clear_color, n_flag ? "SET" : "CLEAR");
    
    // Half-carry flag
    ImGui::Text("H (Half-Carry):");
    ImGui::SameLine();
    ImGui::TextColored(h_flag ? set_color : clear_color, h_flag ? "SET" : "CLEAR");
    
    // Carry flag
    ImGui::Text("C (Carry):     ");
    ImGui::SameLine();
    ImGui::TextColored(c_flag ? set_color : clear_color, c_flag ? "SET" : "CLEAR");
    
    ImGui::End();
}

} // namespace GBDebug
