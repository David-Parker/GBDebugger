#ifndef FLAGS_PANEL_H
#define FLAGS_PANEL_H

#include "IDebuggerPanel.h"
#include "DebuggerTypes.h"

namespace GBDebug {

/**
 * FlagsPanel - Displays CPU flag states with visual indicators
 */
class FlagsPanel : public IDebuggerPanel {
public:
    FlagsPanel();
    ~FlagsPanel() override = default;
    
    // IDebuggerPanel interface
    void Render() override;
    const char* GetName() const override { return "CPU Flags"; }
    bool IsVisible() const override { return visible_; }
    void SetVisible(bool visible) override { visible_ = visible; }
    
    /**
     * Update the CPU state (flags are extracted from AF register)
     */
    void Update(const CPUState& state);

private:
    CPUState state_;
    bool visible_;
};

} // namespace GBDebug

#endif // FLAGS_PANEL_H
