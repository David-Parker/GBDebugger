#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include <cstdint>
#include <array>
#include "panels/VRAMViewerPanel.h"

namespace GBDebug {

/**
 * PaletteManager - Manages color palettes for DMG and CGB modes
 * 
 * Responsible for handling palette storage, mode switching, and color conversion
 * between Game Boy native formats and display-ready RGB values. Supports both
 * the original Game Boy's grayscale palette and Game Boy Color's RGB555 palettes.
 * 
 * DMG Mode:
 * - Uses a fixed 4-shade grayscale palette
 * - Color index 0 = white, 1 = light gray, 2 = dark gray, 3 = black
 * 
 * CGB Mode:
 * - Supports 8 background palettes and 8 sprite palettes
 * - Each palette contains 4 colors in RGB555 format
 * - Colors are converted to RGB888 for display
 * 
 * Usage:
 *   PaletteManager palettes;
 *   palettes.SetMode(EmulationMode::CGB);
 *   palettes.SetBGPalettes(bgPaletteData, 8);
 *   Palette pal = palettes.GetBGPalette(0);
 */
class PaletteManager {
public:
    PaletteManager();
    ~PaletteManager() = default;
    
    /**
     * Set the emulation mode (DMG or CGB)
     * 
     * Switches between grayscale (DMG) and color (CGB) palette modes.
     * In DMG mode, GetBGPalette() and GetSpritePalette() return the
     * grayscale palette regardless of index.
     * 
     * @param mode EmulationMode::DMG or EmulationMode::CGB
     */
    void SetMode(EmulationMode mode);
    
    /**
     * Get the current emulation mode
     * 
     * @return Current EmulationMode (DMG or CGB)
     */
    EmulationMode GetMode() const;
    
    /**
     * Get the DMG grayscale palette
     * 
     * Returns the standard 4-shade grayscale palette used by the
     * original Game Boy:
     * - Color 0: White (255, 255, 255)
     * - Color 1: Light Gray (192, 192, 192)
     * - Color 2: Dark Gray (96, 96, 96)
     * - Color 3: Black (0, 0, 0)
     * 
     * @return Palette with 4 grayscale colors
     */
    Palette GetDMGPalette() const;
    
    /**
     * Convert a CGB RGB555 color to RGB888 format
     * 
     * CGB colors are stored in 15-bit RGB555 format (5 bits per channel).
     * This method converts to 8-bit RGB888 format for display.
     * 
     * RGB555 format: 0bbbbbgggggrrrrr (bits 0-4: red, 5-9: green, 10-14: blue)
     * 
     * @param cgbColor 16-bit value containing RGB555 color
     * @return TileColor struct with RGB888 values
     */
    TileColor ConvertCGBColor(uint16_t cgbColor) const;
    
    /**
     * Set background palettes from CGB palette data
     * 
     * Stores the provided CGB palettes and converts them to RGB888 format
     * for rendering. Up to 8 background palettes are supported.
     * 
     * @param palettes Pointer to array of CGBPalette structures
     * @param count Number of palettes to set (max 8)
     */
    void SetBGPalettes(const CGBPalette* palettes, int count);
    
    /**
     * Set sprite palettes from CGB palette data
     * 
     * Stores the provided CGB palettes and converts them to RGB888 format
     * for rendering. Up to 8 sprite palettes are supported.
     * 
     * @param palettes Pointer to array of CGBPalette structures
     * @param count Number of palettes to set (max 8)
     */
    void SetSpritePalettes(const CGBPalette* palettes, int count);
    
    /**
     * Get a background palette by index
     * 
     * In DMG mode, returns the grayscale palette regardless of index.
     * In CGB mode, returns the specified background palette (0-7).
     * 
     * @param index Palette index (0-7)
     * @return Palette with 4 colors
     */
    Palette GetBGPalette(int index) const;
    
    /**
     * Get a sprite palette by index
     * 
     * In DMG mode, returns the grayscale palette regardless of index.
     * In CGB mode, returns the specified sprite palette (0-7).
     * 
     * @param index Palette index (0-7)
     * @return Palette with 4 colors
     */
    Palette GetSpritePalette(int index) const;
    
    /**
     * Get the currently selected background palette index
     * 
     * @return Selected palette index (0-7)
     */
    int GetSelectedBGPalette() const;
    
    /**
     * Set the selected background palette index for preview
     * 
     * @param index Palette index (0-7)
     */
    void SetSelectedBGPalette(int index);
    
    /**
     * Get the currently selected sprite palette index
     * 
     * @return Selected palette index (0-7)
     */
    int GetSelectedSpritePalette() const;
    
    /**
     * Set the selected sprite palette index for preview
     * 
     * @param index Palette index (0-7)
     */
    void SetSelectedSpritePalette(int index);

private:
    EmulationMode mode_;
    Palette dmgPalette_;
    std::array<Palette, 8> bgPalettes_;
    std::array<Palette, 8> spritePalettes_;
    int selectedBGPalette_;
    int selectedSpritePalette_;
    
    /**
     * Initialize the DMG grayscale palette
     */
    void InitializeDMGPalette();
    
    /**
     * Convert a CGBPalette to a display-ready Palette
     * 
     * @param cgbPalette Source CGB palette in RGB555 format
     * @return Palette with RGB888 colors
     */
    Palette ConvertCGBPalette(const CGBPalette& cgbPalette) const;
};

} // namespace GBDebug

#endif // PALETTE_MANAGER_H
