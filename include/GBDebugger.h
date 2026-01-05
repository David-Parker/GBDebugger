#ifndef GBDEBUGGER_H
#define GBDEBUGGER_H

#include <cstdint>
#include <cstddef>
#include <array>

/**
 * GBDebugger - Emulator-agnostic GameBoy debugger using ImGui
 * 
 * This debugger provides a graphical interface for inspecting CPU state,
 * registers, flags, and memory during GameBoy emulation. It is designed
 * to work with any GameBoy emulator through a simple, standard C++ API.
 */

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
 * Compatible with ImGui's ImVec4 but defined independently to avoid ImGui dependency in header
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

/**
 * Main GBDebugger class
 * 
 * Provides an emulator-agnostic API for visualizing GameBoy state.
 * Usage:
 *   1. Create a GBDebugger instance
 *   2. Call Open() to initialize and show the debugger window
 *   3. Call UpdateCPU() and UpdateMemory() to provide state data
 *   4. Call Render() each frame to update the display
 *   5. Call Close() when done
 */
class GBDebugger {
public:
    GBDebugger();
    ~GBDebugger();
    
    // Lifecycle management
    
    /**
     * Open the debugger window and initialize ImGui
     * @return true if successful, false if initialization failed
     */
    bool Open();
    
    /**
     * Close the debugger window and cleanup resources
     */
    void Close();
    
    /**
     * Check if the debugger window is currently open
     * @return true if open, false otherwise
     */
    bool IsOpen() const;
    
    // State update methods
    
    /**
     * Update CPU state with current register values
     * Can be called before Open() - data will be stored but not rendered
     * 
     * @param cycle Current CPU cycle count
     * @param pc Program Counter value
     * @param sp Stack Pointer value
     * @param af AF register value (A in high byte, F in low byte)
     * @param bc BC register value
     * @param de DE register value
     * @param hl HL register value
     * @param ime Interrupt Master Enable flag state
     */
    void UpdateCPU(uint64_t cycle, 
                   uint16_t pc, 
                   uint16_t sp,
                   uint16_t af, 
                   uint16_t bc, 
                   uint16_t de, 
                   uint16_t hl,
                   bool ime);
    
    /**
     * Update memory state with current memory contents
     * Can be called before Open() - data will be stored but not rendered
     * 
     * @param buffer Pointer to 65536-byte memory buffer
     * @param size Size of buffer (must be 65536)
     * @return true if successful, false if buffer is null or size is invalid
     */
    bool UpdateMemory(const uint8_t* buffer, size_t size);
    
    /**
     * Render the debugger UI
     * Should be called each frame when the debugger is open
     * Safe to call when not open (will do nothing)
     */
    void Render();
    
private:
    CPUState cpu_state_;
    MemoryState memory_state_;
    bool is_open_;
    
    // Disable copy and assignment
    GBDebugger(const GBDebugger&) = delete;
    GBDebugger& operator=(const GBDebugger&) = delete;
};

} // namespace GBDebug

#endif // GBDEBUGGER_H
