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
    // Special handling for I/O Registers region
    if (region.start == 0xFF00 && region.end == 0xFF7F) {
        RenderIORegisters();
        return;
    }
    
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

void MemoryViewerPanel::RenderIORegisters() {
    // Helper lambda to render a single register
    auto renderRegister = [this](const IORegister& reg) {
        if (reg.start == reg.end) {
            // Single register
            uint8_t value = state_.Read(reg.start);
            ImGui::Text("$%04X  %02X  %-12s %s", 
                       reg.start, value, reg.name, reg.description);
        } else {
            // Register range (e.g., Wave RAM)
            ImGui::Text("$%04X-$%04X  %-12s %s", 
                       reg.start, reg.end, reg.name, reg.description);
            
            // Show the bytes in the range
            ImGui::Indent(20.0f);
            for (uint16_t addr = reg.start; addr <= reg.end; addr += 16) {
                uint16_t row_end = (addr + 15 > reg.end) ? reg.end : addr + 15;
                
                ImGui::Text("%04X: ", addr);
                ImGui::SameLine();
                
                char hex_line[64] = {0};
                int pos = 0;
                for (uint16_t a = addr; a <= row_end; a++) {
                    uint8_t byte = state_.Read(a);
                    pos += snprintf(hex_line + pos, 4, "%02X ", byte);
                }
                ImGui::Text("%s", hex_line);
            }
            ImGui::Unindent(20.0f);
        }
    };
    
    // Render non-sound registers first (before $FF10)
    for (size_t i = 0; i < IO_REGISTERS_COUNT; i++) {
        const IORegister& reg = IO_REGISTERS[i];
        if (reg.start < 0xFF10) {
            renderRegister(reg);
        }
    }
    
    // Sound registers in collapsible section ($FF10-$FF3F)
    if (ImGui::CollapsingHeader("Sound Registers ($FF10-$FF3F)")) {
        ImGui::Indent(10.0f);
        for (size_t i = 0; i < IO_REGISTERS_COUNT; i++) {
            const IORegister& reg = IO_REGISTERS[i];
            if (reg.start >= 0xFF10 && reg.end <= 0xFF3F) {
                renderRegister(reg);
            }
        }
        ImGui::Unindent(10.0f);
    }
    
    // PPU registers in collapsible section ($FF40-$FF4B)
    if (ImGui::CollapsingHeader("PPU Registers ($FF40-$FF4B)")) {
        ImGui::Indent(10.0f);
        for (size_t i = 0; i < IO_REGISTERS_COUNT; i++) {
            const IORegister& reg = IO_REGISTERS[i];
            if (reg.start >= 0xFF40 && reg.end <= 0xFF4B) {
                renderRegister(reg);
            }
        }
        ImGui::Unindent(10.0f);
    }
    
    // Render remaining registers (after $FF4B, excluding sound and PPU)
    for (size_t i = 0; i < IO_REGISTERS_COUNT; i++) {
        const IORegister& reg = IO_REGISTERS[i];
        if (reg.start > 0xFF4B) {
            renderRegister(reg);
        }
    }
    
    // Show unmapped/unused addresses in the I/O range (collapsed by default)
    if (ImGui::CollapsingHeader("Unmapped I/O Addresses")) {
        ImGui::Indent(10.0f);
        for (uint16_t addr = 0xFF00; addr <= 0xFF7F; addr++) {
            const IORegister* reg = FindIORegister(addr);
            if (reg == nullptr) {
                uint8_t value = state_.Read(addr);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
                                  "$%04X  %02X  (unmapped)", addr, value);
            }
        }
        ImGui::Unindent(10.0f);
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
