#include "TileRenderer.h"

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace GBDebug {

// ============================================================================
// TexturePool Implementation
// ============================================================================

TexturePool::TexturePool()
    : rows_(0), cols_(0), scale_(1), initialized_(false)
{
}

TexturePool::~TexturePool() {
    Clear();
}

void TexturePool::Initialize(int rows, int cols, int scale) {
    // Clear any existing textures first
    Clear();
    
    rows_ = rows;
    cols_ = cols;
    scale_ = scale;
    
    int totalTextures = rows * cols;
    textures_.resize(totalTextures);
    
    int textureSize = 8 * scale;
    
    // Create all textures upfront
    for (int i = 0; i < totalTextures; i++) {
        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        // Set texture parameters for pixel-perfect rendering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Allocate texture storage (empty initially)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        
        textures_[i] = static_cast<unsigned int>(textureId);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    initialized_ = true;
}

int TexturePool::GetIndex(int row, int col) const {
    if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
        return -1;
    }
    return row * cols_ + col;
}

unsigned int TexturePool::GetTexture(int row, int col) const {
    int index = GetIndex(row, col);
    if (index < 0 || index >= static_cast<int>(textures_.size())) {
        return 0;
    }
    return textures_[index];
}

void TexturePool::UpdateTexture(int row, int col, const uint8_t* rgbaData) {
    int index = GetIndex(row, col);
    if (index < 0 || index >= static_cast<int>(textures_.size()) || rgbaData == nullptr) {
        return;
    }
    
    unsigned int textureId = textures_[index];
    if (textureId == 0) {
        return;
    }
    
    int textureSize = 8 * scale_;
    
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureId));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TexturePool::Clear() {
    for (unsigned int textureId : textures_) {
        if (textureId != 0) {
            GLuint glId = static_cast<GLuint>(textureId);
            glDeleteTextures(1, &glId);
        }
    }
    textures_.clear();
    rows_ = 0;
    cols_ = 0;
    initialized_ = false;
}

bool TexturePool::ReinitializeIfNeeded(int rows, int cols, int scale) {
    if (initialized_ && rows_ == rows && cols_ == cols && scale_ == scale) {
        return false;  // No change needed
    }
    Initialize(rows, cols, scale);
    return true;
}

// ============================================================================
// TileRenderer Implementation
// ============================================================================

TileRenderer::TileRenderer()
    : currentScale_(2)
{
    // Pre-allocate texture buffer for common case (32x32 RGBA = 4096 bytes)
    textureBuffer_.reserve(64 * 64 * 4);
}

TileRenderer::~TileRenderer() {
    ClearTextures();
}

void TileRenderer::InitializeTileGridPool(int rows, int cols, int scale) {
    tileGridPool_.ReinitializeIfNeeded(rows, cols, scale);
    currentScale_ = scale;
}

void TileRenderer::InitializeSpritePool(int maxSprites, int scale) {
    // Each sprite needs 2 textures for 8x16 mode (top and bottom halves)
    spritePool_.ReinitializeIfNeeded(maxSprites, 2, scale);
}

void TileRenderer::InitializeInspectorPool(int scale) {
    // Single texture for inspector view
    inspectorPool_.ReinitializeIfNeeded(1, 1, scale);
}

unsigned int TileRenderer::RenderTileAt(
    int row, int col,
    const std::array<std::array<uint8_t, 8>, 8>& pixelData,
    const Palette& palette
) {
    if (!tileGridPool_.IsInitialized()) {
        return 0;
    }
    
    int scale = tileGridPool_.GetScale();
    
    // Convert pixel data to RGBA
    std::vector<uint8_t> rgbaData = ConvertToRGBA(pixelData, palette, scale);
    
    // Update the texture at this grid position
    tileGridPool_.UpdateTexture(row, col, rgbaData.data());
    
    // Return the texture ID for ImGui::Image()
    return tileGridPool_.GetTexture(row, col);
}

unsigned int TileRenderer::RenderSpriteAt(
    int spriteIndex,
    const std::array<std::array<uint8_t, 8>, 8>& pixelData,
    const Palette& palette,
    bool isBottomHalf
) {
    if (!spritePool_.IsInitialized()) {
        return 0;
    }
    
    int scale = spritePool_.GetScale();
    int col = isBottomHalf ? 1 : 0;  // Column 0 = top/single, Column 1 = bottom
    
    // Convert pixel data to RGBA
    std::vector<uint8_t> rgbaData = ConvertToRGBA(pixelData, palette, scale);
    
    // Update the texture at this sprite position
    spritePool_.UpdateTexture(spriteIndex, col, rgbaData.data());
    
    // Return the texture ID for ImGui::Image()
    return spritePool_.GetTexture(spriteIndex, col);
}

unsigned int TileRenderer::RenderInspectorTile(
    const std::array<std::array<uint8_t, 8>, 8>& pixelData,
    const Palette& palette
) {
    if (!inspectorPool_.IsInitialized()) {
        return 0;
    }
    
    int scale = inspectorPool_.GetScale();
    
    // Convert pixel data to RGBA
    std::vector<uint8_t> rgbaData = ConvertToRGBA(pixelData, palette, scale);
    
    // Update the single inspector texture
    inspectorPool_.UpdateTexture(0, 0, rgbaData.data());
    
    // Return the texture ID for ImGui::Image()
    return inspectorPool_.GetTexture(0, 0);
}

unsigned int TileRenderer::CreateTexture(int width, int height) {
    GLuint textureId = 0;
    
    // Generate a new texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set texture parameters for pixel-perfect rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate texture storage (empty initially)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return static_cast<unsigned int>(textureId);
}

void TileRenderer::UpdateTexture(unsigned int texture, const uint8_t* data, int width, int height) {
    if (texture == 0 || data == nullptr) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
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
    
    // Create a new texture for this render (LEGACY behavior - causes memory leak if called repeatedly)
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
    for (const auto& pair : tileTextures_) {
        dirtyFlags_[pair.first] = true;
    }
}

bool TileRenderer::IsTileDirty(int tileIndex) const {
    auto it = dirtyFlags_.find(tileIndex);
    if (it != dirtyFlags_.end()) {
        return it->second;
    }
    return true;
}

void TileRenderer::ClearTextures() {
    // Clear texture pools
    tileGridPool_.Clear();
    spritePool_.Clear();
    inspectorPool_.Clear();
    
    // Delete legacy cached textures
    for (const auto& pair : tileTextures_) {
        GLuint textureId = static_cast<GLuint>(pair.second);
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }
    }
    
    tileTextures_.clear();
    dirtyFlags_.clear();
}

size_t TileRenderer::GetCacheSize() const {
    return tileTextures_.size() + 
           (tileGridPool_.IsInitialized() ? tileGridPool_.GetRows() * tileGridPool_.GetCols() : 0) +
           (spritePool_.IsInitialized() ? spritePool_.GetRows() * spritePool_.GetCols() : 0) +
           (inspectorPool_.IsInitialized() ? 1 : 0);
}

} // namespace GBDebug
