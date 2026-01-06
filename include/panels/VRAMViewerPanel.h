#ifndef VRAM_VIEWER_PANEL_H
#define VRAM_VIEWER_PANEL_H

#include "IDebuggerPanel.h"
#include <cstdint>
#include <array>
#include <memory>

namespace GBDebug {

// Forward declarations
class TileDecoder;
class TileRenderer;
class PaletteManager;

/**
 * EmulationMode - Specifies the Game Boy hardware mode
 * 
 * Determines which features are available and how data should be interpreted:
 * - DMG: Original Game Boy (monochrome, single VRAM bank, grayscale palette)
 * - CGB: Game Boy Color (color, dual VRAM banks, RGB palettes)
 */
enum class EmulationMode {
    DMG,  // Original Game Boy (monochrome)
    CGB   // Game Boy Color
};

/**
 * TileColor - RGBA color value for tile rendering
 * 
 * Represents a single color in 8-bit RGBA format. Used for both DMG
 * grayscale palettes and CGB RGB palettes after conversion from RGB555.
 * Named TileColor to avoid conflict with Color in DebuggerTypes.h.
 */
struct TileColor {
    uint8_t r, g, b, a;
    
    TileColor() : r(0), g(0), b(0), a(255) {}
    TileColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) 
        : r(r_), g(g_), b(b_), a(a_) {}
};

/**
 * Palette - Collection of 4 colors for tile rendering
 * 
 * Game Boy tiles use 2-bit color indices (0-3), which map to one of
 * 4 colors in a palette. In DMG mode, this is a grayscale palette.
 * In CGB mode, this is one of 8 background or 8 sprite palettes.
 */
struct Palette {
    TileColor colors[4];
    
    Palette() = default;
};

/**
 * CGBPalette - Game Boy Color palette in native RGB555 format
 * 
 * CGB palettes store colors in 15-bit RGB555 format (5 bits per channel).
 * These need to be converted to RGB888 for display. Each palette contains
 * 4 colors, and there are 8 background palettes and 8 sprite palettes.
 */
struct CGBPalette {
    uint16_t colors[4];  // RGB555 format: 0bbbbbgggggrrrrr
    
    CGBPalette() {
        colors[0] = colors[1] = colors[2] = colors[3] = 0;
    }
};

/**
 * SpriteAttributes - Parsed OAM entry for a single sprite
 * 
 * Each sprite in OAM is defined by 4 bytes containing position, tile index,
 * and attribute flags. This struct holds the parsed values for easy access.
 * 
 * OAM Format (4 bytes per sprite):
 * - Byte 0: Y position (Y - 16 = screen Y)
 * - Byte 1: X position (X - 8 = screen X)
 * - Byte 2: Tile index
 * - Byte 3: Attribute flags
 */
struct SpriteAttributes {
    uint8_t y;           // Y position
    uint8_t x;           // X position
    uint8_t tileIndex;   // Tile index in VRAM
    uint8_t flags;       // Raw attribute flags
    
    // Parsed flag values
    bool priority;       // 0=above BG, 1=behind BG colors 1-3 (bit 7)
    bool yFlip;          // Vertical flip (bit 6)
    bool xFlip;          // Horizontal flip (bit 5)
    uint8_t paletteNumber;  // DMG: palette number (bit 4), CGB: VRAM bank (bit 3)
    uint8_t vramBank;    // CGB only: VRAM bank (bit 3)
    
    SpriteAttributes() 
        : y(0), x(0), tileIndex(0), flags(0),
          priority(false), yFlip(false), xFlip(false),
          paletteNumber(0), vramBank(0) {}
};

/**
 * VRAMViewerState - Current state of the VRAM viewer panel
 * 
 * Tracks the panel's UI state including mode, selected tile, bank selection,
 * and view options. This state persists across frames and determines what
 * is displayed and how user interactions are handled.
 */
struct VRAMViewerState {
    EmulationMode mode;      // Current emulation mode (DMG/CGB)
    uint8_t currentBank;     // Currently selected VRAM bank (0 or 1)
    int selectedTile;        // Index of selected tile (-1 if none)
    int selectedPalette;     // Index of selected palette for preview (0-7)
    bool showSprites;        // Whether to show sprite view
    bool showGrid;           // Whether to show grid lines
    int tileScale;           // Scale factor for tile display (1-8)
    bool needsRefresh;       // Flag indicating display needs update
    
    VRAMViewerState() 
        : mode(EmulationMode::DMG),
          currentBank(0),
          selectedTile(-1),
          selectedPalette(0),
          showSprites(false),
          showGrid(true),
          tileScale(2),
          needsRefresh(true) {}
};

/**
 * VRAMViewerPanel - Visual VRAM tile viewer for GBDebugger
 * 
 * Displays tiles stored in Video RAM in a scrollable grid, supporting both
 * DMG (monochrome) and CGB (color) modes. Provides tile inspection, sprite
 * viewing, and palette management. The panel is emulator-agnostic and receives
 * data through standard C++ buffer APIs.
 * 
 * Responsibilities:
 * - Store and manage VRAM data (8KB per bank, up to 2 banks)
 * - Store and manage OAM data (160 bytes, 40 sprites)
 * - Decode tiles from raw VRAM bytes to pixel data
 * - Render tiles with appropriate palettes (grayscale or color)
 * - Provide UI for tile selection, inspection, and sprite viewing
 * - Handle mode switching between DMG and CGB
 * 
 * Usage:
 *   1. Call SetEmulationMode() to set DMG or CGB mode
 *   2. Call UpdateVRAM() each frame with current VRAM contents
 *   3. Call UpdateOAM() each frame with current OAM contents
 *   4. Call UpdatePalettes() for CGB mode with palette data
 *   5. Call Render() each frame to draw the panel
 */
class VRAMViewerPanel : public IDebuggerPanel {
public:
    VRAMViewerPanel();
    ~VRAMViewerPanel() override;
    
    // IDebuggerPanel interface
    void Render() override;
    const char* GetName() const override;
    bool IsVisible() const override;
    void SetVisible(bool visible) override;
    
    /**
     * Update VRAM contents for a specific bank
     * 
     * @param buffer Pointer to 8192-byte VRAM buffer
     * @param size Size of buffer (must be 8192)
     * @param bank VRAM bank (0 or 1, only 0 used in DMG mode)
     * @return true if update successful, false on error
     */
    bool UpdateVRAM(const uint8_t* buffer, size_t size, uint8_t bank);
    
    /**
     * Update OAM (Object Attribute Memory) contents
     * 
     * @param buffer Pointer to 160-byte OAM buffer
     * @param size Size of buffer (must be 160)
     * @return true if update successful, false on error
     */
    bool UpdateOAM(const uint8_t* buffer, size_t size);
    
    /**
     * Update CGB color palettes
     * 
     * @param bgPalettes Pointer to 8 background palettes
     * @param spritePalettes Pointer to 8 sprite palettes
     * @return true if update successful, false on error
     */
    bool UpdatePalettes(const CGBPalette* bgPalettes, const CGBPalette* spritePalettes);
    
    /**
     * Set the emulation mode (DMG or CGB)
     * 
     * @param mode EmulationMode::DMG or EmulationMode::CGB
     */
    void SetEmulationMode(EmulationMode mode);

private:
    // Rendering methods
    void RenderTileGrid();
    void RenderSpriteView();
    void RenderTileInspector();
    
    // Helper components
    std::unique_ptr<TileDecoder> decoder_;
    std::unique_ptr<TileRenderer> renderer_;
    std::unique_ptr<PaletteManager> paletteManager_;
    
    // VRAM storage (8KB per bank)
    std::array<uint8_t, 8192> vramBank0_;
    std::array<uint8_t, 8192> vramBank1_;
    
    // OAM storage (160 bytes, 40 sprites × 4 bytes)
    std::array<uint8_t, 160> oam_;
    
    // CGB palette storage
    std::array<CGBPalette, 8> bgPalettes_;
    std::array<CGBPalette, 8> spritePalettes_;
    
    // Panel state
    VRAMViewerState state_;
    bool visible_;
};

} // namespace GBDebug

#endif // VRAM_VIEWER_PANEL_H
