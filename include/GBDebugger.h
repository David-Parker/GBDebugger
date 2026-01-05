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

/**
 * GBDebugger - Emulator-agnostic GameBoy debugger
 * 
 * This is the main API class for the debugger. It provides a clean interface
 * for updating emulator state and rendering the debugger UI, while hiding
 * all ImGui and rendering details internally.
 * 
 * Usage:
 *   1. Create a GBDebugger instance
 *   2. Call Open() to initialize the debugger window
 *   3. In your main loop:
 *      - Call ProcessEvent() for each SDL event
 *      - Call UpdateCPU() and UpdateMemory() with current state
 *      - Call BeginFrame(), Render(), EndFrame() to draw
 *   4. Call Close() when done
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
    void UpdateCPU(uint64_t cycle, 
                   uint16_t pc, 
                   uint16_t sp,
                   uint16_t af, 
                   uint16_t bc, 
                   uint16_t de, 
                   uint16_t hl,
                   bool ime);
    
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
    
    // ========== Legacy Compatibility ==========
    
    /**
     * Initialize SDL backend (legacy - now called automatically by Open())
     * @deprecated Use Open() instead
     */
    bool InitSDL();

private:
    std::unique_ptr<DebuggerBackend> backend_;
    std::unique_ptr<CPUStatePanel> cpu_panel_;
    std::unique_ptr<FlagsPanel> flags_panel_;
    std::unique_ptr<MemoryViewerPanel> memory_panel_;
    bool is_open_;
    
    // Disable copy
    GBDebugger(const GBDebugger&) = delete;
    GBDebugger& operator=(const GBDebugger&) = delete;
};

} // namespace GBDebug

#endif // GBDEBUGGER_H
