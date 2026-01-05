#include "GBDebugger.h"
#include <cstring>
#include <cstdio>
#include "imgui.h"

namespace GBDebug {

GBDebugger::GBDebugger() 
    : is_open_(false) {
    // Initialize CPU state to zero
    cpu_state_.cycle = 0;
    cpu_state_.pc = 0;
    cpu_state_.sp = 0;
    cpu_state_.af = 0;
    cpu_state_.bc = 0;
    cpu_state_.de = 0;
    cpu_state_.hl = 0;
    cpu_state_.ime = false;
}

GBDebugger::~GBDebugger() {
    Close();
}

bool GBDebugger::Open() {
    // Handle multiple Open() calls gracefully
    if (is_open_) {
        return true;
    }
    
    // Create ImGui context
    ImGui::CreateContext();
    
    // Check if context was created successfully
    if (ImGui::GetCurrentContext() == nullptr) {
        return false;
    }
    
    // Set up minimal ImGui IO state for headless rendering
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1920.0f, 1080.0f);  // Default display size
    io.DeltaTime = 1.0f / 60.0f;  // Default to 60 FPS
    
    // Build font atlas (required by ImGui)
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    is_open_ = true;
    return true;
}

void GBDebugger::Close() {
    if (!is_open_) {
        return;
    }
    
    // Destroy ImGui context
    ImGui::DestroyContext();
    
    is_open_ = false;
}

bool GBDebugger::IsOpen() const {
    return is_open_;
}

void GBDebugger::UpdateCPU(uint64_t cycle, 
                          uint16_t pc, 
                          uint16_t sp,
                          uint16_t af, 
                          uint16_t bc, 
                          uint16_t de, 
                          uint16_t hl,
                          bool ime) {
    // Store CPU state - works even if not open yet
    cpu_state_.cycle = cycle;
    cpu_state_.pc = pc;
    cpu_state_.sp = sp;
    cpu_state_.af = af;
    cpu_state_.bc = bc;
    cpu_state_.de = de;
    cpu_state_.hl = hl;
    cpu_state_.ime = ime;
}

bool GBDebugger::UpdateMemory(const uint8_t* buffer, size_t size) {
    // Validate input
    if (buffer == nullptr) {
        // Handle null buffer gracefully
        return false;
    }
    
    if (size != 65536) {
        // Handle invalid size gracefully
        return false;
    }
    
    // Copy memory buffer
    std::memcpy(memory_state_.buffer.data(), buffer, size);
    memory_state_.is_valid = true;
    
    return true;
}

void GBDebugger::Render() {
    if (!is_open_) {
        // Not open, nothing to render
        return;
    }
    
    // Start new ImGui frame
    ImGui::NewFrame();
    
    // Render all panels
    RenderCPUStatePanel();
    RenderFlagsPanel();
    RenderMemoryViewer();
    
    // Finalize ImGui frame
    ImGui::Render();
}

void GBDebugger::RenderCPUStatePanel() {
    ImGui::Begin("CPU State");
    
    // Display cycle count in both decimal and hexadecimal
    ImGui::Text("Cycle: %llu (0x%llX)", 
                (unsigned long long)cpu_state_.cycle, 
                (unsigned long long)cpu_state_.cycle);
    
    ImGui::Separator();
    
    // Display 16-bit registers in hexadecimal
    ImGui::Text("PC: 0x%04X", cpu_state_.pc);
    ImGui::Text("SP: 0x%04X", cpu_state_.sp);
    
    ImGui::Separator();
    
    ImGui::Text("AF: 0x%04X", cpu_state_.af);
    ImGui::Text("BC: 0x%04X", cpu_state_.bc);
    ImGui::Text("DE: 0x%04X", cpu_state_.de);
    ImGui::Text("HL: 0x%04X", cpu_state_.hl);
    
    ImGui::Separator();
    
    // Display IME flag state
    ImGui::Text("IME: %s", cpu_state_.ime ? "Enabled" : "Disabled");
    
    ImGui::End();
}

void GBDebugger::RenderFlagsPanel() {
    ImGui::Begin("CPU Flags");
    
    // Get flag values from CPU state
    bool z_flag = cpu_state_.GetZFlag();
    bool n_flag = cpu_state_.GetNFlag();
    bool h_flag = cpu_state_.GetHFlag();
    bool c_flag = cpu_state_.GetCFlag();
    
    // Display flags with visual indicators using colors
    // Set flag: green, Clear flag: red
    
    ImGui::Text("Z (Zero):      ");
    ImGui::SameLine();
    if (z_flag) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SET");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CLEAR");
    }
    
    ImGui::Text("N (Subtract):  ");
    ImGui::SameLine();
    if (n_flag) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SET");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CLEAR");
    }
    
    ImGui::Text("H (Half-Carry):");
    ImGui::SameLine();
    if (h_flag) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SET");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CLEAR");
    }
    
    ImGui::Text("C (Carry):     ");
    ImGui::SameLine();
    if (c_flag) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SET");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CLEAR");
    }
    
    ImGui::End();
}

void GBDebugger::RenderMemoryViewer() {
    ImGui::Begin("Memory Viewer", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (!memory_state_.is_valid) {
        ImGui::Text("No memory data available");
        ImGui::End();
        return;
    }
    
    // Use monospace font for better alignment
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Default font
    
    // Track current memory region for segmentation
    int current_region = -1;
    
    // Iterate through all 64KB of memory, 16 bytes per row
    for (uint32_t addr = 0; addr < 65536; addr += 16) {
        // Check if we've entered a new memory region
        for (size_t i = 0; i < sizeof(MEMORY_REGIONS) / sizeof(MEMORY_REGIONS[0]); i++) {
            if (addr == MEMORY_REGIONS[i].start) {
                current_region = i;
                
                // Add visual separator and region header
                if (addr > 0) {
                    ImGui::Separator();
                }
                
                // Display region name and address range with region-specific color
                const MemoryRegion& region = MEMORY_REGIONS[i];
                ImVec4 color(region.color.r, region.color.g, region.color.b, region.color.a);
                ImGui::TextColored(color, "%s (0x%04X - 0x%04X)", 
                                   region.name, region.start, region.end);
                ImGui::Separator();
                break;
            }
        }
        
        // Display address in 4-digit hexadecimal
        ImGui::Text("%04X: ", addr);
        ImGui::SameLine();
        
        // Display 16 bytes in hexadecimal
        char hex_line[64] = {0};
        char ascii_line[17] = {0};
        
        for (int i = 0; i < 16; i++) {
            uint16_t byte_addr = addr + i;
            uint8_t byte = memory_state_.Read(byte_addr);
            
            // Add to hex representation
            snprintf(hex_line + (i * 3), 4, "%02X ", byte);
            
            // Add to ASCII representation (printable characters only)
            if (byte >= 32 && byte <= 126) {
                ascii_line[i] = byte;
            } else {
                ascii_line[i] = '.';
            }
        }
        
        // Display hex values
        ImGui::Text("%s", hex_line);
        ImGui::SameLine();
        
        // Display ASCII representation
        ImGui::Text(" | %s", ascii_line);
    }
    
    ImGui::PopFont();
    ImGui::End();
}

} // namespace GBDebug
