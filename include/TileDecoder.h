#ifndef TILE_DECODER_H
#define TILE_DECODER_H

#include <cstdint>
#include <array>

namespace GBDebug {

/**
 * TileDecoder - Decodes Game Boy tile data from VRAM format to pixel arrays
 * 
 * Responsible for converting raw VRAM bytes into 8x8 arrays of color indices.
 * Game Boy tiles are stored in a specific 2-bit-per-pixel format where each
 * tile occupies 16 bytes (2 bytes per row, 8 rows per tile).
 * 
 * The decoding process:
 * - Each tile is 16 bytes (8 rows × 2 bytes per row)
 * - Each row uses 2 bytes: LSB byte and MSB byte
 * - Each pixel's color index is formed by combining bits from both bytes
 * - Color index = (MSB_bit << 1) | LSB_bit, resulting in values 0-3
 * 
 * Supports horizontal and vertical flipping for sprite rendering.
 * 
 * Usage:
 *   TileDecoder decoder;
 *   auto pixelData = decoder.DecodeTile(vramBuffer, tileIndex, bank);
 *   // pixelData is now an 8x8 array of color indices (0-3)
 */
class TileDecoder {
public:
    TileDecoder() = default;
    ~TileDecoder() = default;
    
    /**
     * Decode a complete tile from VRAM into an 8x8 pixel array
     * 
     * Reads 16 bytes from VRAM starting at the tile's address and decodes
     * them into an 8x8 array of color indices (0-3). Each color index
     * represents which of the 4 palette colors should be used for that pixel.
     * 
     * @param vram Pointer to VRAM buffer (8192 bytes)
     * @param tileIndex Tile index (0-511 for full VRAM)
     * @param bank VRAM bank (0 or 1, only used in CGB mode)
     * @return 8x8 array of color indices (0-3)
     */
    std::array<std::array<uint8_t, 8>, 8> DecodeTile(
        const uint8_t* vram,
        uint16_t tileIndex,
        uint8_t bank = 0
    );
    
    /**
     * Decode a single pixel from tile data
     * 
     * Extracts the color index for a specific pixel within a tile's raw data.
     * Supports horizontal and vertical flipping for sprite rendering.
     * 
     * @param tileData Pointer to 16-byte tile data
     * @param x X coordinate within tile (0-7)
     * @param y Y coordinate within tile (0-7)
     * @param hFlip Horizontal flip flag (mirrors tile left-right)
     * @param vFlip Vertical flip flag (mirrors tile top-bottom)
     * @return Color index (0-3)
     */
    uint8_t DecodePixel(
        const uint8_t* tileData,
        int x,
        int y,
        bool hFlip = false,
        bool vFlip = false
    );
    
    /**
     * Calculate the VRAM address for a given tile index
     * 
     * Game Boy tiles are stored sequentially in VRAM starting at 0x8000.
     * Each tile occupies 16 bytes, so the address is: 0x8000 + (tileIndex × 16)
     * 
     * @param tileIndex Tile index (0-511 for full VRAM)
     * @return VRAM address (0x8000-0x9FFF range)
     */
    uint16_t GetTileAddress(uint16_t tileIndex);
};

} // namespace GBDebug

#endif // TILE_DECODER_H
