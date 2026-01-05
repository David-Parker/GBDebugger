#include "../include/GBDebugger.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace GBDebug;

void testLifecycleMethods() {
    std::cout << "Testing lifecycle methods..." << std::endl;
    
    GBDebugger debugger;
    
    // Test initial state
    assert(debugger.IsOpen() == false);
    
    // Test Open()
    bool openResult = debugger.Open();
    assert(openResult == true);
    assert(debugger.IsOpen() == true);
    
    // Test multiple Open() calls (should be idempotent)
    bool openResult2 = debugger.Open();
    assert(openResult2 == true);
    assert(debugger.IsOpen() == true);
    
    // Test Close()
    debugger.Close();
    assert(debugger.IsOpen() == false);
    
    // Test multiple Close() calls (should be safe)
    debugger.Close();
    assert(debugger.IsOpen() == false);
    
    // Test reopening after close
    bool openResult3 = debugger.Open();
    assert(openResult3 == true);
    assert(debugger.IsOpen() == true);
    
    debugger.Close();
    
    std::cout << "  ✓ Lifecycle methods tests passed" << std::endl;
}

void testUpdateCPU() {
    std::cout << "Testing UpdateCPU() method..." << std::endl;
    
    GBDebugger debugger;
    
    // Test UpdateCPU before Open() - should work gracefully
    debugger.UpdateCPU(12345, 0x1234, 0xFFFE, 0xABF0, 0x1122, 0x3344, 0x5566, true);
    
    // Open debugger
    debugger.Open();
    
    // Test UpdateCPU after Open()
    debugger.UpdateCPU(67890, 0x5678, 0xFFF0, 0xCDF0, 0x7788, 0x99AA, 0xBBCC, false);
    
    debugger.Close();
    
    std::cout << "  ✓ UpdateCPU() tests passed" << std::endl;
}

void testUpdateMemory() {
    std::cout << "Testing UpdateMemory() method..." << std::endl;
    
    GBDebugger debugger;
    
    // Create a test memory buffer
    uint8_t buffer[65536];
    for (size_t i = 0; i < 65536; i++) {
        buffer[i] = static_cast<uint8_t>(i & 0xFF);
    }
    
    // Test UpdateMemory before Open() - should work gracefully
    bool result1 = debugger.UpdateMemory(buffer, 65536);
    assert(result1 == true);
    
    // Test with null buffer - should return false
    bool result2 = debugger.UpdateMemory(nullptr, 65536);
    assert(result2 == false);
    
    // Test with invalid size - should return false
    bool result3 = debugger.UpdateMemory(buffer, 1024);
    assert(result3 == false);
    
    // Test with invalid size (too large) - should return false
    bool result4 = debugger.UpdateMemory(buffer, 100000);
    assert(result4 == false);
    
    // Open debugger
    debugger.Open();
    
    // Test UpdateMemory after Open()
    bool result5 = debugger.UpdateMemory(buffer, 65536);
    assert(result5 == true);
    
    debugger.Close();
    
    std::cout << "  ✓ UpdateMemory() tests passed" << std::endl;
}

void testRender() {
    std::cout << "Testing Render() method..." << std::endl;
    
    GBDebugger debugger;
    
    // Test Render before Open() - should be safe
    debugger.Render();
    
    // Open debugger
    debugger.Open();
    
    // Test Render after Open()
    debugger.Render();
    
    // Close and test Render again
    debugger.Close();
    debugger.Render();
    
    std::cout << "  ✓ Render() tests passed" << std::endl;
}

int main() {
    std::cout << "Running API layer tests..." << std::endl;
    std::cout << std::endl;
    
    testLifecycleMethods();
    testUpdateCPU();
    testUpdateMemory();
    testRender();
    
    std::cout << std::endl;
    std::cout << "All API layer tests passed! ✓" << std::endl;
    
    return 0;
}
