#ifndef CPU_STATE_PANEL_H
#define CPU_STATE_PANEL_H

#include "IDebuggerPanel.h"
#include "DebuggerTypes.h"

namespace GBDebug {

/**
 * CPUStatePanel - Displays CPU register values and cycle count
 */
class CPUStatePanel : public IDebuggerPanel {
public:
    CPUStatePanel();
    ~CPUStatePanel() override = default;
    
    // IDebuggerPanel interface
    void Render() override;
    const char* GetName() const override { return "CPU State"; }
    bool IsVisible() const override { return visible_; }
    void SetVisible(bool visible) override { visible_ = visible; }
    
    /**
     * Update the CPU state to display
     */
    void Update(const CPUState& state);

private:
    CPUState state_;
    bool visible_;
};

} // namespace GBDebug

#endif // CPU_STATE_PANEL_H
