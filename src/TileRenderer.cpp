#include "TileRenderer.h"

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace GBDebug {

TileRenderer::TileRenderer()
    : currentScale_(4)
{
    // Pre-allocate texture buffer for common case (32x32 RGBA = 4096 bytes)
    textureBuffer_.reserve(32 * 32 * 4);
}

TileRenderer::~TileRenderer() {
    ClearTextures();
}

unsigned int TileRenderer::CreateTexture(int width, int height) {
    GLuint textureId = 0;
    
    // Generate a new texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set texture parameters for pixel-perfect rendering
    // Use nearest-neighbor filtering to preserve sharp pixel edges
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Clamp to edge to avoid texture bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate texture storage (empty initially)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return static_cast<unsigned int>(textureId);
}

void TileRenderer::UpdateTexture(unsigned int texture, const uint8_t* data, int width, int height) {
    if (texture == 0 || data == nullptr) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
    
    // Upload pixel data to texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::vector<uint8_t> TileRenderer::ConvertToRGBA(
    const std::array<std::array<uint8_t, 8>, 8>& pixelData,
    const Palette& palette,
    int scale
) {
    int outputSize = 8 * scale;
    std::vector<uint8_t> rgba(outputSize * outputSize * 4);
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Get color index (0-3) from pixel data
            uint8_t colorIndex = pixelData[y][x];
            
            // Clamp to valid range
            if (colorIndex > 3) {
                colorIndex = 3;
            }
            
            // Get color from palette
            const TileColor& color = palette.colors[colorIndex];
            
            // Fill scaled pixels
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int outX = x * scale + sx;
                    int outY = y * scale + sy;
                    int offset = (outY * outputSize + outX) * 4;
                    
                    rgba[offset + 0] = color.r;
                    rgba[offset + 1] = color.g;
                    rgba[offset + 2] = color.b;
                    rgba[offset + 3] = color.a;
                }
            }
        }
    }
    
    return rgba;
}

unsigned int TileRenderer::RenderTile(
    const std::array<std::array<uint8_t, 8>, 8>& pixelData,
    const Palette& palette,
    int scale
) {
    // Clamp scale to reasonable range
    if (scale < 1) scale = 1;
    if (scale > 8) scale = 8;
    
    int textureSize = 8 * scale;
    
    // Convert pixel data to RGBA
    std::vector<uint8_t> rgbaData = ConvertToRGBA(pixelData, palette, scale);
    
    // Create a new texture for this render
    unsigned int texture = CreateTexture(textureSize, textureSize);
    
    if (texture != 0) {
        // Upload the pixel data
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    return texture;
}

void TileRenderer::UpdateTiles(
    const std::vector<TileData>& tiles,
    const Palette& palette
) {
    for (const auto& tile : tiles) {
        int tileIndex = tile.tileIndex;
        
        // Check if we already have a texture for this tile
        auto it = tileTextures_.find(tileIndex);
        
        if (it == tileTextures_.end()) {
            // Create new texture
            unsigned int texture = RenderTile(tile.pixels, palette, currentScale_);
            if (texture != 0) {
                tileTextures_[tileIndex] = texture;
                dirtyFlags_[tileIndex] = false;
            }
        } else if (IsTileDirty(tileIndex)) {
            // Update existing texture
            int textureSize = 8 * currentScale_;
            std::vector<uint8_t> rgbaData = ConvertToRGBA(tile.pixels, palette, currentScale_);
            
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(it->second));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize,
                           GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
            glBindTexture(GL_TEXTURE_2D, 0);
            
            dirtyFlags_[tileIndex] = false;
        }
    }
}

unsigned int TileRenderer::GetTileTexture(int tileIndex) const {
    auto it = tileTextures_.find(tileIndex);
    if (it != tileTextures_.end()) {
        return it->second;
    }
    return 0;
}

void TileRenderer::MarkTileDirty(int tileIndex) {
    dirtyFlags_[tileIndex] = true;
}

void TileRenderer::MarkAllDirty() {
    for (auto& pair : dirtyFlags_) {
        pair.second = true;
    }
    // Also mark any tiles that have textures but no dirty flag entry
    for (const auto& pair : tileTextures_) {
        dirtyFlags_[pair.first] = true;
    }
}

bool TileRenderer::IsTileDirty(int tileIndex) const {
    auto it = dirtyFlags_.find(tileIndex);
    if (it != dirtyFlags_.end()) {
        return it->second;
    }
    // If no entry exists, consider it dirty (needs initial render)
    return true;
}

void TileRenderer::ClearTextures() {
    // Delete all OpenGL textures
    for (const auto& pair : tileTextures_) {
        GLuint textureId = static_cast<GLuint>(pair.second);
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }
    }
    
    // Clear the maps
    tileTextures_.clear();
    dirtyFlags_.clear();
}

size_t TileRenderer::GetCacheSize() const {
    return tileTextures_.size();
}

} // namespace GBDebug
