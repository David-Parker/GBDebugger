#ifndef MEMORY_VIEWER_PANEL_H
#define MEMORY_VIEWER_PANEL_H

#include "IDebuggerPanel.h"
#include "DebuggerTypes.h"

namespace GBDebug {

/**
 * MemoryViewerPanel - Displays the full 64KB memory space with region highlighting
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

private:
    MemoryState state_;
    bool visible_;
};

} // namespace GBDebug

#endif // MEMORY_VIEWER_PANEL_H
