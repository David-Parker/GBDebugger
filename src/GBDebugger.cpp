#include "GBDebugger.h"
#include "DebuggerBackend.h"
#include "DebuggerTypes.h"
#include "panels/CPUStatePanel.h"
#include "panels/FlagsPanel.h"
#include "panels/MemoryViewerPanel.h"
#include "panels/ControlPanel.h"
#include "panels/VRAMViewerPanel.h"

namespace GBDebug {

GBDebugger::GBDebugger()
    : backend_(new DebuggerBackend())
    , cpu_panel_(new CPUStatePanel())
    , flags_panel_(new FlagsPanel())
    , memory_panel_(new MemoryViewerPanel())
    , control_panel_(new ControlPanel())
    , vram_panel_(new VRAMViewerPanel())
    , bankData_(new BankData())
    , is_open_(false) {
}

GBDebugger::~GBDebugger() {
    Close();
}

bool GBDebugger::Open() {
    if (is_open_) {
        return true;
    }
    
    if (!backend_->Initialize("GBDebugger", 900, 1200)) {
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
    control_panel_->Render();
    vram_panel_->Render();
}

void GBDebugger::EndFrame() {
    if (is_open_) {
        backend_->EndFrame();
    }
}

void GBDebugger::UpdateCPU(
    uint64_t cycle,
    uint16_t pc,
    uint16_t sp,
    uint16_t af,
    uint16_t bc,
    uint16_t de,
    uint16_t hl,
    bool ime
) {
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
    bool result = memory_panel_->Update(buffer, size);
    
    // Also update VRAM panel with the same memory buffer
    // VRAM panel will extract VRAM (0x8000-0x9FFF) and OAM (0xFE00-0xFE9F) from it
    if (result && buffer != nullptr && size == 65536) {
        // Auto-detect CGB mode from cartridge header (0x0143)
        // 0x80 = CGB compatible, 0xC0 = CGB only
        uint8_t cgbFlag = buffer[0x0143];
        if (cgbFlag == 0x80 || cgbFlag == 0xC0) {
            vram_panel_->SetEmulationMode(EmulationMode::CGB);
        } else {
            vram_panel_->SetEmulationMode(EmulationMode::DMG);
        }
        
        // Extract VRAM (0x8000-0x9FFF = 8KB)
        vram_panel_->UpdateVRAM(buffer + 0x8000, 8192, 0);
        
        // Extract OAM (0xFE00-0xFE9F = 160 bytes)
        vram_panel_->UpdateOAM(buffer + 0xFE00, 160);
    }
    
    return result;
}

bool GBDebugger::UpdateColorRAM(const uint8_t* bgPaletteRAM, const uint8_t* objPaletteRAM) {
    if (bgPaletteRAM == nullptr || objPaletteRAM == nullptr) {
        return false;
    }
    
    // Convert raw palette RAM bytes to CGBPalette structures
    // Each palette is 8 bytes (4 colors × 2 bytes per color in RGB555 format)
    CGBPalette bgPalettes[8];
    CGBPalette objPalettes[8];
    
    for (int pal = 0; pal < 8; pal++) {
        for (int col = 0; col < 4; col++) {
            int offset = pal * 8 + col * 2;
            // RGB555 is stored little-endian: low byte first, then high byte
            uint16_t color = bgPaletteRAM[offset] | (bgPaletteRAM[offset + 1] << 8);
            bgPalettes[pal].colors[col] = color;
            
            color = objPaletteRAM[offset] | (objPaletteRAM[offset + 1] << 8);
            objPalettes[pal].colors[col] = color;
        }
    }
    
    return vram_panel_->UpdatePalettes(bgPalettes, objPalettes);
}

bool GBDebugger::SetVRAMBank(uint8_t bank, const uint8_t* data) {
    // Validate bank number is 0 or 1
    if (bank > 1) {
        return false;
    }
    
    // Store pointer in BankData structure
    bankData_->vramBanks[bank] = data;
    
    // Set vramBanksProvided flag to true if at least one bank is provided
    if (data != nullptr) {
        bankData_->vramBanksProvided = true;
    }
    
    // Forward VRAM bank data to VRAMViewerPanel (Requirements 1.1, 1.2)
    // Pass both bank pointers so the panel can offer bank selection
    vram_panel_->SetVRAMBankData(bankData_->vramBanks[0], bankData_->vramBanks[1]);
    
    // Also update MemoryViewerPanel with bank data for VRAM region viewing
    memory_panel_->SetBankData(bankData_.get());
    
    return true;
}

bool GBDebugger::SetROMBanks(uint16_t count, std::function<const uint8_t*(uint16_t)> getBankFunc) {
    // Validate bank count is 1-512
    if (count < 1 || count > 512) {
        return false;
    }
    
    // Call function for each bank and store pointers
    for (uint16_t i = 0; i < count; i++) {
        bankData_->romBanks[i] = getBankFunc(i);
    }
    
    // Clear any remaining bank pointers
    for (uint16_t i = count; i < 512; i++) {
        bankData_->romBanks[i] = nullptr;
    }
    
    bankData_->romBankCount = count;
    bankData_->romBanksProvided = true;
    
    // Forward bank data to MemoryViewerPanel for ROM region viewing (Requirement 5.1)
    memory_panel_->SetBankData(bankData_.get());
    
    return true;
}

bool GBDebugger::SetRAMBanks(uint8_t count, size_t bankSize, 
                             std::function<const uint8_t*(uint8_t)> getBankFunc) {
    // Validate bank count is 0-16
    if (count > 16) {
        return false;
    }
    
    // Call function for each bank and store pointers
    for (uint8_t i = 0; i < count; i++) {
        bankData_->ramBanks[i] = getBankFunc(i);
    }
    
    // Clear any remaining bank pointers
    for (uint8_t i = count; i < 16; i++) {
        bankData_->ramBanks[i] = nullptr;
    }
    
    bankData_->ramBankCount = count;
    bankData_->ramBankSize = bankSize;
    bankData_->ramBanksProvided = true;
    
    // Forward bank data to MemoryViewerPanel for RAM region viewing (Requirement 7.1)
    memory_panel_->SetBankData(bankData_.get());
    
    return true;
}

void GBDebugger::ClearBankData() {
    // Reset all VRAM bank pointers to nullptr
    for (int i = 0; i < 2; i++) {
        bankData_->vramBanks[i] = nullptr;
    }
    bankData_->vramBanksProvided = false;
    
    // Reset all ROM bank pointers to nullptr
    for (int i = 0; i < 512; i++) {
        bankData_->romBanks[i] = nullptr;
    }
    bankData_->romBankCount = 0;
    bankData_->romBanksProvided = false;
    
    // Reset all RAM bank pointers to nullptr
    for (int i = 0; i < 16; i++) {
        bankData_->ramBanks[i] = nullptr;
    }
    bankData_->ramBankCount = 0;
    bankData_->ramBankSize = 0;
    bankData_->ramBanksProvided = false;
    
    // Clear panel bank data (Requirements 11.1, 11.2)
    // Clear VRAMViewerPanel external bank data
    vram_panel_->SetVRAMBankData(nullptr, nullptr);
    
    // Clear MemoryViewerPanel bank data (passing nullptr clears the reference)
    memory_panel_->SetBankData(nullptr);
}

SDL_Window* GBDebugger::GetWindow() const {
    return backend_->GetWindow();
}

bool GBDebugger::IsRunning() const {
    return control_panel_->IsRunning();
}

void GBDebugger::SetRunning(bool running) {
    control_panel_->SetRunning(running);
}

void GBDebugger::ToggleRunning() {
    control_panel_->ToggleRunning();
}

bool GBDebugger::IsStepRequested() const {
    return control_panel_->IsStepRequested();
}

void GBDebugger::ClearStepRequest() {
    control_panel_->ClearStepRequest();
}

bool GBDebugger::IsExitRequested() const {
    return control_panel_->IsExitRequested();
}

float GBDebugger::GetSpeedMultiplier() const {
    return control_panel_->GetSpeedMultiplier();
}

void GBDebugger::CycleSpeedUp() {
    control_panel_->CycleSpeedUp();
}

void GBDebugger::CycleSpeedDown() {
    control_panel_->CycleSpeedDown();
}

} // namespace GBDebug
