#ifndef MEMORY_VIEWER_PANEL_H
#define MEMORY_VIEWER_PANEL_H

#include "IDebuggerPanel.h"
#include "DebuggerTypes.h"

namespace GBDebug {

/**
 * BankSource - Indicates the source of memory data for a region
 * 
 * Used by MemoryViewerPanel to determine whether to display data from
 * the currently mapped memory or from a specific bank.
 */
enum class BankSource {
    MappedMemory,  // Use currently mapped memory from the 64KB buffer
    SpecificBank   // Use data from a specific bank number
};

/**
 * RegionBankState - Tracks bank selection state for a memory region
 * 
 * Stores the current bank selection for regions that support bank switching
 * (VRAM, ROM banks, RAM banks). The source indicates whether to use mapped
 * memory or a specific bank, and bankNumber specifies which bank when
 * source is SpecificBank.
 */
struct RegionBankState {
    BankSource source;
    uint16_t bankNumber;  // Used when source == SpecificBank
    
    RegionBankState() 
        : source(BankSource::MappedMemory), bankNumber(0) {}
};

/**
 * MemoryViewerPanel - Displays the full 64KB memory space with region highlighting
 * 
 * Renders a scrollable hex dump of the entire GameBoy memory map with:
 * - Color-coded memory regions (ROM, VRAM, RAM, I/O, etc.)
 * - Hexadecimal and ASCII representation side by side
 * - Region headers showing address ranges
 * - Bank selection dropdowns for VRAM, ROM, and RAM regions
 * 
 * Usage:
 *   1. Call Update() with a 64KB memory buffer after each emulator step
 *   2. Optionally call SetBankData() to enable bank selection
 *   3. Call Render() each frame to draw the panel
 */
class MemoryViewerPanel : public IDebuggerPanel {
public:
    MemoryViewerPanel();
    ~MemoryViewerPanel() override = default;
    
    // IDebuggerPanel interface
    void Render() override;
    const char* GetName() const override { return "Memory Viewer"; }
    bool IsVisible() const override { return visible_; }
    void SetVisible(bool visible) override { visible_ = visible; }
    
    /**
     * Update the memory state to display
     * @param buffer Pointer to 64KB memory buffer
     * @param size Size of buffer (must be 65536)
     * @return true if successful
     */
    bool Update(const uint8_t* buffer, size_t size);
    
    /**
     * Set bank data for viewing individual banks
     * 
     * Enables bank selection dropdowns in the UI for VRAM, ROM, and RAM regions.
     * The panel does not take ownership of the BankData - caller retains ownership.
     * 
     * @param bankData Pointer to BankData structure (nullptr to disable bank selection)
     */
    void SetBankData(const BankData* bankData);

private:
    void RenderMemoryRegion(const MemoryRegion& region);
    void RenderIORegisters();
    
    /**
     * Render a bank selection dropdown for a memory region
     * @param label Label for the dropdown
     * @param state Reference to the region's bank state
     * @param maxBank Maximum bank number available
     */
    void RenderBankSelector(const char* label, RegionBankState& state, uint16_t maxBank);
    
    /**
     * Get the data source for a given address based on bank selection
     * @param address Memory address to read
     * @param state Bank state for the region containing the address
     * @return Byte value at the address from the selected source
     */
    uint8_t GetDataSource(uint16_t address, const RegionBankState& state) const;
    
    MemoryState state_;
    bool visible_;
    
    // Bank selection state per region
    RegionBankState vramBankState_;
    RegionBankState romBankState_;
    RegionBankState ramBankState_;
    
    // Non-owning pointer to bank data
    const BankData* bankData_;
};

} // namespace GBDebug

#endif // MEMORY_VIEWER_PANEL_H
