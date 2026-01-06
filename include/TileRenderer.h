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
 * TileRenderer - Manages OpenGL textures for tile display in the VRAM viewer
 * 
 * Responsible for creating, updating, and caching OpenGL textures that represent
 * decoded Game Boy tiles. Converts tile pixel data (color indices 0-3) to RGBA
 * textures using the provided palette, and manages texture lifecycle.
 * 
 * Key features:
 * - Creates 8x8 RGBA textures for individual tiles
 * - Supports scaling tiles for display (default 4x = 32x32 pixels)
 * - Caches textures by tile index to avoid redundant texture creation
 * - Tracks dirty tiles to minimize texture uploads
 * - Provides batch update capability for efficient multi-tile updates
 * 
 * The renderer uses OpenGL for texture management and is designed to work
 * with ImGui's image rendering functions.
 * 
 * Usage:
 *   TileRenderer renderer;
 *   // Render a single tile
 *   GLuint texture = renderer.RenderTile(pixelData, palette, 4);
 *   // Use texture with ImGui::Image()
 *   
 *   // Batch update multiple tiles
 *   renderer.UpdateTiles(tileDataVector, palette);
 *   
 *   // Cleanup when done
 *   renderer.ClearTextures();
 */
class TileRenderer {
public:
    TileRenderer();
    ~TileRenderer();
    
    /**
     * Render a single tile to an OpenGL texture
     * 
     * Converts the 8x8 pixel data (color indices 0-3) to RGBA using the
     * provided palette, creates or updates a texture, and returns the
     * OpenGL texture ID for use with ImGui::Image().
     * 
     * @param pixelData 8x8 array of color indices (0-3)
     * @param palette Palette containing 4 colors for mapping indices
     * @param scale Scale factor for the texture (1 = 8x8, 4 = 32x32)
     * @return OpenGL texture ID (GLuint cast to unsigned int for portability)
     */
    unsigned int RenderTile(
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette,
        int scale = 1
    );
    
    /**
     * Batch update multiple tiles
     * 
     * Efficiently updates textures for multiple tiles at once. Each tile's
     * texture is created or updated based on the provided pixel data and
     * palette. Tiles are identified by their tileIndex in the TileData struct.
     * 
     * @param tiles Vector of TileData containing tile indices and pixel data
     * @param palette Palette to use for all tiles
     */
    void UpdateTiles(
        const std::vector<TileData>& tiles,
        const Palette& palette
    );
    
    /**
     * Get the cached texture for a tile
     * 
     * Returns the OpenGL texture ID for a previously rendered tile.
     * Returns 0 if the tile has not been rendered yet.
     * 
     * @param tileIndex Index of the tile
     * @return OpenGL texture ID, or 0 if not cached
     */
    unsigned int GetTileTexture(int tileIndex) const;
    
    /**
     * Mark a tile as needing update
     * 
     * Sets the dirty flag for a tile, indicating its texture should be
     * regenerated on the next render. Used when VRAM data changes.
     * 
     * @param tileIndex Index of the tile to mark dirty
     */
    void MarkTileDirty(int tileIndex);
    
    /**
     * Mark all tiles as needing update
     * 
     * Sets the dirty flag for all cached tiles. Used when palette changes
     * or when switching VRAM banks.
     */
    void MarkAllDirty();
    
    /**
     * Check if a tile needs update
     * 
     * @param tileIndex Index of the tile to check
     * @return true if the tile is marked dirty, false otherwise
     */
    bool IsTileDirty(int tileIndex) const;
    
    /**
     * Clear all cached textures
     * 
     * Deletes all OpenGL textures and clears the cache. Should be called
     * when the renderer is no longer needed or when a full refresh is required.
     */
    void ClearTextures();
    
    /**
     * Get the number of cached textures
     * 
     * @return Number of textures currently in the cache
     */
    size_t GetCacheSize() const;

private:
    /**
     * Create a new OpenGL texture
     * 
     * Creates an empty RGBA texture with the specified dimensions.
     * The texture is configured for nearest-neighbor filtering to
     * preserve pixel-perfect rendering of tile data.
     * 
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @return OpenGL texture ID
     */
    unsigned int CreateTexture(int width, int height);
    
    /**
     * Update an existing texture with new pixel data
     * 
     * Uploads RGBA pixel data to an existing OpenGL texture.
     * 
     * @param texture OpenGL texture ID
     * @param data Pointer to RGBA pixel data (4 bytes per pixel)
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     */
    void UpdateTexture(unsigned int texture, const uint8_t* data, int width, int height);
    
    /**
     * Convert tile pixel data to RGBA buffer
     * 
     * Converts 8x8 color indices to RGBA pixels using the palette,
     * optionally scaling the output.
     * 
     * @param pixelData 8x8 array of color indices (0-3)
     * @param palette Palette for color mapping
     * @param scale Scale factor (1 = 8x8 output, 4 = 32x32 output)
     * @return Vector of RGBA pixel data
     */
    std::vector<uint8_t> ConvertToRGBA(
        const std::array<std::array<uint8_t, 8>, 8>& pixelData,
        const Palette& palette,
        int scale
    );
    
    // Texture cache: maps tile index to OpenGL texture ID
    std::unordered_map<int, unsigned int> tileTextures_;
    
    // Dirty flags: tracks which tiles need texture updates
    std::unordered_map<int, bool> dirtyFlags_;
    
    // Reusable buffer for texture data to avoid allocations
    std::vector<uint8_t> textureBuffer_;
    
    // Current scale factor (cached for consistency)
    int currentScale_;
};

} // namespace GBDebug

#endif // TILE_RENDERER_H
