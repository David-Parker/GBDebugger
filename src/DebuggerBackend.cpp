#include "DebuggerBackend.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace GBDebug {

DebuggerBackend::DebuggerBackend()
    : window_(nullptr)
    , gl_context_(nullptr)
    , initialized_(false)
    , should_close_(false) {
}

DebuggerBackend::~DebuggerBackend() {
    Shutdown();
}

bool DebuggerBackend::Initialize(const char* title, int width, int height) {
    if (initialized_) {
        return true;
    }
    
    // Create ImGui context
    ImGui::CreateContext();
    if (ImGui::GetCurrentContext() == nullptr) {
        return false;
    }
    
    // Set OpenGL attributes - use OpenGL 2.1 for maximum compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Create window
    Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    window_ = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        window_flags
    );
    
    if (!window_) {
        ImGui::DestroyContext();
        return false;
    }
    
    // Create OpenGL context
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        ImGui::DestroyContext();
        return false;
    }
    
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    
    // Initialize ImGui backends - use GLSL 120 for OpenGL 2.1 compatibility
    ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
    ImGui_ImplOpenGL3_Init("#version 120");
    
    // Build font atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int font_width, font_height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &font_width, &font_height);
    
    initialized_ = true;
    should_close_ = false;
    
    return true;
}

void DebuggerBackend::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    
    if (gl_context_) {
        SDL_GL_DeleteContext(gl_context_);
        gl_context_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    ImGui::DestroyContext();
    
    initialized_ = false;
    should_close_ = false;
}

void DebuggerBackend::ProcessEvent(SDL_Event* event) {
    if (!initialized_ || !event) {
        return;
    }
    
    // Check for window close
    if (event->type == SDL_WINDOWEVENT && 
        event->window.event == SDL_WINDOWEVENT_CLOSE &&
        window_ && 
        event->window.windowID == SDL_GetWindowID(window_)) {
        should_close_ = true;
    }
    
    // Forward to ImGui
    ImGui_ImplSDL2_ProcessEvent(event);
}

void DebuggerBackend::BeginFrame() {
    if (!initialized_) {
        return;
    }
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void DebuggerBackend::EndFrame() {
    if (!initialized_) {
        return;
    }
    
    ImGui::Render();
    
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    SDL_GL_SwapWindow(window_);
}

} // namespace GBDebug
