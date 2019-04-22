////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __GUI_IVIEW_H__
#define __GUI_IVIEW_H__


#include <NsCore/Noesis.h>
#include <NsCore/Interface.h>
#include <NsGui/InputEnums.h>


namespace Noesis
{

class FrameworkElement;
template<class T> class Delegate;
NS_INTERFACE IRenderer;

////////////////////////////////////////////////////////////////////////////////////////////////////
struct TessellationMaxPixelError
{
    TessellationMaxPixelError(float error_): error(error_) {}

    /// MediumQuality is usually fine for PPAA (non-multisampled) while HighQuality is the 
    /// recommended pixel error if you are rendering to a 8x multisampled surface
    static TessellationMaxPixelError LowQuality() { return 0.7f; }
    static TessellationMaxPixelError MediumQuality() { return 0.4f; }
    static TessellationMaxPixelError HighQuality() { return 0.2f; }

    float error;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
enum RenderFlags
{
    /// Toggles wireframe mode when rendering triangles
    RenderFlags_Wireframe = 1,

    /// Each batch submitted to the GPU is given a unique solid color
    RenderFlags_ColorBatches = 2,

    /// Display pixel overdraw using blending layers. Different colors are used for each type
    /// of triangles. 'Green' for normal ones, 'Red' for opacities and 'Blue' for clipping masks
    RenderFlags_Overdraw = 4,

    /// Inverts the render vertically
    RenderFlags_FlipY = 8,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
struct ViewStats
{
    float frameTime;
    float updateTime;
    float renderTime;

    uint32_t triangles;
    uint32_t draws;
    uint32_t batches;

    uint32_t tessellations;
    uint32_t flushes;
    uint32_t geometrySize;

    uint32_t masks;
    uint32_t opacities;
    uint32_t renderTargetSwitches;

    uint32_t uploadedRamps;
    uint32_t rasterizedGlyphs;
    uint32_t discardedGlyphTiles;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Views manage UI trees
////////////////////////////////////////////////////////////////////////////////////////////////////
NS_INTERFACE IView: public Interface
{
    /// Returns the renderer associated to this view
    virtual IRenderer* GetRenderer() = 0;

    /// Returns the root element
    virtual FrameworkElement* GetContent() const = 0;

    /// Sets the size of the surface where UI elements will layout and render
    virtual void SetSize(uint32_t width, uint32_t height) = 0;

    /// Per-Primitive Antialiasing extrudes the contours of the geometry and smooths them.
    /// It is a 'cheap' antialiasing algorithm useful when GPU MSAA is not enabled
    virtual void SetIsPPAAEnabled(bool enabled) = 0;

    /// Returns true if Per-Primitive Antialiasing is enabled
    virtual bool GetIsPPAAEnabled() const = 0;

    /// Sets the tessellation curve tolerance in screen space. MediumQuality is the default value
    virtual void SetTessellationMaxPixelError(TessellationMaxPixelError maxError) = 0;

    /// Gets current tessellation curve tolerance
    virtual TessellationMaxPixelError GetTessellationMaxPixelError() const = 0;

    /// Enables debugging render flags. No debug flags are active by default
    virtual void SetFlags(uint32_t flags) = 0;

    /// Returns current debugging render flags
    virtual uint32_t GetFlags() const = 0;

    /// Activates the view and recovers keyboard focus
    virtual void Activate() = 0;

    /// Deactivates the view and removes keyboard focus
    virtual void Deactivate() = 0;

    /// Notifies that a mouse button was pressed. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool MouseButtonDown(int x, int y, MouseButton button) = 0;

    /// Notifies that a mouse button was released. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool MouseButtonUp(int x, int y, MouseButton button) = 0;

    /// Notifies that a mouse button was double clicked. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool MouseDoubleClick(int x, int y, MouseButton button) = 0;

    /// Notifies that mouse was moved. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool MouseMove(int x, int y) = 0;

    /// Notifies that mouse vertical wheel was rotated. Origin is in the upper-left corner
    /// The parameter wheelRotation indicates the distance the wheel is rotated, expressed in
    /// multiples or divisions of 120 (is the default value for 3 lines of scrolling). A positive
    /// value indicates that the wheel was rotated forward, away from the user; a negative value
    /// indicates that the wheel was rotated backward, toward the user.
    /// Returns true if event was handled
    virtual bool MouseWheel(int x, int y, int wheelRotation) = 0;

    /// Notifies that mouse horizontal wheel was rotated. Origin is in the upper-left corner
    /// The parameter wheelRotation indicates the distance the wheel is rotated, expressed in
    /// multiples or divisions of 120 (is the default value for 3 lines of scrolling). A positive
    /// value indicates that the wheel was rotated to the right; a negative value indicates that
    /// the wheel was rotated to the left.
    /// Returns true if event was handled
    virtual bool MouseHWheel(int x, int y, int wheelRotation) = 0;

    /// Notifies that a vertical scroll is being actioned. This is normally mapped to a gamepad
    /// stick. Value ranges from -1.0 (fully pressed down) to +1.0 (fully pressed up). The value
    /// is internally adjusted to be frame-rate independent. No dead zone filtering is applied.
    /// Returns true if event was handled
    virtual bool Scroll(float value) = 0;

    /// Notifies that a horizontal scroll is being actioned. This is normally mapped to a gamepad
    /// stick. Value ranges from -1.0 (fully pressed left) to +1.0 (fully pressed right). The value
    /// is internally adjusted to be frame-rate independent. No dead zone filtering is applied.
    /// Returns true if event was handled
    virtual bool HScroll(float value) = 0;

    /// Notifies that a finger touched the screen. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool TouchDown(int x, int y, uint64_t id) = 0;

    /// Notifies that a finger moved on the screen. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool TouchMove(int x, int y, uint64_t id) = 0;

    /// Notifies that a finger raised off of the screen. Origin is in the upper-left corner.
    /// Returns true if event was handled
    virtual bool TouchUp(int x, int y, uint64_t id) = 0;

    /// Notifies that a key was pressed.
    /// Returns true if event was handled
    virtual bool KeyDown(Key key) = 0;

    /// Notifies that a key was released.
    /// Returns true if event was handled
    virtual bool KeyUp(Key key) = 0;

    /// Notifies that a key was translated to the corresponding unicode character.
    /// Returns true if event was handled
    virtual bool Char(uint32_t ch) = 0;

    /// Performs layout pass and calculates changes to be consumed by the renderer.
    /// Returns false if there were no changes since last frame indicating render can be avoided
    virtual bool Update(double timeInSeconds) = 0;

    /// Rendering event occurs after animation and layout have been applied to the composition
    /// tree, just before objects in the composition tree are rendered
    typedef Delegate<void (IView* view)> RenderingEventHandler;
    virtual RenderingEventHandler& Rendering() = 0;

    /// Creates a timer that will be fired at the given milliseconds interval. Returns an ID to
    /// handle restarts and cancellations. The callback returns the next interval or just 0 to stop 
    typedef Delegate<uint32_t ()> TimerCallback;
    virtual uint32_t CreateTimer(uint32_t interval, const TimerCallback& callback) = 0;

    /// Restarts specified timer with a new interval in milliseconds
    virtual void RestartTimer(uint32_t timerId, uint32_t interval) = 0;

    /// Cancels specified timer
    virtual void CancelTimer(uint32_t timerId) = 0;

    /// Gets stats counters
    virtual ViewStats GetStats() const = 0;

    NS_IMPLEMENT_INLINE_REFLECTION_(IView, Interface)
};

}

#endif
