#ifndef DEBUGGER_TYPES_H
#define DEBUGGER_TYPES_H

#include <cstdint>
#include <array>

namespace GBDebug {

/**
 * CPUState - Snapshot of GameBoy CPU register values
 * 
 * Contains all CPU register values and flags at a point in time.
 * Used to transfer state from the emulator to the debugger without
 * creating dependencies on emulator-specific types.
 * 
 * Provides accessor methods for individual registers and flag bits
 * extracted from the combined register pairs.
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
 * MemoryState - Snapshot of the full 64KB GameBoy address space
 * 
 * Holds a copy of the entire memory map for display in the debugger.
 * The buffer is copied from the emulator to avoid direct memory access
 * and maintain separation between debugger and emulator.
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
 * Color - RGBA color for UI rendering
 * 
 * Simple color structure compatible with ImGui's ImVec4 but defined
 * independently to avoid ImGui dependency in public headers.
 */
struct Color {
    float r, g, b, a;
    
    Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
};

/**
 * MemoryRegion - Defines a segment of the GameBoy memory map
 * 
 * Used by the memory viewer to display region boundaries and apply
 * color coding to different memory areas (ROM, RAM, I/O, etc.).
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

/**
 * IORegister - Defines a single I/O register or register range
 * 
 * Used by the memory viewer to display semantic names for I/O registers
 * in the $FF00-$FF7F range. Based on Pan Docs I/O ranges documentation.
 */
struct IORegister {
    const char* name;
    uint16_t start;
    uint16_t end;
    const char* description;
};

/**
 * GameBoy I/O register ranges (based on Pan Docs)
 * https://gbdev.io/pandocs/Memory_Map.html#io-ranges
 */
static const IORegister IO_REGISTERS[] = {
    {"P1/JOYP",     0xFF00, 0xFF00, "Joypad input"},
    {"SB",          0xFF01, 0xFF01, "Serial transfer data"},
    {"SC",          0xFF02, 0xFF02, "Serial transfer control"},
    {"DIV",         0xFF04, 0xFF04, "Divider register"},
    {"TIMA",        0xFF05, 0xFF05, "Timer counter"},
    {"TMA",         0xFF06, 0xFF06, "Timer modulo"},
    {"TAC",         0xFF07, 0xFF07, "Timer control"},
    {"IF",          0xFF0F, 0xFF0F, "Interrupt flag"},
    {"NR10",        0xFF10, 0xFF10, "Sound channel 1 sweep"},
    {"NR11",        0xFF11, 0xFF11, "Sound channel 1 length/duty"},
    {"NR12",        0xFF12, 0xFF12, "Sound channel 1 envelope"},
    {"NR13",        0xFF13, 0xFF13, "Sound channel 1 freq lo"},
    {"NR14",        0xFF14, 0xFF14, "Sound channel 1 freq hi"},
    {"NR21",        0xFF16, 0xFF16, "Sound channel 2 length/duty"},
    {"NR22",        0xFF17, 0xFF17, "Sound channel 2 envelope"},
    {"NR23",        0xFF18, 0xFF18, "Sound channel 2 freq lo"},
    {"NR24",        0xFF19, 0xFF19, "Sound channel 2 freq hi"},
    {"NR30",        0xFF1A, 0xFF1A, "Sound channel 3 on/off"},
    {"NR31",        0xFF1B, 0xFF1B, "Sound channel 3 length"},
    {"NR32",        0xFF1C, 0xFF1C, "Sound channel 3 output level"},
    {"NR33",        0xFF1D, 0xFF1D, "Sound channel 3 freq lo"},
    {"NR34",        0xFF1E, 0xFF1E, "Sound channel 3 freq hi"},
    {"NR41",        0xFF20, 0xFF20, "Sound channel 4 length"},
    {"NR42",        0xFF21, 0xFF21, "Sound channel 4 envelope"},
    {"NR43",        0xFF22, 0xFF22, "Sound channel 4 polynomial"},
    {"NR44",        0xFF23, 0xFF23, "Sound channel 4 control"},
    {"NR50",        0xFF24, 0xFF24, "Master volume"},
    {"NR51",        0xFF25, 0xFF25, "Sound panning"},
    {"NR52",        0xFF26, 0xFF26, "Sound on/off"},
    {"Wave RAM",    0xFF30, 0xFF3F, "Wave pattern RAM"},
    {"LCDC",        0xFF40, 0xFF40, "LCD control"},
    {"STAT",        0xFF41, 0xFF41, "LCD status"},
    {"SCY",         0xFF42, 0xFF42, "Scroll Y"},
    {"SCX",         0xFF43, 0xFF43, "Scroll X"},
    {"LY",          0xFF44, 0xFF44, "LCD Y coordinate"},
    {"LYC",         0xFF45, 0xFF45, "LY compare"},
    {"DMA",         0xFF46, 0xFF46, "OAM DMA transfer"},
    {"BGP",         0xFF47, 0xFF47, "BG palette data"},
    {"OBP0",        0xFF48, 0xFF48, "OBJ palette 0"},
    {"OBP1",        0xFF49, 0xFF49, "OBJ palette 1"},
    {"WY",          0xFF4A, 0xFF4A, "Window Y position"},
    {"WX",          0xFF4B, 0xFF4B, "Window X position"},
    {"KEY0",        0xFF4C, 0xFF4C, "CGB: Speed switch prep"},
    {"KEY1",        0xFF4D, 0xFF4D, "CGB: Speed switch"},
    {"VBK",         0xFF4F, 0xFF4F, "CGB: VRAM bank select"},
    {"BOOT",        0xFF50, 0xFF50, "Boot ROM disable"},
    {"HDMA1",       0xFF51, 0xFF51, "CGB: HDMA source hi"},
    {"HDMA2",       0xFF52, 0xFF52, "CGB: HDMA source lo"},
    {"HDMA3",       0xFF53, 0xFF53, "CGB: HDMA dest hi"},
    {"HDMA4",       0xFF54, 0xFF54, "CGB: HDMA dest lo"},
    {"HDMA5",       0xFF55, 0xFF55, "CGB: HDMA control"},
    {"RP",          0xFF56, 0xFF56, "CGB: IR port"},
    {"BCPS/BGPI",   0xFF68, 0xFF68, "CGB: BG palette index"},
    {"BCPD/BGPD",   0xFF69, 0xFF69, "CGB: BG palette data"},
    {"OCPS/OBPI",   0xFF6A, 0xFF6A, "CGB: OBJ palette index"},
    {"OCPD/OBPD",   0xFF6B, 0xFF6B, "CGB: OBJ palette data"},
    {"OPRI",        0xFF6C, 0xFF6C, "CGB: Object priority mode"},
    {"SVBK",        0xFF70, 0xFF70, "CGB: WRAM bank select"},
    {"PCM12",       0xFF76, 0xFF76, "CGB: Audio digital out 1&2"},
    {"PCM34",       0xFF77, 0xFF77, "CGB: Audio digital out 3&4"},
};

static const size_t IO_REGISTERS_COUNT = sizeof(IO_REGISTERS) / sizeof(IO_REGISTERS[0]);

/**
 * Helper function to find I/O register info for a given address
 * @param address Address in the I/O range ($FF00-$FF7F)
 * @return Pointer to IORegister if found, nullptr otherwise
 */
inline const IORegister* FindIORegister(uint16_t address) {
    for (size_t i = 0; i < IO_REGISTERS_COUNT; i++) {
        if (address >= IO_REGISTERS[i].start && address <= IO_REGISTERS[i].end) {
            return &IO_REGISTERS[i];
        }
    }
    return nullptr;
}

} // namespace GBDebug

#endif // DEBUGGER_TYPES_H
