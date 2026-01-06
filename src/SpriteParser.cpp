#include "SpriteParser.h"

namespace GBDebug {

std::vector<SpriteAttributes> SpriteParser::ParseOAM(const uint8_t* oam, size_t size) {
    std::vector<SpriteAttributes> sprites;
    
    // Validate input
    if (oam == nullptr || size < OAM_SIZE) {
        return sprites;  // Return empty vector on error
    }
    
    // Reserve space for all 40 sprites
    sprites.reserve(MAX_SPRITES);
    
    // Parse each sprite entry (4 bytes each)
    for (size_t i = 0; i < MAX_SPRITES; i++) {
        const uint8_t* entry = oam + (i * SPRITE_ENTRY_SIZE);
        sprites.push_back(ParseSprite(entry));
    }
    
    return sprites;
}

SpriteAttributes SpriteParser::ParseSprite(const uint8_t* entry) {
    SpriteAttributes sprite;
    
    if (entry == nullptr) {
        return sprite;  // Return default-initialized sprite
    }
    
    // Byte 0: Y position
    sprite.y = entry[0];
    
    // Byte 1: X position
    sprite.x = entry[1];
    
    // Byte 2: Tile index
    sprite.tileIndex = entry[2];
    
    // Byte 3: Attribute flags
    sprite.flags = entry[3];
    
    // Parse individual flag bits
    // Bit 7: Priority (0=above BG, 1=behind BG colors 1-3)
    sprite.priority = (sprite.flags & 0x80) != 0;
    
    // Bit 6: Y flip (vertical flip)
    sprite.yFlip = (sprite.flags & 0x40) != 0;
    
    // Bit 5: X flip (horizontal flip)
    sprite.xFlip = (sprite.flags & 0x20) != 0;
    
    // Bit 4: DMG palette number (0 or 1)
    // In CGB mode, this bit is used for VRAM bank selection (bit 3)
    // For DMG mode, we use bit 4 for palette selection
    // For CGB mode, bits 0-2 are used for palette selection
    // We store the DMG palette number here; caller interprets based on mode
    sprite.paletteNumber = (sprite.flags & 0x10) >> 4;
    
    // Bit 3: CGB VRAM bank (0 or 1)
    sprite.vramBank = (sprite.flags & 0x08) >> 3;
    
    // Note: For CGB mode, the palette number is in bits 0-2 (0-7)
    // The caller should extract this from flags if needed:
    //   uint8_t cgbPalette = sprite.flags & 0x07;
    // We keep paletteNumber as bit 4 for DMG compatibility
    
    return sprite;
}

bool SpriteParser::IsSpriteVisible(const SpriteAttributes& sprite) {
    // Game Boy screen is 160x144 pixels
    // Sprite Y position: actual screen Y = Y - 16
    // Sprite X position: actual screen X = X - 8
    // Sprites are 8 pixels wide and 8 or 16 pixels tall
    
    // Y visibility check:
    // - Y=0 means sprite is 16 pixels above screen (not visible)
    // - Y=1-15 means sprite is partially above screen (top clipped)
    // - Y=16-159 means sprite is fully or partially on screen
    // - Y>=160 means sprite is below screen (not visible for 8x8)
    // For 8x16 sprites, Y>=152 would still be partially visible
    
    // A sprite is visible if any part of it is on screen
    // Y range for visibility: Y > 0 && Y < 160 (for 8x8 sprites)
    // X range for visibility: X > 0 && X < 168
    
    // Y=0 means completely off-screen (16 pixels above)
    // Y>=160 means completely off-screen (below screen for 8x8)
    if (sprite.y == 0 || sprite.y >= 160) {
        return false;
    }
    
    // X=0 means completely off-screen (8 pixels to the left)
    // X>=168 means completely off-screen (to the right)
    if (sprite.x == 0 || sprite.x >= 168) {
        return false;
    }
    
    return true;
}

} // namespace GBDebug
