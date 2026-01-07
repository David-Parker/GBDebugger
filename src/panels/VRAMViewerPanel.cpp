#include "panels/VRAMViewerPanel.h"
#include "TileDecoder.h"
#include "TileRenderer.h"
#include "PaletteManager.h"
#include "SpriteParser.h"
#include "imgui.h"
#include <cstring>
#include <memory>

namespace GBDebug {

// Constants for VRAM and OAM sizes
static constexpr size_t VRAM_BANK_SIZE = 8192;
static constexpr size_t OAM_SIZE = 160;
static constexpr size_t MAX_VRAM_BANKS = 2;
static constexpr int TILES_PER_ROW = 16;
static constexpr int DMG_TILE_COUNT = 384;
static constexpr int CGB_BANK0_TILE_COUNT = 384;
static constexpr int CGB_BANK1_TILE_COUNT = 384;
static constexpr int TILE_DISPLAY_SIZE = 16;  // 8x8 tiles at 2x scale = 16x16 pixels
static constexpr float TILE_SPACING = 1.0f;  // 1 pixel border between tiles

VRAMViewerPanel::VRAMViewerPanel()
    : decoder_(new TileDecoder()),
      renderer_(new TileRenderer()),
      paletteManager_(new PaletteManager()),
      visible_(true),
      vramBank0External_(nullptr),
      vramBank1External_(nullptr),
      vramSource_(VRAMSource::MappedMemory) {
    // Initialize VRAM buffers to zero
    vramBank0_.fill(0);
    vramBank1_.fill(0);
    
    // Initialize OAM buffer to zero
    oam_.fill(0);
    
    // Initialize CGB palettes to default values
    for (auto& palette : bgPalettes_) {
        for (int i = 0; i < 4; i++) {
            palette.colors[i] = 0;
        }
    }
    for (auto& palette : spritePalettes_) {
        for (int i = 0; i < 4; i++) {
            palette.colors[i] = 0;
        }
    }
    
    // NOTE: Texture pools are initialized lazily in Render methods
    // because OpenGL context may not be available at construction time
}

VRAMViewerPanel::~VRAMViewerPanel() {
    // Clean up textures before destroying renderer
    if (renderer_) {
        renderer_->ClearTextures();
    }
    // unique_ptr members will clean up automatically
}

const char* VRAMViewerPanel::GetName() const {
    return "VRAM Tile Viewer";
}

bool VRAMViewerPanel::IsVisible() const {
    return visible_;
}

void VRAMViewerPanel::SetVisible(bool visible) {
    visible_ = visible;
}

bool VRAMViewerPanel::UpdateVRAM(const uint8_t* buffer, size_t size, uint8_t bank) {
    // Validate buffer pointer
    if (buffer == nullptr) {
        // Log error: VRAM buffer is null
        // Gracefully handle without crashing (Requirement 13.1)
        return false;
    }
    
    // Validate buffer size (Requirement 13.2)
    if (size != VRAM_BANK_SIZE) {
        // Log error: Invalid VRAM buffer size
        return false;
    }
    
    // Validate bank index
    if (bank > 1) {
        // Log error: Invalid VRAM bank
        return false;
    }
    
    // In DMG mode, only bank 0 is used (Requirement 2.4)
    if (state_.mode == EmulationMode::DMG && bank != 0) {
        // Silently ignore bank 1 updates in DMG mode
        return true;
    }
    
    // Copy VRAM data to appropriate bank (Requirements 1.2, 2.2, 2.3)
    if (bank == 0) {
        std::memcpy(vramBank0_.data(), buffer, VRAM_BANK_SIZE);
    } else {
        std::memcpy(vramBank1_.data(), buffer, VRAM_BANK_SIZE);
    }
    
    // Mark display as needing refresh (Requirement 1.3)
    state_.needsRefresh = true;
    
    return true;
}

bool VRAMViewerPanel::UpdateOAM(const uint8_t* buffer, size_t size) {
    // Validate buffer pointer (Requirement 13.3)
    if (buffer == nullptr) {
        // Log error: OAM buffer is null
        // Gracefully handle without crashing
        return false;
    }
    
    // Validate buffer size (Requirement 13.4)
    if (size != OAM_SIZE) {
        // Log error: Invalid OAM buffer size
        return false;
    }
    
    // Copy OAM data (Requirements 3.1, 3.2)
    std::memcpy(oam_.data(), buffer, OAM_SIZE);
    
    // Mark display as needing refresh (Requirement 3.3)
    state_.needsRefresh = true;
    
    return true;
}

bool VRAMViewerPanel::UpdatePalettes(const CGBPalette* bgPalettes, const CGBPalette* spritePalettes) {
    // Validate palette pointers
    // Note: It's valid to update only one set of palettes
    bool updated = false;
    
    // Update background palettes if provided (Requirement 7.1)
    if (bgPalettes != nullptr) {
        for (int i = 0; i < 8; i++) {
            bgPalettes_[i] = bgPalettes[i];
        }
        // Pass palettes to PaletteManager for color conversion
        if (paletteManager_) {
            paletteManager_->SetBGPalettes(bgPalettes, 8);
        }
        updated = true;
    }
    
    // Update sprite palettes if provided (Requirement 7.5)
    if (spritePalettes != nullptr) {
        for (int i = 0; i < 8; i++) {
            spritePalettes_[i] = spritePalettes[i];
        }
        // Pass palettes to PaletteManager for color conversion
        if (paletteManager_) {
            paletteManager_->SetSpritePalettes(spritePalettes, 8);
        }
        updated = true;
    }
    
    if (updated) {
        // Mark all tiles as dirty since palette changed
        if (renderer_) {
            renderer_->MarkAllDirty();
        }
        // Mark display as needing refresh
        state_.needsRefresh = true;
    }
    
    return updated;
}

void VRAMViewerPanel::SetEmulationMode(EmulationMode mode) {
    // Only update if mode actually changed
    if (state_.mode != mode) {
        state_.mode = mode;
        
        // Update palette manager mode
        if (paletteManager_) {
            paletteManager_->SetMode(mode);
        }
        
        // Reset bank to 0 when switching modes (Requirement 12.1)
        state_.currentBank = 0;
        
        // Mark all tiles as dirty since palette mode changed
        if (renderer_) {
            renderer_->MarkAllDirty();
        }
        
        // Mark display as needing refresh (Requirements 12.2, 12.3, 12.4)
        state_.needsRefresh = true;
    }
}

void VRAMViewerPanel::SetVRAMBankData(const uint8_t* bank0, const uint8_t* bank1) {
    // Store pointers to external bank data (Requirements 1.3, 1.6)
    // We do not take ownership - caller retains ownership of the memory
    vramBank0External_ = bank0;
    vramBank1External_ = bank1;
    
    // If external bank data is cleared, revert to mapped memory
    if (bank0 == nullptr && bank1 == nullptr) {
        vramSource_ = VRAMSource::MappedMemory;
    }
    
    // Mark display as needing refresh
    state_.needsRefresh = true;
}

const uint8_t* VRAMViewerPanel::GetVRAMData() const {
    // Return appropriate data based on vramSource_ selection (Requirements 2.3, 2.4, 2.5)
    switch (vramSource_) {
        case VRAMSource::Bank0:
            // Return external bank 0 if available, otherwise fall back to mapped memory
            if (vramBank0External_ != nullptr) {
                return vramBank0External_;
            }
            // Fall back to internal buffer (mapped memory)
            return vramBank0_.data();
            
        case VRAMSource::Bank1:
            // Return external bank 1 if available, otherwise fall back to mapped memory
            if (vramBank1External_ != nullptr) {
                return vramBank1External_;
            }
            // Fall back to internal buffer (mapped memory)
            return vramBank0_.data();
            
        case VRAMSource::MappedMemory:
        default:
            // Return internal buffer (currently mapped VRAM)
            return vramBank0_.data();
    }
}

void VRAMViewerPanel::RenderVRAMBankSelector() {
    // Check if external bank data is provided (Requirement 2.6)
    // If not provided, don't show selector (backward compatibility)
    bool hasExternalBankData = (vramBank0External_ != nullptr || vramBank1External_ != nullptr);
    
    if (!hasExternalBankData) {
        // No external bank data provided - don't show selector
        return;
    }
    
    // Render bank selection dropdown (Requirements 2.1, 2.2)
    ImGui::Text("VRAM Source:");
    ImGui::SameLine();
    
    // Define the options
    const char* sourceOptions[] = { "Mapped Memory", "Bank 0", "Bank 1" };
    int currentSource = static_cast<int>(vramSource_);
    
    // Create combo box for bank selection
    ImGui::SetNextItemWidth(150);
    if (ImGui::Combo("##VRAMSource", &currentSource, sourceOptions, IM_ARRAYSIZE(sourceOptions))) {
        // Update vramSource_ based on selection
        VRAMSource newSource = static_cast<VRAMSource>(currentSource);
        
        // Validate selection - Bank 1 only available if external data provided
        if (newSource == VRAMSource::Bank1 && vramBank1External_ == nullptr) {
            // Bank 1 not available, keep current selection
            // Could show a tooltip or warning here
        } else {
            vramSource_ = newSource;
            
            // Mark display as needing refresh
            state_.needsRefresh = true;
            
            // Mark all tiles as dirty since data source changed
            if (renderer_) {
                renderer_->MarkAllDirty();
            }
        }
    }
    
    // Show tooltip with current source info
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        switch (vramSource_) {
            case VRAMSource::MappedMemory:
                ImGui::Text("Viewing currently mapped VRAM");
                break;
            case VRAMSource::Bank0:
                ImGui::Text("Viewing VRAM Bank 0 directly");
                break;
            case VRAMSource::Bank1:
                ImGui::Text("Viewing VRAM Bank 1 directly");
                break;
        }
        ImGui::EndTooltip();
    }
    
    // Show indicator if Bank 1 is not available
    if (vramBank1External_ == nullptr) {
        ImGui::SameLine();
        ImGui::TextDisabled("(Bank 1 N/A)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Bank 1 not available (DMG mode or not provided)");
            ImGui::EndTooltip();
        }
    }
}

void VRAMViewerPanel::Render() {
    if (!visible_) {
        return;
    }
    
    // Set initial window position and size (only on first use)
    ImGui::SetNextWindowPos(ImVec2(10, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(580, 450), ImGuiCond_FirstUseEver);
    
    ImGui::Begin(GetName());
    
    // Render bank selector at top of panel (Requirement 2.1)
    RenderVRAMBankSelector();
    
    // Render the tile grid (main content)
    RenderTileGrid();
    
    // Render tile inspector (always shown, collapsed by default)
    RenderTileInspector();
    
    // Always render sprite view (collapsed by default)
    RenderSpriteView();
    
    ImGui::End();
}

void VRAMViewerPanel::RenderTileGrid() {
    // Display mode indicator
    ImGui::Text("Mode: %s", state_.mode == EmulationMode::DMG ? "DMG" : "CGB");
    
    ImGui::Separator();
    
    // Calculate tile count based on mode
    // VRAM contains 384 tiles (8KB / 16 bytes per tile)
    int tileCount = 384;
    
    // Get VRAM data based on current source selection (Requirements 2.3, 2.4, 2.5)
    const uint8_t* vramBuffer = GetVRAMData();
    
    // Get the palette for rendering (use palette 0 for tile grid display)
    Palette palette = paletteManager_->GetBGPalette(0);
    
    // Ensure texture pool is initialized with correct parameters
    int numRows = (tileCount + TILES_PER_ROW - 1) / TILES_PER_ROW;
    renderer_->InitializeTileGridPool(numRows, TILES_PER_ROW, state_.tileScale);
    
    // Calculate the exact height needed for the tile grid
    // Each tile is TILE_DISPLAY_SIZE pixels + TILE_SPACING between rows
    float gridHeight = numRows * (TILE_DISPLAY_SIZE + TILE_SPACING) + 32.0f;  // +32 for vertical padding
    float gridWidth = TILES_PER_ROW * (TILE_DISPLAY_SIZE + TILE_SPACING) + 16.0f;  // +16 for horizontal padding
    
    // Create scrollable child region for tile grid with calculated size
    ImGui::BeginChild("TileGrid", ImVec2(gridWidth, gridHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Set tight spacing for tile grid
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(TILE_SPACING, TILE_SPACING));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    
    // Render tiles in rows of 16 (Requirement 5.6)
    for (int i = 0; i < tileCount; i++) {
        // Calculate row and column for texture pool
        int row = i / TILES_PER_ROW;
        int col = i % TILES_PER_ROW;
        
        // Start new row every TILES_PER_ROW tiles
        if (col != 0) {
            ImGui::SameLine(0, TILE_SPACING);
        }
        
        // Decode tile from VRAM
        auto pixelData = decoder_->DecodeTile(vramBuffer, static_cast<uint16_t>(i), state_.currentBank);
        
        // Render tile to texture using pool-based method (no memory leak)
        unsigned int texture = renderer_->RenderTileAt(row, col, pixelData, palette);
        
        // Display tile using ImGui::Image with invisible button for selection
        ImGui::PushID(i);
        
        // Get cursor position for drawing selection highlight
        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        // Draw selection highlight behind the tile
        if (state_.selectedTile == i) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(
                ImVec2(pos.x - 1, pos.y - 1),
                ImVec2(pos.x + TILE_DISPLAY_SIZE + 1, pos.y + TILE_DISPLAY_SIZE + 1),
                IM_COL32(100, 150, 200, 255)
            );
        }
        
        // Display tile image
        ImGui::Image((void*)(intptr_t)texture, ImVec2(TILE_DISPLAY_SIZE, TILE_DISPLAY_SIZE));
        
        // Make the image clickable for selection
        if (ImGui::IsItemClicked()) {
            state_.selectedTile = i;
        }
        
        // Show tile index on hover (Requirement 5.3)
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Tile %d (0x%03X)", i, i);
            uint16_t address = decoder_->GetTileAddress(static_cast<uint16_t>(i));
            ImGui::Text("Address: 0x%04X", address);
            ImGui::EndTooltip();
        }
        
        ImGui::PopID();
    }
    
    ImGui::PopStyleVar(2);
    ImGui::EndChild();
    
    // Clear refresh flag
    state_.needsRefresh = false;
}

void VRAMViewerPanel::RenderSpriteView() {
    // Render sprite view in a separate collapsible section (collapsed by default)
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("Sprite View")) {
        // Parse OAM data using SpriteParser (Requirement 9.2)
        SpriteParser spriteParser;
        auto sprites = spriteParser.ParseOAM(oam_.data(), oam_.size());
        
        // Display sprite count
        ImGui::Text("Sprites: %zu / 40", sprites.size());
        ImGui::SameLine();
        
        // Add checkbox for 8x16 sprite mode (Requirement 9.5)
        static bool sprite8x16Mode = false;
        ImGui::Checkbox("8x16 Mode", &sprite8x16Mode);
        
        ImGui::Separator();
        
        // Ensure sprite pool is initialized
        renderer_->InitializeSpritePool(40, 4);
        
        // Create scrollable child region for sprite list
        ImGui::BeginChild("SpriteList", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        // Display sprites in a grid layout (Requirement 9.1)
        const int spritesPerRow = 8;
        const int spriteDisplaySize = 32;  // 8x8 scaled 4x
        
        int visibleCount = 0;
        
        for (size_t i = 0; i < sprites.size(); i++) {
            const SpriteAttributes& sprite = sprites[i];
            
            // Start new row every spritesPerRow sprites
            if (i % spritesPerRow != 0) {
                ImGui::SameLine();
            }
            
            ImGui::BeginGroup();
            ImGui::PushID(static_cast<int>(i));
            
            // Get the appropriate VRAM buffer based on sprite's VRAM bank (CGB only)
            const uint8_t* vramBuffer;
            if (state_.mode == EmulationMode::CGB && sprite.vramBank == 1) {
                vramBuffer = vramBank1_.data();
            } else {
                vramBuffer = vramBank0_.data();
            }
            
            // Get the appropriate palette for this sprite (Requirement 9.3)
            Palette palette;
            if (state_.mode == EmulationMode::CGB) {
                uint8_t cgbPaletteNum = sprite.flags & 0x07;
                palette = paletteManager_->GetSpritePalette(cgbPaletteNum);
            } else {
                palette = paletteManager_->GetSpritePalette(sprite.paletteNumber);
            }
            
            // Decode and render the sprite tile
            uint16_t tileIndex = sprite.tileIndex;
            
            // In 8x16 mode, the LSB of tile index is ignored (Requirement 9.5)
            if (sprite8x16Mode) {
                tileIndex &= 0xFE;
            }
            
            // Decode the tile with flip flags applied
            auto pixelData = decoder_->DecodeTile(vramBuffer, tileIndex, sprite.vramBank);
            
            // Apply flip transformations if needed
            if (sprite.xFlip || sprite.yFlip) {
                std::array<std::array<uint8_t, 8>, 8> flippedData;
                for (int y = 0; y < 8; y++) {
                    for (int x = 0; x < 8; x++) {
                        int srcX = sprite.xFlip ? (7 - x) : x;
                        int srcY = sprite.yFlip ? (7 - y) : y;
                        flippedData[y][x] = pixelData[srcY][srcX];
                    }
                }
                pixelData = flippedData;
            }
            
            // Render the sprite tile using pool-based method (no memory leak)
            unsigned int texture = renderer_->RenderSpriteAt(static_cast<int>(i), pixelData, palette, false);
            
            // Display the sprite tile
            if (sprite8x16Mode) {
                // 8x16 mode: render both tiles vertically (Requirement 9.5)
                ImGui::Image((void*)(intptr_t)texture, ImVec2(spriteDisplaySize, spriteDisplaySize));
                
                // Decode and render the bottom tile (tileIndex + 1)
                uint16_t bottomTileIndex = tileIndex | 0x01;
                auto bottomPixelData = decoder_->DecodeTile(vramBuffer, bottomTileIndex, sprite.vramBank);
                
                // Apply flip transformations to bottom tile
                if (sprite.xFlip || sprite.yFlip) {
                    std::array<std::array<uint8_t, 8>, 8> flippedData;
                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            int srcX = sprite.xFlip ? (7 - x) : x;
                            int srcY = sprite.yFlip ? (7 - y) : y;
                            flippedData[y][x] = bottomPixelData[srcY][srcX];
                        }
                    }
                    bottomPixelData = flippedData;
                }
                
                // Render bottom tile using pool-based method
                unsigned int bottomTexture = renderer_->RenderSpriteAt(static_cast<int>(i), bottomPixelData, palette, true);
                ImGui::Image((void*)(intptr_t)bottomTexture, ImVec2(spriteDisplaySize, spriteDisplaySize));
            } else {
                // 8x8 mode: render single tile
                ImGui::Image((void*)(intptr_t)texture, ImVec2(spriteDisplaySize, spriteDisplaySize));
            }
            
            // Show sprite info on hover (Requirement 9.3)
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Sprite %zu", i);
                ImGui::Separator();
                
                int screenY = static_cast<int>(sprite.y) - 16;
                int screenX = static_cast<int>(sprite.x) - 8;
                ImGui::Text("Position: (%d, %d)", screenX, screenY);
                ImGui::Text("Raw Y/X: (%d, %d)", sprite.y, sprite.x);
                ImGui::Text("Tile: %d (0x%02X)", sprite.tileIndex, sprite.tileIndex);
                
                if (state_.mode == EmulationMode::CGB) {
                    uint8_t cgbPaletteNum = sprite.flags & 0x07;
                    ImGui::Text("Palette: OBP%d", cgbPaletteNum);
                    ImGui::Text("VRAM Bank: %d", sprite.vramBank);
                } else {
                    ImGui::Text("Palette: OBP%d", sprite.paletteNumber);
                }
                
                ImGui::Text("Flip: %s%s", 
                    sprite.xFlip ? "H" : "-",
                    sprite.yFlip ? "V" : "-");
                ImGui::Text("Priority: %s", sprite.priority ? "Behind BG" : "Above BG");
                
                bool visible = spriteParser.IsSpriteVisible(sprite);
                ImGui::Text("Visible: %s", visible ? "Yes" : "No");
                
                ImGui::EndTooltip();
            }
            
            ImGui::Text("%02zu", i);
            
            ImGui::PopID();
            ImGui::EndGroup();
            
            if (spriteParser.IsSpriteVisible(sprite)) {
                visibleCount++;
            }
        }
        
        ImGui::EndChild();
        
        ImGui::Text("Visible on screen: %d / 40", visibleCount);
    }
}

void VRAMViewerPanel::RenderTileInspector() {
    // Render tile inspector in a separate collapsible section (collapsed by default)
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("Tile Inspector")) {
        // Get the selected tile index
        int tileIndex = state_.selectedTile;
        
        // Display tile index (Requirement 8.2)
        ImGui::Text("Selected Tile: %d (0x%03X)", tileIndex, tileIndex);
        
        // Calculate and display VRAM address range (Requirement 8.3)
        uint16_t startAddress = decoder_->GetTileAddress(static_cast<uint16_t>(tileIndex));
        uint16_t endAddress = startAddress + 15;  // 16 bytes per tile (0-15 offset)
        ImGui::Text("VRAM Address: 0x%04X - 0x%04X", startAddress, endAddress);
        
        // Display bank info for CGB mode
        if (state_.mode == EmulationMode::CGB) {
            ImGui::Text("Bank: %d", state_.currentBank);
        }
        
        ImGui::Spacing();
        
        // Render selected tile at larger scale (Requirement 8.4)
        // Use 8x scale for inspector view (64x64 pixels)
        const int inspectorScale = 8;
        const int inspectorTileSize = 8 * inspectorScale;  // 64 pixels
        
        // Ensure inspector pool is initialized
        renderer_->InitializeInspectorPool(inspectorScale);
        
        // Get the appropriate VRAM buffer based on current source selection
        // This ensures the inspector shows data from the same source as the tile grid
        const uint8_t* vramBuffer = GetVRAMData();
        
        // Determine the bank number for decoding based on vramSource_
        uint8_t bankForDecoding = 0;
        if (vramSource_ == VRAMSource::Bank1) {
            bankForDecoding = 1;
        } else if (vramSource_ == VRAMSource::MappedMemory) {
            bankForDecoding = state_.currentBank;
        }
        
        // Decode the tile
        auto pixelData = decoder_->DecodeTile(vramBuffer, static_cast<uint16_t>(tileIndex), bankForDecoding);
        
        // Get the palette for rendering
        Palette palette = paletteManager_->GetBGPalette(state_.selectedPalette);
        
        // Render tile using pool-based method (no memory leak)
        unsigned int texture = renderer_->RenderInspectorTile(pixelData, palette);
        
        // Display the enlarged tile
        ImGui::Text("Preview (8x scale):");
        ImGui::Image((void*)(intptr_t)texture, ImVec2(inspectorTileSize, inspectorTileSize));
        
        ImGui::Spacing();
        
        // Display raw byte data (Requirement 8.5)
        ImGui::Text("Raw Tile Data (16 bytes):");
        
        // Calculate the offset within the VRAM buffer
        size_t tileOffset = static_cast<size_t>(tileIndex) * 16;
        
        // Ensure we don't read past the buffer
        if (tileOffset + 16 <= VRAM_BANK_SIZE) {
            const uint8_t* tileData = vramBuffer + tileOffset;
            
            ImGui::BeginChild("TileBytes", ImVec2(0, 80), true);
            
            // Display as 8 rows (one per tile row), showing LSB and MSB bytes
            for (int row = 0; row < 8; row++) {
                uint8_t lsb = tileData[row * 2];
                uint8_t msb = tileData[row * 2 + 1];
                ImGui::Text("Row %d: %02X %02X", row, lsb, msb);
                
                if (row < 7) {
                    if ((row % 2) == 0) {
                        ImGui::SameLine(150);
                    }
                }
            }
            
            ImGui::EndChild();
            
            // Also show all 16 bytes in a single line for easy copying
            ImGui::Text("Hex: ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f),
                "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                tileData[0], tileData[1], tileData[2], tileData[3],
                tileData[4], tileData[5], tileData[6], tileData[7],
                tileData[8], tileData[9], tileData[10], tileData[11],
                tileData[12], tileData[13], tileData[14], tileData[15]);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error: Tile data out of bounds");
        }
        
        // Add a button to deselect the tile
        ImGui::Spacing();
        if (ImGui::Button("Clear Selection")) {
            state_.selectedTile = -1;
        }
    }
}

} // namespace GBDebug
