#ifndef GBDEBUGGER_H
#define GBDEBUGGER_H

#include <cstdint>
#include <cstddef>
#include <memory>

// Forward declarations
struct SDL_Window;
typedef void* SDL_GLContext;
union SDL_Event;

namespace GBDebug {

// Forward declarations for internal components
class DebuggerBackend;
class CPUStatePanel;
class FlagsPanel;
class MemoryViewerPanel;
class ControlPanel;
class VRAMViewerPanel;

// Forward declarations for VRAM viewer types
enum class EmulationMode;
struct CGBPalette;

/**
 * GBDebugger - Emulator-agnostic GameBoy debugger
 * 
 * This is the main public API for the debugger library. It provides a clean,
 * high-level interface for integrating debugging capabilities into any GameBoy
 * emulator. All ImGui and rendering details are hidden internally.
 * 
 * The debugger displays:
 * - CPU register values (PC, SP, AF, BC, DE, HL)
 * - CPU flags (Z, N, H, C) with visual indicators
 * - Full 64KB memory viewer with region highlighting
 * - Control panel with Run/Stop, Step, and Exit buttons
 * 
 * Architecture:
 * - GBDebugger (this class): Public API, coordinates panels and backend
 * - DebuggerBackend: SDL2/OpenGL window and ImGui frame management
 * - Panel classes: Individual UI components (CPUStatePanel, FlagsPanel, etc.)
 * 
 * Usage:
 *   GBDebugger debugger;
 *   debugger.Open();
 *   
 *   while (running) {
 *       debugger.ProcessSDLEvent(&event);
 *       debugger.UpdateCPU(cycle, pc, sp, af, bc, de, hl, ime);
 *       debugger.UpdateMemory(memoryBuffer, 65536);
 *       debugger.BeginFrame();
 *       debugger.Render();
 *       debugger.EndFrame();
 *       
 *       if (debugger.IsRunning()) { ... }
 *       if (debugger.IsStepRequested()) { ... }
 *       if (debugger.IsExitRequested()) { break; }
 *   }
 *   
 *   debugger.Close();
 */
class GBDebugger {
public:
    GBDebugger();
    ~GBDebugger();
    
    // ========== Lifecycle ==========
    
    /**
     * Open the debugger window
     * @return true if successful
     */
    bool Open();
    
    /**
     * Close the debugger and cleanup resources
     */
    void Close();
    
    /**
     * Check if debugger is open
     */
    bool IsOpen() const;
    
    /**
     * Check if the debugger window should close (user clicked X)
     */
    bool ShouldClose() const;
    
    // ========== Event Handling ==========
    
    /**
     * Process an SDL event
     * Should be called for each SDL event in your main loop
     */
    void ProcessSDLEvent(SDL_Event* event);
    
    // ========== Frame Management ==========
    
    /**
     * Begin a new frame - call before Render()
     */
    void BeginFrame();
    
    /**
     * Render all debugger panels
     */
    void Render();
    
    /**
     * End the frame - call after Render()
     */
    void EndFrame();
    
    // ========== State Updates ==========
    
    /**
     * Update CPU state
     */
    void UpdateCPU(
        uint64_t cycle,
        uint16_t pc,
        uint16_t sp,
        uint16_t af,
        uint16_t bc,
        uint16_t de,
        uint16_t hl,
        bool ime
    );
    
    /**
     * Update memory state
     * @param buffer Pointer to 64KB memory buffer
     * @param size Must be 65536
     * @return true if successful
     */
    bool UpdateMemory(const uint8_t* buffer, size_t size);
    
    // ========== Window Access ==========
    
    /**
     * Get the SDL window (for advanced use cases)
     */
    SDL_Window* GetWindow() const;
    
    // ========== Control State ==========
    
    /**
     * Check if emulator should be running
     */
    bool IsRunning() const;
    
    /**
     * Set running state
     */
    void SetRunning(bool running);
    
    /**
     * Toggle running state
     */
    void ToggleRunning();
    
    /**
     * Check if a single step was requested
     */
    bool IsStepRequested() const;
    
    /**
     * Clear the step request flag
     */
    void ClearStepRequest();
    
    /**
     * Check if exit was requested
     */
    bool IsExitRequested() const;
    
    /**
     * Get speed multiplier (0.125 to 8.0)
     */
    float GetSpeedMultiplier() const;
    
    /**
     * Increase speed: 1/8x -> 1/4x -> 1/2x -> 1x -> 2x -> 4x -> 8x
     */
    void CycleSpeedUp();
    
    /**
     * Decrease speed: 8x -> 4x -> 2x -> 1x -> 1/2x -> 1/4x -> 1/8x
     */
    void CycleSpeedDown();

private:
    std::unique_ptr<DebuggerBackend> backend_;
    std::unique_ptr<CPUStatePanel> cpu_panel_;
    std::unique_ptr<FlagsPanel> flags_panel_;
    std::unique_ptr<MemoryViewerPanel> memory_panel_;
    std::unique_ptr<ControlPanel> control_panel_;
    std::unique_ptr<VRAMViewerPanel> vram_panel_;
    bool is_open_;
    
    // Disable copy
    GBDebugger(const GBDebugger&) = delete;
    GBDebugger& operator=(const GBDebugger&) = delete;
};

} // namespace GBDebug

#endif // GBDEBUGGER_H
