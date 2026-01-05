#include "GBDebugger.h"
#include "DebuggerBackend.h"
#include "DebuggerTypes.h"
#include "panels/CPUStatePanel.h"
#include "panels/FlagsPanel.h"
#include "panels/MemoryViewerPanel.h"

namespace GBDebug {

GBDebugger::GBDebugger()
    : backend_(new DebuggerBackend())
    , cpu_panel_(new CPUStatePanel())
    , flags_panel_(new FlagsPanel())
    , memory_panel_(new MemoryViewerPanel())
    , is_open_(false) {
}

GBDebugger::~GBDebugger() {
    Close();
}

bool GBDebugger::Open() {
    if (is_open_) {
        return true;
    }
    
    if (!backend_->Initialize("GBDebugger", 800, 600)) {
        return false;
    }
    
    is_open_ = true;
    return true;
}

void GBDebugger::Close() {
    if (!is_open_) {
        return;
    }
    
    backend_->Shutdown();
    is_open_ = false;
}

bool GBDebugger::IsOpen() const {
    return is_open_;
}

bool GBDebugger::ShouldClose() const {
    return backend_->ShouldClose();
}

void GBDebugger::ProcessSDLEvent(SDL_Event* event) {
    backend_->ProcessEvent(event);
}

void GBDebugger::BeginFrame() {
    if (is_open_) {
        backend_->BeginFrame();
    }
}

void GBDebugger::Render() {
    if (!is_open_) {
        return;
    }
    
    cpu_panel_->Render();
    flags_panel_->Render();
    memory_panel_->Render();
}

void GBDebugger::EndFrame() {
    if (is_open_) {
        backend_->EndFrame();
    }
}

void GBDebugger::UpdateCPU(uint64_t cycle, 
                           uint16_t pc, 
                           uint16_t sp,
                           uint16_t af, 
                           uint16_t bc, 
                           uint16_t de, 
                           uint16_t hl,
                           bool ime) {
    CPUState state;
    state.cycle = cycle;
    state.pc = pc;
    state.sp = sp;
    state.af = af;
    state.bc = bc;
    state.de = de;
    state.hl = hl;
    state.ime = ime;
    
    cpu_panel_->Update(state);
    flags_panel_->Update(state);
}

bool GBDebugger::UpdateMemory(const uint8_t* buffer, size_t size) {
    return memory_panel_->Update(buffer, size);
}

SDL_Window* GBDebugger::GetWindow() const {
    return backend_->GetWindow();
}

bool GBDebugger::InitSDL() {
    // Legacy compatibility - just call Open()
    return Open();
}

} // namespace GBDebug
