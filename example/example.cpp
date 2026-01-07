#include "GBDebugger.h"
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
 * 5. Pass bank data for memory bank viewing (VRAM, ROM, RAM)
 * 6. Render the debugger in a loop
 * 7. Close the debugger
 * 
 * Requirements: 7.1, 7.2, 7.3, 7.4, 7.5
 * Bank Viewing Requirements: 1.1, 1.2, 5.1, 7.1
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
    
    // Set CGB flag to indicate CGB mode (0x80 = CGB compatible)
    memory[0x0143] = 0x80;
    
    // Sample code at PC location (0x0150)
    memory[0x0150] = 0x3E;  // LD A, 0x42
    memory[0x0151] = 0x42;
    memory[0x0152] = 0x06;  // LD B, 0x10
    memory[0x0153] = 0x10;
    memory[0x0154] = 0xC3;  // JP 0x0100
    memory[0x0155] = 0x00;
    memory[0x0156] = 0x01;
    
    // Fill VRAM with pattern (0x8000-0x9FFF)
    for (size_t i = 0x8000; i < 0xA000; i++) {
        memory[i] = static_cast<uint8_t>((i & 0xFF));
    }
    
    // Fill WRAM with test data (0xC000-0xDFFF)
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

    // ========================================================================
    // Step 5: Pass bank data for memory bank viewing
    // ========================================================================
    // 
    // The bank data API allows inspection of individual memory banks
    // independently of what is currently mapped into the GameBoy's address space.
    // This is essential for debugging bank-switched games.
    //
    // Key points:
    // - The debugger does NOT take ownership of the memory (caller retains ownership)
    // - Pointers must remain valid while the debugger is using them
    // - Bank data is optional - the debugger works without it (backward compatible)
    // - When bank data is provided, UI shows bank selection dropdowns
    // ========================================================================
    
    std::cout << "Step 5: Setting up bank data for memory bank viewing...\n";
    
    // ---- 5a: VRAM Bank Data ----
    // CGB has 2 VRAM banks (8KB each). DMG has only 1 bank.
    // Create sample VRAM banks with identifiable patterns.
    
    std::cout << "  5a: Setting VRAM bank data...\n";
    
    // Create 8KB VRAM bank 0
    std::vector<uint8_t> vramBank0(8192, 0);
    
    // Create sample tiles in VRAM bank 0
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
    
    // Tile 2: Horizontal stripes
    for (int row = 0; row < 8; row++) {
        uint8_t pattern = (row % 2 == 0) ? 0xFF : 0x00;
        vramBank0[32 + row * 2] = pattern;
        vramBank0[32 + row * 2 + 1] = pattern;
    }
    
    // Fill remaining tiles with generated patterns
    for (int tile = 3; tile < 384; tile++) {
        int baseAddr = tile * 16;
        for (int row = 0; row < 8; row++) {
            vramBank0[baseAddr + row * 2] = static_cast<uint8_t>((tile + row) & 0xFF);
            vramBank0[baseAddr + row * 2 + 1] = static_cast<uint8_t>((tile * 2 + row) & 0xFF);
        }
    }
    
    // Create 8KB VRAM bank 1 (CGB only) with inverted patterns
    std::vector<uint8_t> vramBank1(8192, 0);
    for (size_t i = 0; i < vramBank1.size(); i++) {
        vramBank1[i] = static_cast<uint8_t>(~vramBank0[i]);
    }
    
    // Pass VRAM bank 0 (8KB)
    if (debugger.SetVRAMBank(0, vramBank0.data())) {
        std::cout << "    ✓ VRAM bank 0 set (8192 bytes)\n";
    } else {
        std::cerr << "    ✗ Failed to set VRAM bank 0\n";
    }
    
    // Pass VRAM bank 1 (8KB, CGB only)
    if (debugger.SetVRAMBank(1, vramBank1.data())) {
        std::cout << "    ✓ VRAM bank 1 set (8192 bytes)\n";
    } else {
        std::cerr << "    ✗ Failed to set VRAM bank 1\n";
    }
    
    std::cout << "    Note: VRAM Viewer now shows bank selection dropdown\n";
    std::cout << "          Options: 'Mapped Memory', 'Bank 0', 'Bank 1'\n\n";
    
    // ---- 5b: ROM Bank Data ----
    // ROM banks are 16KB each. A typical cartridge has 2-512 banks.
    // We simulate a cartridge with 4 ROM banks for this example.
    
    std::cout << "  5b: Setting ROM bank data...\n";
    
    const uint16_t ROM_BANK_COUNT = 4;
    const size_t ROM_BANK_SIZE = 16384;  // 16KB per bank
    
    // Create sample ROM banks
    std::vector<std::vector<uint8_t>> romBanks(ROM_BANK_COUNT);
    for (uint16_t bank = 0; bank < ROM_BANK_COUNT; bank++) {
        romBanks[bank].resize(ROM_BANK_SIZE);
        
        // Fill each bank with identifiable patterns
        for (size_t i = 0; i < ROM_BANK_SIZE; i++) {
            // Pattern: bank number in high nibble, offset in low nibble
            romBanks[bank][i] = static_cast<uint8_t>((bank << 4) | (i & 0x0F));
        }
        
        // Add bank identifier at the start
        romBanks[bank][0] = 'R';
        romBanks[bank][1] = 'O';
        romBanks[bank][2] = 'M';
        romBanks[bank][3] = static_cast<uint8_t>('0' + bank);
    }
    
    // Pass ROM banks using a lambda that returns pointers to each bank
    // The lambda captures romBanks by reference - ensure it stays valid!
    if (debugger.SetROMBanks(ROM_BANK_COUNT, 
            [&romBanks](uint16_t bank) -> const uint8_t* {
                if (bank < romBanks.size()) {
                    return romBanks[bank].data();
                }
                return nullptr;
            })) {
        std::cout << "    ✓ ROM banks set (" << ROM_BANK_COUNT << " banks, 16KB each)\n";
        std::cout << "      - Bank 0: Contains 'ROM0' identifier\n";
        std::cout << "      - Bank 1: Contains 'ROM1' identifier\n";
        std::cout << "      - Bank 2: Contains 'ROM2' identifier\n";
        std::cout << "      - Bank 3: Contains 'ROM3' identifier\n";
    } else {
        std::cerr << "    ✗ Failed to set ROM banks\n";
    }
    
    std::cout << "    Note: Memory Viewer shows ROM bank dropdown for 0x4000-0x7FFF\n\n";

    // ---- 5c: RAM Bank Data ----
    // Cartridge RAM banks vary in size (512 bytes for MBC2, 8KB for others).
    // We simulate a cartridge with 4 RAM banks of 8KB each.
    
    std::cout << "  5c: Setting RAM bank data...\n";
    
    const uint8_t RAM_BANK_COUNT = 4;
    const size_t RAM_BANK_SIZE = 8192;  // 8KB per bank (typical for MBC1/3/5)
    
    // Create sample RAM banks
    std::vector<std::vector<uint8_t>> ramBanks(RAM_BANK_COUNT);
    for (uint8_t bank = 0; bank < RAM_BANK_COUNT; bank++) {
        ramBanks[bank].resize(RAM_BANK_SIZE);
        
        // Fill each bank with identifiable patterns
        for (size_t i = 0; i < RAM_BANK_SIZE; i++) {
            // Pattern: inverted bank number + offset
            ramBanks[bank][i] = static_cast<uint8_t>((~bank << 4) | (i & 0x0F));
        }
        
        // Add bank identifier at the start
        ramBanks[bank][0] = 'S';  // "SRAM" for Save RAM
        ramBanks[bank][1] = 'R';
        ramBanks[bank][2] = 'A';
        ramBanks[bank][3] = 'M';
        ramBanks[bank][4] = static_cast<uint8_t>('0' + bank);
    }
    
    // Pass RAM banks using a lambda
    if (debugger.SetRAMBanks(RAM_BANK_COUNT, RAM_BANK_SIZE,
            [&ramBanks](uint8_t bank) -> const uint8_t* {
                if (bank < ramBanks.size()) {
                    return ramBanks[bank].data();
                }
                return nullptr;
            })) {
        std::cout << "    ✓ RAM banks set (" << (int)RAM_BANK_COUNT << " banks, 8KB each)\n";
        std::cout << "      - Bank 0: Contains 'SRAM0' identifier\n";
        std::cout << "      - Bank 1: Contains 'SRAM1' identifier\n";
        std::cout << "      - Bank 2: Contains 'SRAM2' identifier\n";
        std::cout << "      - Bank 3: Contains 'SRAM3' identifier\n";
    } else {
        std::cerr << "    ✗ Failed to set RAM banks\n";
    }
    
    std::cout << "    Note: Memory Viewer shows RAM bank dropdown for 0xA000-0xBFFF\n\n";
    
    // ---- Summary of Bank Data API ----
    std::cout << "  Bank Data API Summary:\n";
    std::cout << "    - SetVRAMBank(bank, data): Set VRAM bank 0 or 1 (8KB each)\n";
    std::cout << "    - SetROMBanks(count, getter): Set ROM banks (16KB each, up to 512)\n";
    std::cout << "    - SetRAMBanks(count, size, getter): Set RAM banks (size varies)\n";
    std::cout << "    - ClearBankData(): Reset all bank data (revert to mapped memory)\n\n";
    
    std::cout << "  Integration with GBLib:\n";
    std::cout << "    // In your emulator main loop:\n";
    std::cout << "    debugger.SetVRAMBank(0, gameboy.GetVRAMBank(0));\n";
    std::cout << "    debugger.SetVRAMBank(1, gameboy.GetVRAMBank(1));\n";
    std::cout << "    debugger.SetROMBanks(gameboy.GetROMBankCount(),\n";
    std::cout << "        [&gameboy](uint16_t bank) { return gameboy.GetROMBank(bank); });\n";
    std::cout << "    debugger.SetRAMBanks(gameboy.GetRAMBankCount(), gameboy.GetRAMBankSize(),\n";
    std::cout << "        [&gameboy](uint8_t bank) { return gameboy.GetRAMBank(bank); });\n\n";
    
    // Step 6: Render the debugger in a loop
    std::cout << "Step 6: Rendering debugger...\n";
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
    std::cout << "      \n";
    std::cout << "      // Update bank data for bank viewing (optional)\n";
    std::cout << "      debugger.SetVRAMBank(0, gameboy.GetVRAMBank(0));\n";
    std::cout << "      debugger.SetVRAMBank(1, gameboy.GetVRAMBank(1));\n";
    std::cout << "      // ROM/RAM banks typically set once at cartridge load\n";
    std::cout << "      \n";
    std::cout << "      // Render debugger UI\n";
    std::cout << "      debugger.Render();\n";
    std::cout << "      \n";
    std::cout << "      // Swap buffers, handle events, etc.\n";
    std::cout << "    }\n\n";

    // Step 7: Demonstrate clearing bank data (optional)
    std::cout << "Step 7: Demonstrating ClearBankData()...\n";
    std::cout << "  Calling debugger.ClearBankData() reverts to mapped memory only.\n";
    std::cout << "  This is useful when unloading a cartridge or resetting.\n";
    debugger.ClearBankData();
    std::cout << "  ✓ Bank data cleared - UI now shows only 'Mapped Memory' option\n\n";
    
    // Step 8: Close the debugger
    std::cout << "Step 8: Closing debugger...\n";
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
    std::cout << "  5. debugger.SetVRAMBank(bank, data);       // Set VRAM bank data\n";
    std::cout << "     debugger.SetROMBanks(count, getter);    // Set ROM bank data\n";
    std::cout << "     debugger.SetRAMBanks(count, size, getter); // Set RAM bank data\n";
    std::cout << "  6. debugger.Render();                      // Render (in loop)\n";
    std::cout << "  7. debugger.ClearBankData();               // Clear bank data (optional)\n";
    std::cout << "  8. debugger.Close();                       // Close window\n";
    
    return 0;
}
