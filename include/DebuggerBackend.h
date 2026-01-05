#ifndef DEBUGGER_BACKEND_H
#define DEBUGGER_BACKEND_H

// Forward declarations for SDL types
struct SDL_Window;
typedef void* SDL_GLContext;
union SDL_Event;

namespace GBDebug {

/**
 * DebuggerBackend - Handles SDL2/OpenGL initialization and ImGui frame management
 * 
 * This class encapsulates all the low-level rendering setup, separating it
 * from the debugger's API and panel logic.
 */
class DebuggerBackend {
public:
    DebuggerBackend();
    ~DebuggerBackend();
    
    /**
     * Initialize the SDL2/OpenGL backend
     * Creates a window with OpenGL context and initializes ImGui backends
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @return true if successful
     */
    bool Initialize(const char* title = "GBDebugger", int width = 800, int height = 600);
    
    /**
     * Shutdown and cleanup all resources
     */
    void Shutdown();
    
    /**
     * Check if backend is initialized
     */
    bool IsInitialized() const { return initialized_; }
    
    /**
     * Process an SDL event
     * Forwards to ImGui and checks for window close
     * @param event SDL event to process
     */
    void ProcessEvent(SDL_Event* event);
    
    /**
     * Begin a new frame
     * Must be called before any ImGui rendering
     */
    void BeginFrame();
    
    /**
     * End the current frame
     * Renders ImGui and swaps buffers
     */
    void EndFrame();
    
    /**
     * Check if window close was requested
     */
    bool ShouldClose() const { return should_close_; }
    
    /**
     * Get the SDL window
     */
    SDL_Window* GetWindow() const { return window_; }
    
    /**
     * Get the OpenGL context
     */
    SDL_GLContext GetGLContext() const { return gl_context_; }

private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    bool initialized_;
    bool should_close_;
    
    // Disable copy
    DebuggerBackend(const DebuggerBackend&) = delete;
    DebuggerBackend& operator=(const DebuggerBackend&) = delete;
};

} // namespace GBDebug

#endif // DEBUGGER_BACKEND_H
