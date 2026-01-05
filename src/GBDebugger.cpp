#include "GBDebugger.h"
#include <cstring>

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
    // TODO: Initialize ImGui context and create window
    // For now, just mark as open
    if (is_open_) {
        // Already open, handle gracefully
        return true;
    }
    
    is_open_ = true;
    return true;
}

void GBDebugger::Close() {
    if (!is_open_) {
        return;
    }
    
    // TODO: Cleanup ImGui resources
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
    
    // TODO: Implement ImGui rendering
    // - CPU state panel
    // - Flags panel
    // - Memory viewer
}

} // namespace GBDebug
