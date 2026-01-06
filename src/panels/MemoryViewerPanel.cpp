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

void MemoryViewerPanel::RenderMemoryRegion(const MemoryRegion& region) {
    // Iterate through memory region, 16 bytes per row
    for (uint32_t addr = region.start; addr <= region.end; addr += 16) {
        // Calculate end of this row (don't go past region end)
        uint32_t row_end = (addr + 15 > region.end) ? region.end : addr + 15;
        int bytes_in_row = row_end - addr + 1;
        
        // Address
        ImGui::Text("%04X: ", addr);
        ImGui::SameLine();
        
        // Hex and ASCII representation
        char hex_line[64] = {0};
        char ascii_line[17] = {0};
        
        for (int i = 0; i < 16; i++) {
            if (i < bytes_in_row) {
                uint16_t byte_addr = addr + i;
                uint8_t byte = state_.Read(byte_addr);
                
                snprintf(hex_line + (i * 3), 4, "%02X ", byte);
                ascii_line[i] = (byte >= 32 && byte <= 126) ? byte : '.';
            } else {
                // Pad with spaces if row is incomplete
                snprintf(hex_line + (i * 3), 4, "   ");
                ascii_line[i] = ' ';
            }
        }
        ascii_line[bytes_in_row] = '\0';
        
        ImGui::Text("%s", hex_line);
        ImGui::SameLine();
        ImGui::Text(" | %s", ascii_line);
    }
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
    
    // Render each memory region as a collapsible section (default collapsed)
    for (size_t i = 0; i < MEMORY_REGIONS_COUNT; i++) {
        const MemoryRegion& region = MEMORY_REGIONS[i];
        
        // Create header label with address range
        char header[128];
        snprintf(header, sizeof(header), "%s (0x%04X - 0x%04X)", 
                 region.name, region.start, region.end);
        
        // Push color for the header text
        ImVec4 color(region.color.r, region.color.g, region.color.b, region.color.a);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        
        // Default to collapsed (no DefaultOpen flag)
        bool is_open = ImGui::CollapsingHeader(header);
        
        ImGui::PopStyleColor();
        
        if (is_open) {
            ImGui::Indent(10.0f);
            RenderMemoryRegion(region);
            ImGui::Unindent(10.0f);
        }
    }
    
    ImGui::End();
}

} // namespace GBDebug
