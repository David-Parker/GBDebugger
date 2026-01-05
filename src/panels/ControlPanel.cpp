#include "panels/ControlPanel.h"
#include "imgui.h"

namespace GBDebug {

ControlPanel::ControlPanel()
    : visible_(true)
    , running_(false)
    , step_requested_(false)
    , exit_requested_(false)
    , speed_index_(0) {
}

int ControlPanel::GetSpeedMultiplier() const {
    static const int multipliers[] = {1, 2, 4, 8};
    return multipliers[speed_index_];
}

void ControlPanel::CycleSpeed() {
    speed_index_ = (speed_index_ + 1) % 4;
}

void ControlPanel::Render() {
    if (!visible_) {
        return;
    }
    
    // Set initial window position and size (only on first use)
    ImGui::SetNextWindowPos(ImVec2(10, 390), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 140), ImGuiCond_FirstUseEver);
    
    ImGui::Begin(GetName());
    
    // Run/Stop button
    if (running_) {
        if (ImGui::Button("Stop (R)", ImVec2(180, 0))) {
            running_ = false;
        }
    } else {
        if (ImGui::Button("Run (R)", ImVec2(180, 0))) {
            running_ = true;
        }
    }
    
    // Step button (only enabled when not running)
    if (running_) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Step (S)", ImVec2(180, 0))) {
        step_requested_ = true;
    }
    if (running_) {
        ImGui::EndDisabled();
    }
    
    // Speed dropdown
    static const char* speed_labels[] = {"1x", "2x", "4x", "8x"};
    ImGui::SetNextItemWidth(130);
    if (ImGui::BeginCombo("Speed (T)", speed_labels[speed_index_])) {
        for (int i = 0; i < 4; i++) {
            bool is_selected = (speed_index_ == i);
            if (ImGui::Selectable(speed_labels[i], is_selected)) {
                speed_index_ = i;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    // Exit button
    if (ImGui::Button("Exit (ESC)", ImVec2(180, 0))) {
        exit_requested_ = true;
    }
    
    ImGui::End();
}

} // namespace GBDebug
