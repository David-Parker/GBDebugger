#ifndef TILE_RENDERER_H
#define TILE_RENDERER_H

#include <cstdint>
#include <array>
#include <vector>
#include <unordered_map>
#include "panels/VRAMViewerPanel.h"

namespace GBDebug {

/**
 * TileData - Container for decoded tile pixel data with metadata
 * 
 * Holds the 8x8 pixel array along with the tile index for identification.
 * Used for batch tile updates where multiple tiles need to be processed.
 */
struct TileData {
    int tileIndex;
    std::array<std::array<uint8_t, 8>, 8> pixels;
    
    TileData() : tileIndex(0) {}
    TileData(int index, const std::array<std::array<uint8_t, 8>, 8>& pixelData)
        : tileIndex(index), pixels(pixelData) {}
};

/**
 * TexturePool - Fixed-size pool of reusable OpenGL textures for grid display
 * 
 * Manages a 2D grid of pre-allocated textures that can be updated each frame
 * without creating/destroying textures. This prevents memory leaks from
 * continuous texture allocation during rendering.
 * 
 * Usage:
 *   TexturePool pool;
 *   pool.Initialize(24, 16, 2);  // 24 rows, 16 cols, scale 2
 *   unsigned int tex = pool.GetTexture(row, col);
 *   pool.UpdateTexture(row, col, rgbaData);
 */
class TexturePool {
public:
    TexturePool();
    ~TexturePool();
    
    /**
     * Initialize the texture pool with a fixed grid size
     * 
     * @param rows Number of rows in the grid
     * @param cols Number of columns in the grid
     * @param scale Scale factor for each tile (1 = 8x8, 2 = 16x16, etc.)
     */
    void Initialize(int rows, int cols, int scale);
    
    /**
     * Get the texture ID for a specific grid position
     * 
     * @param row Row index (0-based)
     * @param col Column index (0-based)
     * @return OpenGL texture ID, or 0 if position is invalid
     */
    unsigned int GetTexture(int row, int col) const;
    
    /**
     * Update the texture at a specific grid position with new pixel data
     * 
     * @param row Row index (0-based)
     * @param col Column index (0-based)
     * @param rgbaData RGBA pixel data (must match texture size)
     */
    void UpdateTexture(int row, int col, const uint8_t* rgbaData);
    
    /**
     * Check if the pool is initialized
     */
    bool IsInitialized() const { return initialized_; }
    
    /**
     * Get the current scale factor
     */
    int GetScale() const { return scale_; }
    
    /**
     * Get the grid dimensions
     */
    int GetRows() const { return rows_; }
    int GetCols() const { return cols_; }
    
    /**
     * Clear and deallocate all textures
     */
    void Clear();
    
    /**
     * Reinitialize if parameters changed
     * 
     * @return true if reinitialization occurred
     */
    bool ReinitializeIfNeeded(int rows, int cols, int scale);

private:
    std::vector<unsigned int> textures_;  // Flat array of texture IDs
    int rows_;
    int cols_;
    int scale_;
    bool initialized_;
    
    int GetIndex(int row, int col) const;
};

/**
 * TileRenderer - Manages OpenGL textures for tile display in the VRAM viewer
 * 
 * Responsible for creating, updating, and caching OpenGL textures that represent
 * decoded Game Boy tiles. Uses fixed-size texture pools to prevent memory leaks
 * from continuous texture allocation. Converts tile pixel data (color indices 0-3)
 * to RGBA textures using the provided palette.
 * 
 * Key features:
 * - Uses TexturePool for fixed-size grids (tile grid, sprite grid, inspector)
 * - Supports scaling tiles for display (default 2x = 16x16 pixels)
 * - Updates existing textures in-place rather than creating new ones
 * - Provides separate pools for tile grid, sprites, and inspector views
 * 
 * The renderer uses OpenGL for texture management and is designed to work
 * with ImGui's image rendering functions.
 * 
 * Usage:
 *   TileRenderer renderer;
 *   // Initialize pools for your grid sizes
 *   renderer.InitializeTileGridPool(24, 16, 2);  // 384 tiles, 16 per row
 *   
 *   // Render a tile at a grid position (updates existing texture)
 *   unsigned int texture = renderer.RenderTileAt(row, col, pixelData, palette);
 *   // Use texture with ImGui::Image()
 *   
 *   // Cleanup when done
 *   renderer.ClearTextures();
 */
class TileRenderer {
public:
    TileRenderer();
    ~TileRenderer();
    
    // ========== Texture Pool Management ==========
    
    /**
     * Initialize the tile grid texture pool
     * 
     * Creates a fixed pool of textures for the main tile grid display.
     * Call this before rendering tiles. Will reinitialize if parameters change.
     * 
     * @param rows Number of rows in the tile grid
     * @param cols Number of columns (typically 16)
     * @param scale Scale factor for tiles (2 = 16x16 display)
     */
    void InitializeTileGridPool(int rows, int cols, int scale);
    
    /**
     * Initialize the sprite grid texture pool
     * 
     * Creates a fixed pool of textures for sprite display.
     * Supports both 8x8 and 8x16 sprite modes.
     * 
     * @param maxSprites Maximum number of sprites (typically 40)
     * @param scale Scale factor for sprites
     */
    void InitializeSpritePool(int maxSprites, int scale);
    
    /**
     * Initialize the inspector texture pool
     * 
     * Creates textures for the tile inspector view (enlarged tile preview).
     * 
     * @param scale Scale factor for inspector (typically 8 for 64x64)
     */
    void InitializeInspectorPool(int scale);
    
    // ========== Grid-Based Rendering (Preferred) ==========
    
    /**
     * Render a tile at a specific grid position
     * 
     * Updates the texture at the given grid position with new pixel data.
     * This is the preferred method for rendering tiles in the grid view.
     * 
     * @param row Row index in the tile grid
     * @param col Column index in the tile grid
     * @param pixelData 8x8 array of color indices (0-3)
     * @param palette Palette containing 4 colors for mapping indices
     * @return OpenGL texture ID for use with ImGui::Image()
     */
    unsigned int RenderTileAt(
        int row, int col,
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette
    );
    
    /**
     * Render a sprite at a specific index
     * 
     * Updates the texture for a sprite at the given index.
     * 
     * @param spriteIndex Index of the sprite (0-39)
     * @param pixelData 8x8 array of color indices (0-3)
     * @param palette Palette containing 4 colors for mapping indices
     * @param isBottomHalf For 8x16 mode, true if this is the bottom tile
     * @return OpenGL texture ID for use with ImGui::Image()
     */
    unsigned int RenderSpriteAt(
        int spriteIndex,
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette,
        bool isBottomHalf = false
    );
    
    /**
     * Render a tile in the inspector view
     * 
     * Updates the inspector texture with an enlarged tile preview.
     * 
     * @param pixelData 8x8 array of color indices (0-3)
     * @param palette Palette containing 4 colors for mapping indices
     * @return OpenGL texture ID for use with ImGui::Image()
     */
    unsigned int RenderInspectorTile(
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette
    );
    
    // ========== Legacy Methods (for compatibility) ==========
    
    /**
     * Render a single tile to an OpenGL texture (LEGACY - creates new texture)
     * 
     * WARNING: This method creates a new texture each call. Use RenderTileAt()
     * for grid rendering to avoid memory leaks.
     * 
     * @deprecated Use RenderTileAt() for grid rendering
     */
    unsigned int RenderTile(
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette,
        int scale = 1
    );
    
    /**
     * Batch update multiple tiles
     */
    void UpdateTiles(
        const std::vector<TileData>& tiles,
        const Palette& palette
    );
    
    /**
     * Get the cached texture for a tile
     */
    unsigned int GetTileTexture(int tileIndex) const;
    
    /**
     * Mark a tile as needing update
     */
    void MarkTileDirty(int tileIndex);
    
    /**
     * Mark all tiles as needing update
     */
    void MarkAllDirty();
    
    /**
     * Check if a tile needs update
     */
    bool IsTileDirty(int tileIndex) const;
    
    /**
     * Clear all cached textures and pools
     */
    void ClearTextures();
    
    /**
     * Get the number of cached textures
     */
    size_t GetCacheSize() const;

private:
    /**
     * Create a new OpenGL texture
     */
    unsigned int CreateTexture(int width, int height);
    
    /**
     * Update an existing texture with new pixel data
     */
    void UpdateTexture(unsigned int texture, const uint8_t* data, int width, int height);
    
    /**
     * Convert tile pixel data to RGBA buffer
     */
    std::vector<uint8_t> ConvertToRGBA(
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette,
        int scale
    );
    
    // Texture pools for different views
    TexturePool tileGridPool_;      // Main tile grid (24 rows x 16 cols for 384 tiles)
    TexturePool spritePool_;        // Sprite display (40 sprites, 2 textures each for 8x16)
    TexturePool inspectorPool_;     // Inspector view (1 texture at larger scale)
    
    // Legacy texture cache (for backward compatibility)
    std::unordered_map<int, unsigned int> tileTextures_;
    std::unordered_map<int, bool> dirtyFlags_;
    
    // Reusable buffer for texture data to avoid allocations
    std::vector<uint8_t> textureBuffer_;
    
    // Current scale factor (cached for consistency)
    int currentScale_;
};

} // namespace GBDebug

#endif // TILE_RENDERER_H
