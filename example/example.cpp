#include "GBDebugger.h"
#include <iostream>
#include <vector>

/**
 * Simple example demonstrating GBDebugger API usage
 * 
 * This example shows how to:
 * 1. Create a GBDebugger instance
 * 2. Open the debugger window
 * 3. Update CPU state
 * 4. Update memory contents
 * 5. Render the debugger
 * 6. Close the debugger
 */

int main() {
    std::cout << "GBDebugger Example\n";
    std::cout << "==================\n\n";
    
    // Create debugger instance
    GBDebug::GBDebugger debugger;
    std::cout << "Created GBDebugger instance\n";
    
    // Open the debugger window
    if (debugger.Open()) {
        std::cout << "Debugger opened successfully\n";
    } else {
        std::cerr << "Failed to open debugger\n";
        return 1;
    }
    
    // Check if open
    if (debugger.IsOpen()) {
        std::cout << "Debugger is open\n";
    }
    
    // Example CPU state
    uint64_t cycle = 12345;
    uint16_t pc = 0x0150;
    uint16_t sp = 0xFFFE;
    uint16_t af = 0x01B0;  // A=0x01, F=0xB0 (Z=1, N=0, H=1, C=1)
    uint16_t bc = 0x0013;
    uint16_t de = 0x00D8;
    uint16_t hl = 0x014D;
    bool ime = true;
    
    // Update CPU state
    debugger.UpdateCPU(cycle, pc, sp, af, bc, de, hl, ime);
    std::cout << "Updated CPU state:\n";
    std::cout << "  Cycle: " << cycle << "\n";
    std::cout << "  PC: 0x" << std::hex << pc << "\n";
    std::cout << "  SP: 0x" << sp << "\n";
    std::cout << "  AF: 0x" << af << "\n";
    std::cout << "  BC: 0x" << bc << "\n";
    std::cout << "  DE: 0x" << de << "\n";
    std::cout << "  HL: 0x" << hl << "\n";
    std::cout << "  IME: " << (ime ? "enabled" : "disabled") << "\n";
    std::cout << std::dec;
    
    // Create sample memory buffer (64KB)
    std::vector<uint8_t> memory(65536, 0);
    
    // Fill with some sample data
    // ROM header area
    for (size_t i = 0x0100; i < 0x0150; i++) {
        memory[i] = static_cast<uint8_t>(i & 0xFF);
    }
    
    // Some sample code
    memory[0x0150] = 0x3E;  // LD A, n
    memory[0x0151] = 0x42;
    memory[0x0152] = 0xC3;  // JP nn
    memory[0x0153] = 0x00;
    memory[0x0154] = 0x01;
    
    // Update memory
    if (debugger.UpdateMemory(memory.data(), memory.size())) {
        std::cout << "Updated memory (64KB)\n";
    } else {
        std::cerr << "Failed to update memory\n";
    }
    
    // In a real application, you would call Render() in your main loop
    // For this example, we just demonstrate the API
    std::cout << "\nIn a real application, call debugger.Render() each frame\n";
    
    // Close the debugger
    debugger.Close();
    std::cout << "Debugger closed\n";
    
    // Verify it's closed
    if (!debugger.IsOpen()) {
        std::cout << "Debugger is closed\n";
    }
    
    std::cout << "\nExample completed successfully!\n";
    return 0;
}
