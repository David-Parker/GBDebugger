#include "GBDebugger.h"
#include "panels/VRAMViewerPanel.h"  // For EmulationMode and CGBPalette
#include <iostream>
#include <vector>
#include <cstdint>

/**
 * Simple example demonstrating GBDebugger API usage
 * 
 * This example shows how to:
 * 1. Create a GBDebugger instance
 * 2. Open the debugger window
 * 3. Update CPU state with sample data
 * 4. Update memory contents with sample data
 * 5. Update VRAM with sample tile data
 * 6. Update OAM with sample sprite data
 * 7. Update palettes with sample color data
 * 8. Set emulation mode (DMG/CGB)
 * 9. Render the debugger in a loop
 * 10. Close the debugger
 * 
 * Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, VRAM Viewer Requirements
 */

int main() {
    std::cout << "GBDebugger API Usage Example\n";
    std::cout << "============================\n\n";
    
    // Step 1: Create a GBDebugger instance
    std::cout << "Step 1: Creating GBDebugger instance...\n";
    GBDebug::GBDebugger debugger;
    std::cout << "  ✓ GBDebugger instance created\n\n";
    
    // Step 2: Open the debugger window
    std::cout << "Step 2: Opening debugger window...\n";
    if (debugger.Open()) {
        std::cout << "  ✓ Debugger opened successfully\n";
    } else {
        std::cerr << "  ✗ Failed to open debugger\n";
        return 1;
    }
    
    // Verify the debugger is open
    if (debugger.IsOpen()) {
        std::cout << "  ✓ Debugger is open (IsOpen() returned true)\n\n";
    }
    
    // Step 3: Update CPU state with sample data
    std::cout << "Step 3: Updating CPU state with sample data...\n";
    
    // Sample CPU state representing a GameBoy at boot
    uint64_t cycle = 12345;
    uint16_t pc = 0x0150;    // Program counter after boot ROM
    uint16_t sp = 0xFFFE;    // Stack pointer at top of memory
    uint16_t af = 0x01B0;    // A=0x01, F=0xB0 (Z=1, N=0, H=1, C=1)
    uint16_t bc = 0x0013;    // BC register
    uint16_t de = 0x00D8;    // DE register
    uint16_t hl = 0x014D;    // HL register
    bool ime = true;         // Interrupts enabled
    
    debugger.UpdateCPU(cycle, pc, sp, af, bc, de, hl, ime);
    
    std::cout << "  ✓ CPU state updated:\n";
    std::cout << "    - Cycle: " << cycle << "\n";
    std::cout << "    - PC: 0x" << std::hex << pc << "\n";
    std::cout << "    - SP: 0x" << sp << "\n";
    std::cout << "    - AF: 0x" << af << " (A=0x" << (af >> 8) << ", F=0x" << (af & 0xFF) << ")\n";
    std::cout << "    - BC: 0x" << bc << "\n";
    std::cout << "    - DE: 0x" << de << "\n";
    std::cout << "    - HL: 0x" << hl << "\n";
    std::cout << "    - IME: " << (ime ? "enabled" : "disabled") << "\n";
    std::cout << std::dec << "\n";
    
    // Step 4: Update memory with sample data
    std::cout << "Step 4: Updating memory with sample data...\n";
    
    // Create a 64KB memory buffer (full GameBoy address space)
    std::vector<uint8_t> memory(65536, 0);
    
    // Fill ROM area with sample data
    // Nintendo logo area (0x0104-0x0133)
    for (size_t i = 0x0104; i < 0x0134; i++) {
        memory[i] = static_cast<uint8_t>((i * 7) & 0xFF);
    }
    
    // ROM header area (0x0134-0x014F)
    const char* title = "EXAMPLE";
    for (size_t i = 0; i < 7 && title[i] != '\0'; i++) {
        memory[0x0134 + i] = static_cast<uint8_t>(title[i]);
    }
    
    // Sample code at PC location (0x0150)
    memory[0x0150] = 0x3E;  // LD A, 0x42
    memory[0x0151] = 0x42;
    memory[0x0152] = 0x06;  // LD B, 0x10
    memory[0x0153] = 0x10;
    memory[0x0154] = 0xC3;  // JP 0x0100
    memory[0x0155] = 0x00;
    memory[0x0156] = 0x01;
    
    // Fill VRAM with pattern
    for (size_t i = 0x8000; i < 0x9000; i++) {
        memory[i] = static_cast<uint8_t>((i & 0xFF));
    }
    
    // Fill WRAM with test data
    for (size_t i = 0xC000; i < 0xC100; i++) {
        memory[i] = static_cast<uint8_t>((i - 0xC000) & 0xFF);
    }
    
    // Update the debugger with memory contents
    if (debugger.UpdateMemory(memory.data(), memory.size())) {
        std::cout << "  ✓ Memory updated (65536 bytes)\n";
        std::cout << "    - ROM area filled with sample data\n";
        std::cout << "    - VRAM filled with pattern\n";
        std::cout << "    - WRAM filled with test data\n\n";
    } else {
        std::cerr << "  ✗ Failed to update memory\n";
        return 1;
    }
    
    // Step 5: Set emulation mode for VRAM viewer
    std::cout << "Step 5: Setting emulation mode...\n";
    debugger.SetEmulationMode(GBDebug::EmulationMode::CGB);
    std::cout << "  ✓ Emulation mode set to CGB (Game Boy Color)\n\n";
    
    // Step 6: Update VRAM with sample tile data
    std::cout << "Step 6: Updating VRAM with sample tile data...\n";
    
    // Create 8KB VRAM buffer for bank 0
    std::vector<uint8_t> vramBank0(8192, 0);
    
    // Create sample tiles in VRAM
    // Each tile is 16 bytes (8 rows × 2 bytes per row)
    // Tile format: Each row uses 2 bytes - LSB and MSB combine to form 2-bit color indices
    
    // Tile 0: Solid color 0 (all zeros - transparent/white)
    // Already initialized to 0
    
    // Tile 1: Checkerboard pattern (alternating colors 0 and 3)
    for (int row = 0; row < 8; row++) {
        uint8_t pattern = (row % 2 == 0) ? 0xAA : 0x55;  // Alternating bits
        vramBank0[16 + row * 2] = pattern;      // LSB
        vramBank0[16 + row * 2 + 1] = pattern;  // MSB (same = color 0 or 3)
    }
    
    // Tile 2: Horizontal stripes (alternating rows of color 0 and 3)
    for (int row = 0; row < 8; row++) {
        uint8_t pattern = (row % 2 == 0) ? 0xFF : 0x00;
        vramBank0[32 + row * 2] = pattern;
        vramBank0[32 + row * 2 + 1] = pattern;
    }
    
    // Tile 3: Vertical stripes (alternating columns)
    for (int row = 0; row < 8; row++) {
        vramBank0[48 + row * 2] = 0xAA;      // LSB: 10101010
        vramBank0[48 + row * 2 + 1] = 0xAA;  // MSB: 10101010 -> color 3 for odd pixels
    }
    
    // Tile 4: Gradient (different colors per row)
    for (int row = 0; row < 8; row++) {
        uint8_t lsb = (row < 4) ? 0x00 : 0xFF;
        uint8_t msb = (row % 2 == 0) ? 0x00 : 0xFF;
        vramBank0[64 + row * 2] = lsb;
        vramBank0[64 + row * 2 + 1] = msb;
    }
    
    // Tile 5: Border/frame pattern
    vramBank0[80] = 0xFF; vramBank0[81] = 0xFF;  // Top row: all color 3
    for (int row = 1; row < 7; row++) {
        vramBank0[80 + row * 2] = 0x81;      // Left and right edges
        vramBank0[80 + row * 2 + 1] = 0x81;
    }
    vramBank0[94] = 0xFF; vramBank0[95] = 0xFF;  // Bottom row: all color 3
    
    // Tile 6: Diagonal pattern
    for (int row = 0; row < 8; row++) {
        uint8_t pattern = 0x80 >> row;  // Single bit moving right each row
        vramBank0[96 + row * 2] = pattern;
        vramBank0[96 + row * 2 + 1] = pattern;
    }
    
    // Tile 7: X pattern
    for (int row = 0; row < 8; row++) {
        uint8_t left = 0x80 >> row;
        uint8_t right = 0x01 << row;
        uint8_t pattern = left | right;
        vramBank0[112 + row * 2] = pattern;
        vramBank0[112 + row * 2 + 1] = pattern;
    }
    
    // Fill remaining tiles with various patterns
    for (int tile = 8; tile < 384; tile++) {
        int baseAddr = tile * 16;
        for (int row = 0; row < 8; row++) {
            // Create unique patterns based on tile index
            vramBank0[baseAddr + row * 2] = static_cast<uint8_t>((tile + row) & 0xFF);
            vramBank0[baseAddr + row * 2 + 1] = static_cast<uint8_t>((tile * 2 + row) & 0xFF);
        }
    }
    
    // Update VRAM bank 0
    if (debugger.UpdateVRAM(vramBank0.data(), vramBank0.size(), 0)) {
        std::cout << "  ✓ VRAM bank 0 updated (8192 bytes)\n";
        std::cout << "    - Tile 0: Solid transparent\n";
        std::cout << "    - Tile 1: Checkerboard pattern\n";
        std::cout << "    - Tile 2: Horizontal stripes\n";
        std::cout << "    - Tile 3: Vertical stripes\n";
        std::cout << "    - Tile 4: Gradient\n";
        std::cout << "    - Tile 5: Border frame\n";
        std::cout << "    - Tile 6: Diagonal\n";
        std::cout << "    - Tile 7: X pattern\n";
        std::cout << "    - Tiles 8-383: Generated patterns\n";
    } else {
        std::cerr << "  ✗ Failed to update VRAM bank 0\n";
    }
    
    // Create and update VRAM bank 1 (CGB only)
    std::vector<uint8_t> vramBank1(8192, 0);
    
    // Fill bank 1 with inverted patterns
    for (int tile = 0; tile < 128; tile++) {
        int baseAddr = tile * 16;
        for (int row = 0; row < 8; row++) {
            vramBank1[baseAddr + row * 2] = static_cast<uint8_t>(~vramBank0[baseAddr + row * 2]);
            vramBank1[baseAddr + row * 2 + 1] = static_cast<uint8_t>(~vramBank0[baseAddr + row * 2 + 1]);
        }
    }
    
    if (debugger.UpdateVRAM(vramBank1.data(), vramBank1.size(), 1)) {
        std::cout << "  ✓ VRAM bank 1 updated (8192 bytes, CGB mode)\n";
        std::cout << "    - Contains inverted patterns from bank 0\n\n";
    } else {
        std::cerr << "  ✗ Failed to update VRAM bank 1\n";
    }
    
    // Step 7: Update OAM with sample sprite data
    std::cout << "Step 7: Updating OAM with sample sprite data...\n";
    
    // Create 160-byte OAM buffer (40 sprites × 4 bytes each)
    std::vector<uint8_t> oam(160, 0);
    
    // OAM entry format:
    // Byte 0: Y position (Y - 16 = screen Y, so Y=16 means screen Y=0)
    // Byte 1: X position (X - 8 = screen X, so X=8 means screen X=0)
    // Byte 2: Tile index
    // Byte 3: Attributes (priority, y-flip, x-flip, palette, bank)
    
    // Sprite 0: At position (20, 30), tile 1, no flip
    oam[0] = 30 + 16;  // Y = 30 (screen) + 16 (offset)
    oam[1] = 20 + 8;   // X = 20 (screen) + 8 (offset)
    oam[2] = 1;        // Tile index 1 (checkerboard)
    oam[3] = 0x00;     // No flags
    
    // Sprite 1: At position (40, 30), tile 2, x-flip
    oam[4] = 30 + 16;
    oam[5] = 40 + 8;
    oam[6] = 2;        // Tile index 2 (horizontal stripes)
    oam[7] = 0x20;     // X-flip (bit 5)
    
    // Sprite 2: At position (60, 30), tile 3, y-flip
    oam[8] = 30 + 16;
    oam[9] = 60 + 8;
    oam[10] = 3;       // Tile index 3 (vertical stripes)
    oam[11] = 0x40;    // Y-flip (bit 6)
    
    // Sprite 3: At position (80, 30), tile 4, both flips
    oam[12] = 30 + 16;
    oam[13] = 80 + 8;
    oam[14] = 4;       // Tile index 4 (gradient)
    oam[15] = 0x60;    // X-flip and Y-flip (bits 5 and 6)
    
    // Sprite 4: At position (100, 30), tile 5, behind BG
    oam[16] = 30 + 16;
    oam[17] = 100 + 8;
    oam[18] = 5;       // Tile index 5 (border)
    oam[19] = 0x80;    // Priority (bit 7) - behind BG
    
    // Sprite 5-9: Row of sprites at Y=60
    for (int i = 0; i < 5; i++) {
        int idx = (5 + i) * 4;
        oam[idx] = 60 + 16;
        oam[idx + 1] = static_cast<uint8_t>((20 + i * 20) + 8);
        oam[idx + 2] = static_cast<uint8_t>(6 + i);  // Tiles 6-10
        oam[idx + 3] = static_cast<uint8_t>(i & 0x07);  // CGB palette 0-4
    }
    
    // Fill remaining sprites with test data
    for (int sprite = 10; sprite < 40; sprite++) {
        int idx = sprite * 4;
        oam[idx] = static_cast<uint8_t>(80 + (sprite / 10) * 20 + 16);  // Y
        oam[idx + 1] = static_cast<uint8_t>((sprite % 10) * 16 + 8);    // X
        oam[idx + 2] = static_cast<uint8_t>(sprite);                     // Tile
        oam[idx + 3] = static_cast<uint8_t>((sprite % 8));              // Palette
    }
    
    if (debugger.UpdateOAM(oam.data(), oam.size())) {
        std::cout << "  ✓ OAM updated (160 bytes, 40 sprites)\n";
        std::cout << "    - Sprite 0: Checkerboard at (20, 30)\n";
        std::cout << "    - Sprite 1: H-stripes at (40, 30), X-flip\n";
        std::cout << "    - Sprite 2: V-stripes at (60, 30), Y-flip\n";
        std::cout << "    - Sprite 3: Gradient at (80, 30), both flips\n";
        std::cout << "    - Sprite 4: Border at (100, 30), behind BG\n";
        std::cout << "    - Sprites 5-9: Row at Y=60\n";
        std::cout << "    - Sprites 10-39: Grid pattern\n\n";
    } else {
        std::cerr << "  ✗ Failed to update OAM\n";
    }
    
    // Step 8: Update CGB palettes
    std::cout << "Step 8: Updating CGB palettes...\n";
    
    // Create 8 background palettes
    GBDebug::CGBPalette bgPalettes[8];
    
    // Palette 0: Classic DMG grayscale
    bgPalettes[0].colors[0] = 0x7FFF;  // White (RGB555: 31, 31, 31)
    bgPalettes[0].colors[1] = 0x5294;  // Light gray
    bgPalettes[0].colors[2] = 0x294A;  // Dark gray
    bgPalettes[0].colors[3] = 0x0000;  // Black
    
    // Palette 1: Green (classic Game Boy look)
    bgPalettes[1].colors[0] = 0x03E0;  // Light green
    bgPalettes[1].colors[1] = 0x02C0;  // Medium green
    bgPalettes[1].colors[2] = 0x0180;  // Dark green
    bgPalettes[1].colors[3] = 0x0000;  // Black
    
    // Palette 2: Blue theme
    bgPalettes[2].colors[0] = 0x7FFF;  // White
    bgPalettes[2].colors[1] = 0x5EF7;  // Light blue
    bgPalettes[2].colors[2] = 0x3DEF;  // Medium blue
    bgPalettes[2].colors[3] = 0x001F;  // Dark blue
    
    // Palette 3: Red theme
    bgPalettes[3].colors[0] = 0x7FFF;  // White
    bgPalettes[3].colors[1] = 0x7E10;  // Light red
    bgPalettes[3].colors[2] = 0x5C00;  // Medium red
    bgPalettes[3].colors[3] = 0x0000;  // Black
    
    // Palette 4: Yellow/Gold theme
    bgPalettes[4].colors[0] = 0x7FFF;  // White
    bgPalettes[4].colors[1] = 0x7FE0;  // Yellow
    bgPalettes[4].colors[2] = 0x5EC0;  // Orange
    bgPalettes[4].colors[3] = 0x0000;  // Black
    
    // Palette 5: Purple theme
    bgPalettes[5].colors[0] = 0x7FFF;  // White
    bgPalettes[5].colors[1] = 0x7C1F;  // Light purple
    bgPalettes[5].colors[2] = 0x5810;  // Medium purple
    bgPalettes[5].colors[3] = 0x0000;  // Black
    
    // Palette 6: Cyan theme
    bgPalettes[6].colors[0] = 0x7FFF;  // White
    bgPalettes[6].colors[1] = 0x03FF;  // Cyan
    bgPalettes[6].colors[2] = 0x02BF;  // Teal
    bgPalettes[6].colors[3] = 0x0000;  // Black
    
    // Palette 7: Sepia/Brown theme
    bgPalettes[7].colors[0] = 0x7FFF;  // White
    bgPalettes[7].colors[1] = 0x5ED6;  // Light brown
    bgPalettes[7].colors[2] = 0x3D8C;  // Medium brown
    bgPalettes[7].colors[3] = 0x1842;  // Dark brown
    
    // Create 8 sprite palettes (similar but with transparent color 0)
    GBDebug::CGBPalette spritePalettes[8];
    
    // Copy background palettes but color 0 is typically transparent for sprites
    for (int i = 0; i < 8; i++) {
        spritePalettes[i].colors[0] = 0x0000;  // Transparent (black, but ignored)
        spritePalettes[i].colors[1] = bgPalettes[i].colors[1];
        spritePalettes[i].colors[2] = bgPalettes[i].colors[2];
        spritePalettes[i].colors[3] = bgPalettes[i].colors[3];
    }
    
    // Make sprite palette 0 use bright colors
    spritePalettes[0].colors[1] = 0x001F;  // Red
    spritePalettes[0].colors[2] = 0x03E0;  // Green
    spritePalettes[0].colors[3] = 0x7C00;  // Blue
    
    if (debugger.UpdatePalettes(bgPalettes, spritePalettes)) {
        std::cout << "  ✓ CGB palettes updated\n";
        std::cout << "    - BG Palette 0: Grayscale\n";
        std::cout << "    - BG Palette 1: Green (classic GB)\n";
        std::cout << "    - BG Palette 2: Blue theme\n";
        std::cout << "    - BG Palette 3: Red theme\n";
        std::cout << "    - BG Palette 4: Yellow/Gold\n";
        std::cout << "    - BG Palette 5: Purple\n";
        std::cout << "    - BG Palette 6: Cyan\n";
        std::cout << "    - BG Palette 7: Sepia\n";
        std::cout << "    - Sprite palettes: Similar with transparent color 0\n\n";
    } else {
        std::cerr << "  ✗ Failed to update palettes\n";
    }
    
    // Step 9: Render the debugger in a loop
    std::cout << "Step 9: Rendering debugger...\n";
    std::cout << "  Note: In a real application with a window system (SDL/GLFW),\n";
    std::cout << "        you would call Render() in your main loop each frame.\n";
    std::cout << "        For this example, we simulate a few render calls:\n\n";
    
    // Simulate a few frames of rendering
    for (int frame = 0; frame < 5; frame++) {
        std::cout << "  Frame " << (frame + 1) << ": Calling debugger.Render()...\n";
        debugger.Render();
        
        // Simulate CPU state changes between frames
        cycle += 1000;
        pc += 3;
        debugger.UpdateCPU(cycle, pc, sp, af, bc, de, hl, ime);
    }
    
    std::cout << "  ✓ Render loop completed\n\n";
    
    std::cout << "  In a real emulator integration, your main loop would look like:\n";
    std::cout << "    while (running) {\n";
    std::cout << "      // Run emulator for one frame\n";
    std::cout << "      emulator.RunFrame();\n";
    std::cout << "      \n";
    std::cout << "      // Update debugger with current state\n";
    std::cout << "      debugger.UpdateCPU(cpu.cycle, cpu.pc, cpu.sp, ...);\n";
    std::cout << "      debugger.UpdateMemory(memory.data(), memory.size());\n";
    std::cout << "      debugger.UpdateVRAM(vram.data(), vram.size(), bank);\n";
    std::cout << "      debugger.UpdateOAM(oam.data(), oam.size());\n";
    std::cout << "      debugger.UpdatePalettes(bgPalettes, spritePalettes);\n";
    std::cout << "      \n";
    std::cout << "      // Render debugger UI\n";
    std::cout << "      debugger.Render();\n";
    std::cout << "      \n";
    std::cout << "      // Swap buffers, handle events, etc.\n";
    std::cout << "    }\n\n";
    
    // Step 10: Close the debugger
    std::cout << "Step 10: Closing debugger...\n";
    debugger.Close();
    std::cout << "  ✓ Debugger closed\n";
    
    // Verify the debugger is closed
    if (!debugger.IsOpen()) {
        std::cout << "  ✓ Debugger is closed (IsOpen() returned false)\n\n";
    }
    
    std::cout << "============================\n";
    std::cout << "Example completed successfully!\n";
    std::cout << "============================\n\n";
    
    std::cout << "Summary of API usage:\n";
    std::cout << "  1. GBDebug::GBDebugger debugger;           // Create instance\n";
    std::cout << "  2. debugger.Open();                        // Open window\n";
    std::cout << "  3. debugger.UpdateCPU(...);                // Update CPU state\n";
    std::cout << "  4. debugger.UpdateMemory(buffer, size);    // Update memory\n";
    std::cout << "  5. debugger.SetEmulationMode(mode);        // Set DMG/CGB mode\n";
    std::cout << "  6. debugger.UpdateVRAM(buffer, size, bank);// Update VRAM\n";
    std::cout << "  7. debugger.UpdateOAM(buffer, size);       // Update OAM\n";
    std::cout << "  8. debugger.UpdatePalettes(bg, sprite);    // Update palettes\n";
    std::cout << "  9. debugger.Render();                      // Render (in loop)\n";
    std::cout << "  10. debugger.Close();                      // Close window\n";
    
    return 0;
}
