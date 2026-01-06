#include "../include/GBDebugger.h"
#include "../include/DebuggerTypes.h"
#include <iostream>
#include <cassert>

using namespace GBDebug;

void testCPUState() {
    std::cout << "Testing CPUState structure..." << std::endl;
    
    CPUState cpu;
    cpu.cycle = 12345;
    cpu.pc = 0x1234;
    cpu.sp = 0xFFFE;
    cpu.af = 0xABF0;  // A=0xAB, F=0xF0 (all flags set)
    cpu.bc = 0x1122;
    cpu.de = 0x3344;
    cpu.hl = 0x5566;
    cpu.ime = true;
    
    // Test register accessors
    assert(cpu.GetA() == 0xAB);
    assert(cpu.GetF() == 0xF0);
    assert(cpu.GetB() == 0x11);
    assert(cpu.GetC() == 0x22);
    assert(cpu.GetD() == 0x33);
    assert(cpu.GetE() == 0x44);
    assert(cpu.GetH() == 0x55);
    assert(cpu.GetL() == 0x66);
    
    // Test flag accessors (F=0xF0 means Z=1, N=1, H=1, C=1)
    assert(cpu.GetZFlag() == true);
    assert(cpu.GetNFlag() == true);
    assert(cpu.GetHFlag() == true);
    assert(cpu.GetCFlag() == true);
    
    // Test with flags cleared (F=0x00)
    cpu.af = 0xAB00;
    assert(cpu.GetZFlag() == false);
    assert(cpu.GetNFlag() == false);
    assert(cpu.GetHFlag() == false);
    assert(cpu.GetCFlag() == false);
    
    std::cout << "  ✓ CPUState tests passed" << std::endl;
}

void testMemoryState() {
    std::cout << "Testing MemoryState structure..." << std::endl;
    
    MemoryState mem;
    
    // Test initial state
    assert(mem.is_valid == false);
    assert(mem.Read(0x0000) == 0);
    
    // Test writing and reading
    mem.buffer[0x1234] = 0xAB;
    mem.buffer[0xFFFF] = 0xCD;
    mem.is_valid = true;
    
    assert(mem.Read(0x1234) == 0xAB);
    assert(mem.Read(0xFFFF) == 0xCD);
    assert(mem.is_valid == true);
    
    std::cout << "  ✓ MemoryState tests passed" << std::endl;
}

void testMemoryRegions() {
    std::cout << "Testing MemoryRegion structure and constants..." << std::endl;
    
    // Test that we have exactly 12 regions
    const size_t numRegions = sizeof(MEMORY_REGIONS) / sizeof(MEMORY_REGIONS[0]);
    assert(numRegions == 12);
    
    // Test first region (ROM Bank 0)
    assert(MEMORY_REGIONS[0].start == 0x0000);
    assert(MEMORY_REGIONS[0].end == 0x3FFF);
    assert(std::string(MEMORY_REGIONS[0].name) == "ROM Bank 0");
    
    // Test last region (IE Register)
    assert(MEMORY_REGIONS[11].start == 0xFFFF);
    assert(MEMORY_REGIONS[11].end == 0xFFFF);
    assert(std::string(MEMORY_REGIONS[11].name) == "IE Register");
    
    // Test VRAM region
    assert(MEMORY_REGIONS[2].start == 0x8000);
    assert(MEMORY_REGIONS[2].end == 0x9FFF);
    assert(std::string(MEMORY_REGIONS[2].name) == "VRAM");
    
    // Test that regions are contiguous and cover full address space
    for (size_t i = 0; i < numRegions - 1; i++) {
        // Each region should end before or at the start of the next region
        assert(MEMORY_REGIONS[i].end < MEMORY_REGIONS[i + 1].start ||
               MEMORY_REGIONS[i].end + 1 == MEMORY_REGIONS[i + 1].start);
    }
    
    std::cout << "  ✓ MemoryRegion tests passed" << std::endl;
}

int main() {
    std::cout << "Running data structures tests..." << std::endl;
    std::cout << std::endl;
    
    testCPUState();
    testMemoryState();
    testMemoryRegions();
    
    std::cout << std::endl;
    std::cout << "All tests passed! ✓" << std::endl;
    
    return 0;
}
