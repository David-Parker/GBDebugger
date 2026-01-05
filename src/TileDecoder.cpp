#include "TileDecoder.h"

namespace GBDebug {

uint16_t TileDecoder::GetTileAddress(uint16_t tileIndex) {
    // Each tile is 16 bytes, VRAM starts at 0x8000
    // Address = 0x8000 + (tileIndex * 16)
    return 0x8000 + (tileIndex * 16);
}

uint8_t TileDecoder::DecodePixel(
    const uint8_t* tileData,
    int x,
    int y,
    bool hFlip,
    bool vFlip
) {
    // Apply vertical flip
    int actualY = vFlip ? (7 - y) : y;
    
    // Apply horizontal flip
    int actualX = hFlip ? (7 - x) : x;
    
    // Each row is 2 bytes: LSB byte and MSB byte
    // Row data starts at byte offset: actualY * 2
    int rowOffset = actualY * 2;
    uint8_t lsbByte = tileData[rowOffset];
    uint8_t msbByte = tileData[rowOffset + 1];
    
    // Extract the bit for this pixel
    // Bit 7 = leftmost pixel (x=0), Bit 0 = rightmost pixel (x=7)
    int bitPosition = 7 - actualX;
    
    // Combine LSB and MSB bits to form color index
    uint8_t lsb = (lsbByte >> bitPosition) & 0x01;
    uint8_t msb = (msbByte >> bitPosition) & 0x01;
    
    // Color index = (MSB << 1) | LSB
    return (msb << 1) | lsb;
}

std::array<std::array<uint8_t, 8>, 8> TileDecoder::DecodeTile(
    const uint8_t* vram,
    uint16_t tileIndex,
    uint8_t bank
) {
    std::array<std::array<uint8_t, 8>, 8> pixelData;
    
    // Calculate offset within VRAM buffer (not absolute address)
    // Each tile is 16 bytes
    uint16_t offset = tileIndex * 16;
    
    // Get pointer to tile data within VRAM
    const uint8_t* tileData = vram + offset;
    
    // Decode each pixel in the 8x8 tile
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            pixelData[y][x] = DecodePixel(tileData, x, y, false, false);
        }
    }
    
    return pixelData;
}

} // namespace GBDebug
