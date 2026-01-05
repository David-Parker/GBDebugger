#ifndef DEBUGGER_TYPES_H
#define DEBUGGER_TYPES_H

#include <cstdint>
#include <array>

namespace GBDebug {

/**
 * CPU state structure containing all register values and flags
 */
struct CPUState {
    uint64_t cycle;  // Current CPU cycle count
    uint16_t pc;     // Program Counter
    uint16_t sp;     // Stack Pointer
    uint16_t af;     // Accumulator and Flags register
    uint16_t bc;     // BC register pair
    uint16_t de;     // DE register pair
    uint16_t hl;     // HL register pair
    bool ime;        // Interrupt Master Enable flag
    
    CPUState() : cycle(0), pc(0), sp(0), af(0), bc(0), de(0), hl(0), ime(false) {}
    
    // Computed flag accessors (from F register - lower byte of AF)
    bool GetZFlag() const { return (af & 0x80) != 0; }  // Zero flag (bit 7)
    bool GetNFlag() const { return (af & 0x40) != 0; }  // Subtraction flag (bit 6)
    bool GetHFlag() const { return (af & 0x20) != 0; }  // Half-carry flag (bit 5)
    bool GetCFlag() const { return (af & 0x10) != 0; }  // Carry flag (bit 4)
    
    // Individual register accessors
    uint8_t GetA() const { return (af >> 8) & 0xFF; }
    uint8_t GetF() const { return af & 0xFF; }
    uint8_t GetB() const { return (bc >> 8) & 0xFF; }
    uint8_t GetC() const { return bc & 0xFF; }
    uint8_t GetD() const { return (de >> 8) & 0xFF; }
    uint8_t GetE() const { return de & 0xFF; }
    uint8_t GetH() const { return (hl >> 8) & 0xFF; }
    uint8_t GetL() const { return hl & 0xFF; }
};

/**
 * Memory state structure containing the full 64KB GameBoy address space
 */
struct MemoryState {
    std::array<uint8_t, 65536> buffer;
    bool is_valid;
    
    MemoryState() : is_valid(false) {
        buffer.fill(0);
    }
    
    uint8_t Read(uint16_t address) const {
        return buffer[address];
    }
};

/**
 * Color structure for memory region visualization
 */
struct Color {
    float r, g, b, a;
    
    Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
};

/**
 * Memory region structure defining GameBoy memory map segments
 */
struct MemoryRegion {
    const char* name;
    uint16_t start;
    uint16_t end;
    Color color;
};

/**
 * GameBoy memory map regions (12 distinct regions)
 */
static const MemoryRegion MEMORY_REGIONS[] = {
    {"ROM Bank 0",      0x0000, 0x3FFF, Color(0.8f, 0.8f, 1.0f, 1.0f)},
    {"ROM Bank N",      0x4000, 0x7FFF, Color(0.7f, 0.7f, 1.0f, 1.0f)},
    {"VRAM",            0x8000, 0x9FFF, Color(1.0f, 0.8f, 0.8f, 1.0f)},
    {"External RAM",    0xA000, 0xBFFF, Color(0.8f, 1.0f, 0.8f, 1.0f)},
    {"WRAM Bank 0",     0xC000, 0xCFFF, Color(1.0f, 1.0f, 0.8f, 1.0f)},
    {"WRAM Bank N",     0xD000, 0xDFFF, Color(1.0f, 0.9f, 0.7f, 1.0f)},
    {"Echo RAM",        0xE000, 0xFDFF, Color(0.6f, 0.6f, 0.6f, 1.0f)},
    {"OAM",             0xFE00, 0xFE9F, Color(1.0f, 0.8f, 1.0f, 1.0f)},
    {"Unusable",        0xFEA0, 0xFEFF, Color(0.5f, 0.5f, 0.5f, 1.0f)},
    {"I/O Registers",   0xFF00, 0xFF7F, Color(0.8f, 1.0f, 1.0f, 1.0f)},
    {"HRAM",            0xFF80, 0xFFFE, Color(1.0f, 1.0f, 0.6f, 1.0f)},
    {"IE Register",     0xFFFF, 0xFFFF, Color(1.0f, 0.6f, 0.6f, 1.0f)}
};

static const size_t MEMORY_REGIONS_COUNT = sizeof(MEMORY_REGIONS) / sizeof(MEMORY_REGIONS[0]);

} // namespace GBDebug

#endif // DEBUGGER_TYPES_H
