#include "panels/MemoryViewerPanel.h"
#include "imgui.h"
#include <cstring>
#include <cstdio>

namespace GBDebug {

MemoryViewerPanel::MemoryViewerPanel()
    : visible_(true) {
}

bool MemoryViewerPanel::Update(const uint8_t* buffer, size_t size) {
    if (buffer == nullptr || size != 65536) {
        return false;
    }
    
    std::memcpy(state_.buffer.data(), buffer, size);
    state_.is_valid = true;
    return true;
}

void MemoryViewerPanel::Render() {
    if (!visible_) {
        return;
    }
    
    // Set initial window position and size (only on first use)
    ImGui::SetNextWindowPos(ImVec2(220, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(560, 580), ImGuiCond_FirstUseEver);
    
    ImGui::Begin(GetName(), nullptr, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (!state_.is_valid) {
        ImGui::Text("No memory data available");
        ImGui::End();
        return;
    }
    
    int current_region = -1;
    
    // Iterate through memory, 16 bytes per row
    for (uint32_t addr = 0; addr < 65536; addr += 16) {
        // Check for new memory region
        for (size_t i = 0; i < MEMORY_REGIONS_COUNT; i++) {
            if (addr == MEMORY_REGIONS[i].start) {
                current_region = static_cast<int>(i);
                
                if (addr > 0) {
                    ImGui::Separator();
                }
                
                const MemoryRegion& region = MEMORY_REGIONS[i];
                ImVec4 color(region.color.r, region.color.g, region.color.b, region.color.a);
                ImGui::TextColored(color, "%s (0x%04X - 0x%04X)", 
                                   region.name, region.start, region.end);
                ImGui::Separator();
                break;
            }
        }
        
        // Address
        ImGui::Text("%04X: ", addr);
        ImGui::SameLine();
        
        // Hex and ASCII representation
        char hex_line[64] = {0};
        char ascii_line[17] = {0};
        
        for (int i = 0; i < 16; i++) {
            uint16_t byte_addr = addr + i;
            uint8_t byte = state_.Read(byte_addr);
            
            snprintf(hex_line + (i * 3), 4, "%02X ", byte);
            ascii_line[i] = (byte >= 32 && byte <= 126) ? byte : '.';
        }
        
        ImGui::Text("%s", hex_line);
        ImGui::SameLine();
        ImGui::Text(" | %s", ascii_line);
    }
    
    ImGui::End();
}

} // namespace GBDebug
