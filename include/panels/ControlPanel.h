#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "IDebuggerPanel.h"

namespace GBDebug {

/**
 * ControlPanel - Provides debugger control buttons
 * 
 * This panel displays Run/Stop, Step, Exit, and Speed controls for
 * the emulator execution. It communicates state changes back to the
 * main application through getter methods.
 * 
 * Usage:
 *   panel.Render();
 *   if (panel.IsStepRequested()) { step }
 *   if (panel.IsRunning()) { run }
 *   if (panel.IsExitRequested()) { exit }
 *   int multiplier = panel.GetSpeedMultiplier();
 */
class ControlPanel : public IDebuggerPanel {
public:
    ControlPanel();
    
    void Render() override;
    const char* GetName() const override { return "Controls"; }
    bool IsVisible() const override { return visible_; }
    void SetVisible(bool visible) override { visible_ = visible; }
    
    // State accessors
    bool IsRunning() const { return running_; }
    void SetRunning(bool running) { running_ = running; }
    void ToggleRunning() { running_ = !running_; }
    
    bool IsStepRequested() const { return step_requested_; }
    void ClearStepRequest() { step_requested_ = false; }
    
    bool IsExitRequested() const { return exit_requested_; }
    
    // Speed control: 0=1x, 1=2x, 2=4x, 3=8x
    int GetSpeedMultiplier() const;
    void CycleSpeed(); // Cycles through 1x -> 2x -> 4x -> 8x -> 1x
    void SetSpeedIndex(int index) { speed_index_ = index % 4; }
    int GetSpeedIndex() const { return speed_index_; }

private:
    bool visible_;
    bool running_;
    bool step_requested_;
    bool exit_requested_;
    int speed_index_; // 0=1x, 1=2x, 2=4x, 3=8x
};

} // namespace GBDebug

#endif // CONTROLPANEL_H
