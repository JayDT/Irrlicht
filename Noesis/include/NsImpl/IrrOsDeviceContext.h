#ifndef __APP_IRR_OS_CONTEXT_H__
#define __APP_IRR_OS_CONTEXT_H__

#include <NsApp/IrrNoesis.h>
#include <NsCore/Noesis.h>
#include <NsGui/InputEnums.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/Delegate.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsCore/ReflectionDeclareEnum.h>

#include "include/IEventReceiver.h"
#include "include/irrlicht.h"

#undef PlaySound

namespace irr
{
    class IrrlichtDevice;
}

namespace NoesisApp
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Specifies whether a window can be resized and, if so, how it can be resized
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum ResizeMode
    {
        /// A window can only be minimized and restored. The Minimize and Maximize buttons are both
        /// shown, but only the Minimize button is enabled
        ResizeMode_CanMinimize,

        /// A window can be resized. The Minimize and Maximize buttons are both shown and enabled
        ResizeMode_CanResize,

        /// A window can be resized. The Minimize and Maximize buttons are both shown and enabled.
        /// A resize grip appears in the bottom-right corner of the window
        ResizeMode_CanResizeWithGrip,

        /// A window cannot be resized. The Minimize and Maximize buttons are not displayed in the
        /// title bar
        ResizeMode_NoResize
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Specifies whether a window is minimized, maximized, or restored
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum WindowState
    {
        /// The window is maximized
        WindowState_Maximized,

        /// The window is minimized
        WindowState_Minimized,

        /// The window is restored
        WindowState_Normal

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Specifies the type of border that a Window has
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum WindowStyle
    {
        /// Only the client area is visible - the title bar and border are not shown
        WindowStyle_None,

        /// A window with a single border. This is the default value
        WindowStyle_SingleBorderWindow,

        /// A window with a 3-D border
        WindowStyle_ThreeDBorderWindow,

        /// A fixed tool window
        WindowStyle_ToolWindow
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Specifies the position that a Window will be shown in when it is first opened
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum WindowStartupLocation
    {
        /// The startup location of a Window is set from code, or defers to the default location
        WindowStartupLocation_Manual,

        /// The startup location of a Window is the center of the screen that contains the mouse cursor
        WindowStartupLocation_CenterScreen,

        /// The startup location of a Window is the center of the Window that owns it, as specified by
        /// the Window Owner property
        WindowStartupLocation_CenterOwner
    };

    NS_WARNING_PUSH
    NS_MSVC_WARNING_DISABLE(4251)

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Abstraction for operating system windows 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class NS_IRR_NOESIS_API IrrNsDeviceStub
        : public Noesis::BaseComponent
        , public irr::IEventReceiver
    {
    public:

        IrrNsDeviceStub();
		virtual ~IrrNsDeviceStub();

		void SetIrrlichtDevice(irr::IrrlichtDevice* contextDevice);
		irr::IrrlichtDevice* GetIrrDevice() const { return mContextDevice.GetPtr(); }

        /// Stores custom private data
        static void SetPrivateData(void* data);

        /// Returns stored private data
        static void* GetPrivateData();

        /// Changes the text of the title bar (if it has one)
        void SetTitle(const char* title);

        /// Changes position
        void SetLocation(int x, int y);

        /// Changes size
        void SetSize(uint32_t width, uint32_t height);

		/// Changes size
		void GetSize(uint32_t& width, uint32_t& height);

        /// Sets window border style
        void SetWindowStyle(WindowStyle windowStyle);

        /// Sets window state
        void SetWindowState(WindowState windowState);

        /// Sets window resize mode
        void SetResizeMode(ResizeMode resizeMode);

        /// Sets if the window has a task bar button 
        void SetShowInTaskbar(bool showInTaskbar);

        /// Sets if a window appears in the topmost z-order
        void SetTopmost(bool topmost);

        /// Sets if this element can be used as the target of a drag-and-drop operation
        void SetAllowFileDrop(bool allowFileDrop);

        /// Sets the position of the window when first shown
        void SetWindowStartupLocation(WindowStartupLocation location);

        /// Sets the mouse capture
        void Capture();

        /// Unsets the mouse capture
        void ReleaseCapture();

        /// Sets the keyboard focus
        void Focus();

        /// Activates the window and displays it in its current size and position
        void Show();

        /// Destroys window
        void Close();

        /// Opens on-screen keyboard
        void OpenSoftwareKeyboard(Noesis::UIElement* focused);

        /// Closes on-screen keyboard
        void CloseSoftwareKeyboard();

        /// Updates mouse cursor icon
        void SetCursor(Noesis::Cursor cursor);

        /// Opens URL in a browser
        void OpenUrl(const char* url);

        /// Gets system window handle
        void* GetNativeHandle() const;

        /// Returns the width of the window client area
        uint32_t GetClientWidth() const;

        /// Returns the height of the window client area
        uint32_t GetClientHeight() const;

        /// Plays audio file
        void PlaySound(const char* filename, float volume);

        /// Halts the currently playing audio
        void PauseAudio();

        /// Resumes playback
        void ResumeAudio();

        /// Occurs when the window's location changes
        typedef bool LocationChangedT(IrrNsDeviceStub* display, int x, int y);
        Noesis::Delegate<LocationChangedT>& LocationChanged();

        /// Occurs when the window's size changes
        typedef bool SizeChangedT(IrrNsDeviceStub* display, uint32_t width, uint32_t height);
        Noesis::Delegate<SizeChangedT>& SizeChanged();

        /// Occurs when the window's state changes
        typedef bool StateChangedT(IrrNsDeviceStub* display, WindowState state);
        Noesis::Delegate<StateChangedT>& StateChanged();

        /// Occurs when files are dragged and dropped over the window
        typedef bool FileDroppedT(IrrNsDeviceStub* display, const char* filename);
        Noesis::Delegate<FileDroppedT>& FileDropped();

        /// Occurs when a window becomes the foreground window
        typedef bool ActivatedT(IrrNsDeviceStub* display);
        Noesis::Delegate<ActivatedT>& Activated();

        /// Occurs when a window becomes a background window
        typedef bool DeactivatedT(IrrNsDeviceStub* display);
        Noesis::Delegate<DeactivatedT>& Deactivated();

        /// Occurs when window needs to render a frame
        typedef bool RenderT(IrrNsDeviceStub* display);
        Noesis::Delegate<RenderT>& Render();

        /// Occurs when mouse is moved over the window
        typedef bool MouseMoveT(IrrNsDeviceStub* display, int x, int y);
        Noesis::Delegate<MouseMoveT>& MouseMove();

        /// Occurs when a mouse button is pressed over the window
        typedef bool MouseButtonDownT(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        Noesis::Delegate<MouseButtonDownT>& MouseButtonDown();

        /// Occurs when a mouse button is released over the window
        typedef bool MouseButtonUpT(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        Noesis::Delegate<MouseButtonUpT>& MouseButtonUp();

        /// Occurs when a mouse button is double clicked over the window
        typedef bool MouseDoubleClickT(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        Noesis::Delegate<MouseDoubleClickT>& MouseDoubleClick();

        /// Occurs when mouse vertical wheel is rotated over the window
        typedef bool MouseWheelT(IrrNsDeviceStub* display, int x, int y, int delta);
        Noesis::Delegate<MouseWheelT>& MouseWheel();

        /// Occurs when mouse horizontal wheel is rotated over the window
        typedef bool MouseHWheelT(IrrNsDeviceStub* display, int x, int y, int delta);
        Noesis::Delegate<MouseHWheelT>& MouseHWheel();

        /// Occurs when vertical scroll axis is actioned
        typedef bool ScrollT(IrrNsDeviceStub* display, float value);
        Noesis::Delegate<ScrollT>& Scroll();

        /// Occurs when horizontal scroll axis is actioned
        typedef bool HScrollT(IrrNsDeviceStub* display, float value);
        Noesis::Delegate<HScrollT>& HScroll();

        /// Occurs when a key is pressed over the window
        typedef bool KeyDownT(IrrNsDeviceStub* display, Noesis::Key key);
        Noesis::Delegate<KeyDownT>& KeyDown();

        /// Occurs when a key is released over the window
        typedef bool KeyUpT(IrrNsDeviceStub* display, Noesis::Key key);
        Noesis::Delegate<KeyUpT>& KeyUp();

        /// Occurs when a key is translated to the corresponding character over the window
        typedef bool CharT(IrrNsDeviceStub* display, uint32_t c);
        Noesis::Delegate<CharT>& Char();

        /// Occurs when a finger touches the window
        typedef bool TouchDownT(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        Noesis::Delegate<TouchDownT>& TouchDown();

        /// Occurs when a finger moves on the window
        typedef bool TouchMoveT(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        Noesis::Delegate<TouchMoveT>& TouchMove();

        /// Occurs when a finger raises off of the window
        typedef bool TouchUpT(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        Noesis::Delegate<TouchUpT>& TouchUp();

        /// Occurs when a window becomes a background window
        typedef bool ClosingT(IrrNsDeviceStub* display);
        Noesis::Delegate<ClosingT>& Closing();

        /// Occurs when a window becomes a background window
        typedef bool ClosedT(IrrNsDeviceStub* display);
        Noesis::Delegate<ClosedT>& Closed();


    protected:

        // Inherited via IEventReceiver
        bool OnEvent(const irr::SEvent& event) override final;

    protected:
        Noesis::Delegate<LocationChangedT> mLocationChanged;
        Noesis::Delegate<SizeChangedT> mSizeChanged;
        Noesis::Delegate<StateChangedT> mStateChanged;
        Noesis::Delegate<FileDroppedT> mFileDropped;
        Noesis::Delegate<ActivatedT> mActivated;
        Noesis::Delegate<DeactivatedT> mDeactivated;
        Noesis::Delegate<RenderT> mRender;
        Noesis::Delegate<MouseMoveT> mMouseMove;
        Noesis::Delegate<MouseButtonDownT> mMouseButtonDown;
        Noesis::Delegate<MouseButtonUpT> mMouseButtonUp;
        Noesis::Delegate<MouseDoubleClickT> mMouseDoubleClick;
        Noesis::Delegate<MouseWheelT> mMouseWheel;
        Noesis::Delegate<MouseHWheelT> mMouseHWheel;
        Noesis::Delegate<ScrollT> mScroll;
        Noesis::Delegate<HScrollT> mHScroll;
        Noesis::Delegate<KeyDownT> mKeyDown;
        Noesis::Delegate<KeyUpT> mKeyUp;
        Noesis::Delegate<CharT> mChar;
        Noesis::Delegate<TouchDownT> mTouchDown;
        Noesis::Delegate<TouchMoveT> mTouchMove;
        Noesis::Delegate<TouchUpT> mTouchUp;
        Noesis::Delegate<ClosingT> mClosing;
        Noesis::Delegate<ClosedT> mClosed;

    private:
        irr::Ptr<irr::IrrlichtDevice> mContextDevice;

        NS_DECLARE_REFLECTION(IrrNsDeviceStub, BaseComponent)

};

    NS_WARNING_POP

    /// Creates a ISystemWindow choosing the best implementation available
    NS_IRR_NOESIS_API Noesis::Ptr<IrrNsDeviceStub> CreateDisplay(irr::IrrlichtDevice*);

}

NS_DECLARE_REFLECTION_ENUM_EXPORT(NS_IRR_NOESIS_API, NoesisApp::ResizeMode)
NS_DECLARE_REFLECTION_ENUM_EXPORT(NS_IRR_NOESIS_API, NoesisApp::WindowState)
NS_DECLARE_REFLECTION_ENUM_EXPORT(NS_IRR_NOESIS_API, NoesisApp::WindowStyle)
NS_DECLARE_REFLECTION_ENUM_EXPORT(NS_IRR_NOESIS_API, NoesisApp::WindowStartupLocation)


#endif
