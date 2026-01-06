#ifndef SPRITE_PARSER_H
#define SPRITE_PARSER_H

#include <cstdint>
#include <vector>
#include "panels/VRAMViewerPanel.h"

namespace GBDebug {

/**
 * SpriteParser - Parses OAM (Object Attribute Memory) data to extract sprite information
 * 
 * Responsible for converting raw OAM bytes into structured sprite attribute data.
 * The Game Boy OAM contains 40 sprite entries, each 4 bytes, defining sprite
 * position, tile index, and rendering attributes (flip, priority, palette).
 * 
 * OAM Format (4 bytes per sprite, 40 sprites total = 160 bytes):
 * - Byte 0: Y position (sprite Y = value - 16)
 * - Byte 1: X position (sprite X = value - 8)
 * - Byte 2: Tile index
 * - Byte 3: Attribute flags
 *   - Bit 7: Priority (0=above BG, 1=behind BG colors 1-3)
 *   - Bit 6: Y flip (vertical flip)
 *   - Bit 5: X flip (horizontal flip)
 *   - Bit 4: DMG palette number / CGB VRAM bank
 *   - Bits 0-2: CGB palette number
 * 
 * Usage:
 *   SpriteParser parser;
 *   auto sprites = parser.ParseOAM(oamBuffer, 160);
 *   for (const auto& sprite : sprites) {
 *       if (parser.IsSpriteVisible(sprite)) {
 *           // Render sprite
 *       }
 *   }
 */
class SpriteParser {
public:
    SpriteParser() = default;
    ~SpriteParser() = default;
    
    /**
     * Parse all sprites from OAM buffer
     * 
     * Reads the entire OAM buffer and parses all 40 sprite entries.
     * Each entry is 4 bytes, so the buffer must be at least 160 bytes.
     * 
     * @param oam Pointer to OAM buffer (160 bytes)
     * @param size Size of OAM buffer (must be 160)
     * @return Vector of 40 SpriteAttributes, or empty vector on error
     */
    std::vector<SpriteAttributes> ParseOAM(const uint8_t* oam, size_t size);
    
    /**
     * Parse a single sprite entry from 4 bytes
     * 
     * Extracts all sprite attributes from a 4-byte OAM entry including
     * position, tile index, and all flag bits.
     * 
     * @param entry Pointer to 4-byte sprite entry
     * @return SpriteAttributes with parsed values
     */
    SpriteAttributes ParseSprite(const uint8_t* entry);
    
    /**
     * Check if a sprite is visible on screen
     * 
     * A sprite is considered visible if its position places any part of it
     * within the Game Boy's 160x144 screen area. Sprites are 8x8 or 8x16 pixels.
     * 
     * Screen visibility rules:
     * - Y position: sprite is visible if (Y - 16) is in range [-16, 143]
     * - X position: sprite is visible if (X - 8) is in range [-8, 159]
     * - Y=0 or Y>=160 means sprite is off-screen vertically
     * - X=0 or X>=168 means sprite is off-screen horizontally
     * 
     * @param sprite SpriteAttributes to check
     * @return true if sprite is at least partially visible
     */
    bool IsSpriteVisible(const SpriteAttributes& sprite);
    
    /// Maximum number of sprites in OAM
    static constexpr size_t MAX_SPRITES = 40;
    
    /// Size of each sprite entry in bytes
    static constexpr size_t SPRITE_ENTRY_SIZE = 4;
    
    /// Total OAM size in bytes
    static constexpr size_t OAM_SIZE = MAX_SPRITES * SPRITE_ENTRY_SIZE;
};

} // namespace GBDebug

#endif // SPRITE_PARSER_H
