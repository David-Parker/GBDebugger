#include "PaletteManager.h"
#include <algorithm>

namespace GBDebug {

PaletteManager::PaletteManager()
    : mode_(EmulationMode::DMG),
      selectedBGPalette_(0),
      selectedSpritePalette_(0) {
    InitializeDMGPalette();
    
    // Initialize CGB palettes to default (all black)
    for (int i = 0; i < 8; i++) {
        for (int c = 0; c < 4; c++) {
            bgPalettes_[i].colors[c] = TileColor(0, 0, 0, 255);
            spritePalettes_[i].colors[c] = TileColor(0, 0, 0, 255);
        }
    }
}

void PaletteManager::InitializeDMGPalette() {
    // DMG grayscale palette as specified in requirements:
    // Color 0: White (255, 255, 255)
    // Color 1: Light Gray (192, 192, 192)
    // Color 2: Dark Gray (96, 96, 96)
    // Color 3: Black (0, 0, 0)
    dmgPalette_.colors[0] = TileColor(255, 255, 255, 255);  // White
    dmgPalette_.colors[1] = TileColor(192, 192, 192, 255);  // Light Gray
    dmgPalette_.colors[2] = TileColor(96, 96, 96, 255);     // Dark Gray
    dmgPalette_.colors[3] = TileColor(0, 0, 0, 255);        // Black
}

void PaletteManager::SetMode(EmulationMode mode) {
    mode_ = mode;
}

EmulationMode PaletteManager::GetMode() const {
    return mode_;
}

Palette PaletteManager::GetDMGPalette() const {
    return dmgPalette_;
}

TileColor PaletteManager::ConvertCGBColor(uint16_t cgbColor) const {
    // CGB RGB555 format: 0bbbbbgggggrrrrr
    // Bits 0-4: Red (5 bits)
    // Bits 5-9: Green (5 bits)
    // Bits 10-14: Blue (5 bits)
    
    // Extract 5-bit components
    uint8_t r5 = cgbColor & 0x1F;
    uint8_t g5 = (cgbColor >> 5) & 0x1F;
    uint8_t b5 = (cgbColor >> 10) & 0x1F;
    
    // Convert 5-bit to 8-bit by shifting left 3 bits
    // This maps 0-31 to 0-248, then we add the top 3 bits
    // to the bottom 3 bits for better distribution (0-255)
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g5 << 3) | (g5 >> 2);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    
    return TileColor(r8, g8, b8, 255);
}

Palette PaletteManager::ConvertCGBPalette(const CGBPalette& cgbPalette) const {
    Palette result;
    for (int i = 0; i < 4; i++) {
        result.colors[i] = ConvertCGBColor(cgbPalette.colors[i]);
    }
    return result;
}

void PaletteManager::SetBGPalettes(const CGBPalette* palettes, int count) {
    if (palettes == nullptr) {
        return;
    }
    
    // Clamp count to valid range
    int actualCount = std::min(count, 8);
    
    for (int i = 0; i < actualCount; i++) {
        bgPalettes_[i] = ConvertCGBPalette(palettes[i]);
    }
}

void PaletteManager::SetSpritePalettes(const CGBPalette* palettes, int count) {
    if (palettes == nullptr) {
        return;
    }
    
    // Clamp count to valid range
    int actualCount = std::min(count, 8);
    
    for (int i = 0; i < actualCount; i++) {
        spritePalettes_[i] = ConvertCGBPalette(palettes[i]);
    }
}

Palette PaletteManager::GetBGPalette(int index) const {
    // In DMG mode, always return the grayscale palette
    if (mode_ == EmulationMode::DMG) {
        return dmgPalette_;
    }
    
    // In CGB mode, return the specified palette (clamped to valid range)
    int clampedIndex = std::max(0, std::min(index, 7));
    return bgPalettes_[clampedIndex];
}

Palette PaletteManager::GetSpritePalette(int index) const {
    // In DMG mode, always return the grayscale palette
    if (mode_ == EmulationMode::DMG) {
        return dmgPalette_;
    }
    
    // In CGB mode, return the specified palette (clamped to valid range)
    int clampedIndex = std::max(0, std::min(index, 7));
    return spritePalettes_[clampedIndex];
}

int PaletteManager::GetSelectedBGPalette() const {
    return selectedBGPalette_;
}

void PaletteManager::SetSelectedBGPalette(int index) {
    selectedBGPalette_ = std::max(0, std::min(index, 7));
}

int PaletteManager::GetSelectedSpritePalette() const {
    return selectedSpritePalette_;
}

void PaletteManager::SetSelectedSpritePalette(int index) {
    selectedSpritePalette_ = std::max(0, std::min(index, 7));
}

} // namespace GBDebug
