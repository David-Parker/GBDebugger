#ifndef IDEBUGGER_PANEL_H
#define IDEBUGGER_PANEL_H

namespace GBDebug {

/**
 * Interface for debugger panels
 * 
 * Each panel is a self-contained UI component that can render itself
 * using ImGui. Panels receive data through their specific update methods
 * and render that data when Render() is called.
 */
class IDebuggerPanel {
public:
    virtual ~IDebuggerPanel() = default;
    
    /**
     * Render the panel using ImGui
     * Called once per frame when the debugger is visible
     */
    virtual void Render() = 0;
    
    /**
     * Get the panel's display name
     * Used for window titles and identification
     */
    virtual const char* GetName() const = 0;
    
    /**
     * Check if the panel is visible
     */
    virtual bool IsVisible() const = 0;
    
    /**
     * Set panel visibility
     */
    virtual void SetVisible(bool visible) = 0;
};

} // namespace GBDebug

#endif // IDEBUGGER_PANEL_H
