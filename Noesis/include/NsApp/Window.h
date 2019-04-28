#ifndef __APP_WINDOW_H__
#define __APP_WINDOW_H__

#include <NsApp/IrrNoesis.h>
#include <NsCore/Noesis.h>
#include <NsGui/ContentControl.h>
#include <NsGui/Enums.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsImpl/IrrOsDeviceContext.h>


namespace Noesis
{

    NS_INTERFACE IView;
    class TextBlock;
    class Storyboard;
    class ToggleButton;
    class MeshGeometry;
    class Color;
    struct TimelineEventArgs;
    template<class T> class Delegate;

}

namespace NoesisApp
{
    class IrrNsDeviceStub;
    class IrrRenderContext;

    NS_WARNING_PUSH
    NS_MSVC_WARNING_DISABLE(4251 4275)

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Provides the ability to create, configure, show, and manage the lifetime of windows.
    ///
    /// http://msdn.microsoft.com/en-us/library/system.windows.window.aspx
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class NS_IRR_NOESIS_API Window : public Noesis::ContentControl
    {
    public:
        Window();
        virtual ~Window();

        /// To avoid overloaded virtual function warning
        using Noesis::DependencyObject::Init;

        /// Initializes the window with the given display and render context
        void Init(IrrNsDeviceStub* display, IrrRenderContext* context, uint32_t samples, bool ppaa);

        /// Renders frame at the given time
        void Render(double time);

        /// Frees allocated render resources
        void Shutdown();

        /// Manually closes window
        void Close();

        /// Gets or sets a value that indicates whether a window's client area supports transparency
        //@{
        bool GetAllowsTransparency() const;
        void SetAllowsTransparency(bool allowsTransparency);
        //@}

        /// Gets or sets window fullscreen flag
        //@{
        bool GetFullscreen() const;
        void SetFullscreen(bool fullscreen);
        //@}

        /// Gets a value that indicates whether the window is active
        //@{
        bool GetIsActive() const;
        void SetIsActive(bool value);
        //@}

        /// Gets or sets the position of the window's left edge, in relation to the desktop
        //@{
        float GetLeft() const;
        void SetLeft(float value);
        //@}

        /// Gets or sets the resize mode
        //@{
        ResizeMode GetResizeMode() const;
        void SetResizeMode(ResizeMode value);
        //@}

        /// Gets or sets a value that indicates whether the window has a task bar button
        //@{
        bool GetShowInTaskbar() const;
        void SetShowInTaskbar(bool value);
        //@}

        /// Gets or sets window title
        //@{
        const char* GetTitle() const;
        void SetTitle(const char* value);
        //@}

        /// Gets or sets the position of the window's top edge, in relation to the desktop
        //@{
        float GetTop() const;
        void SetTop(float value);
        //@}

        /// Gets or sets a value that indicates whether window appears in the topmost z-order
        //@{
        bool GetTopmost() const;
        void SetTopmost(bool value);
        //@}

        /// Gets or sets a value that indicates whether window is restored, minimized, or maximized
        //@{
        WindowState GetWindowState() const;
        void SetWindowState(WindowState value);
        //@}

        /// Gets or sets window border style
        //@{
        WindowStyle GetWindowStyle() const;
        void SetWindowStyle(WindowStyle value);
        //@}

        /// Gets or sets the position of the window when first shown
        //@{
        WindowStartupLocation GetWindowStartupLocation() const;
        void SetWindowStartupLocation(WindowStartupLocation location);
        //@}

        /// Occurs when window becomes the foreground window
        Noesis::EventHandler& Activated();

        /// Occurs when window becomes a background window.
        Noesis::EventHandler& Deactivated();

        /// Occurs after window has been closed
        Noesis::EventHandler& Closed();

        /// Occurs when the window's location changes
        Noesis::EventHandler& LocationChanged();

        /// Occurs when the window's WindowState property changes
        Noesis::EventHandler& StateChanged();

    public:
        static const Noesis::DependencyProperty* ClearColorProperty;
        static const Noesis::DependencyProperty* AllowsTransparencyProperty;
        static const Noesis::DependencyProperty* FullscreenProperty;
        static const Noesis::DependencyProperty* IsActiveProperty;
        static const Noesis::DependencyProperty* LeftProperty;
        static const Noesis::DependencyProperty* ResizeModeProperty;
        static const Noesis::DependencyProperty* ShowInTaskbarProperty;
        static const Noesis::DependencyProperty* TitleProperty;
        static const Noesis::DependencyProperty* TopmostProperty;
        static const Noesis::DependencyProperty* TopProperty;
        static const Noesis::DependencyProperty* WindowStateProperty;
        static const Noesis::DependencyProperty* WindowStyleProperty;
        static const Noesis::DependencyProperty* WindowStartupLocationProperty;

		IrrNsDeviceStub* GetDisplayDevice() const { return mDisplay; }
		IrrRenderContext* GetIrrRenderContext() const { return mRenderContext; }

    protected:
        virtual void OnFullscreenChanged(bool fullscreen);
        virtual void OnTitleChanged(const char* title);
        virtual void OnLeftChanged(float left);
        virtual void OnTopChanged(float top);
        virtual void OnWindowStyleChanged(WindowStyle windowStyle);
        virtual void OnStateChanged(WindowState windowState);
        virtual void OnResizeModeChanged(ResizeMode resizeMode);
        virtual void OnShowInTaskbarChanged(bool showInTaskbar);
        virtual void OnTopmostChanged(bool topmost);
        virtual void OnFileDropped(const char* filename);
		virtual void OnQueryCursor(const Noesis::QueryCursorEventArgs& e);

        /// Shows option toolbar for 2 seconds
        void PreviewToolbar();

        /// From DependencyObject
        //@{
        bool OnPropertyChanged(const Noesis::DependencyPropertyChangedEventArgs& args) override;
        //@}

        /// From FrameworkElement
        //@{
        void OnWidthChanged(float width) override;
        void OnHeightChanged(float height) override;
        Noesis::Size MeasureOverride(const Noesis::Size& availableSize) override;
        Noesis::Size ArrangeOverride(const Noesis::Size& finalSize) override;
        //@}

        /// From UIElement
        //@{
        void OnManipulationStarted(const Noesis::ManipulationStartedEventArgs& e) override;
        void OnManipulationInertiaStarting(const Noesis::ManipulationInertiaStartingEventArgs& e) override;
        //@}

    private:
        void HideToolbar(BaseComponent* sender, const Noesis::TimelineEventArgs& args);
        void OnToolbarMouseInteraction(BaseComponent* sender, const Noesis::MouseEventArgs& args);
        void OnToolbarTouchInteraction(BaseComponent* sender, const Noesis::TouchEventArgs& args);
        void OnToolbarKeyInteraction(BaseComponent* sender, const Noesis::KeyEventArgs& args);
        void HitTestToolbar(int x, int y);
        void ShowToolbar(bool visible);

        void EnablePPAA(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void DisablePPAA(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void EnableWireframe(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void DisableWireframe(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void EnableBatches(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void DisableBatches(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void EnableOverdraw(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void DisableOverdraw(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void EnableTitleStats(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void EnableWidgetStats(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void DisableStats(BaseComponent* sender, const Noesis::RoutedEventArgs& arg);
        void SyncViewFlags();
        void LoadOverlayXAML();

        void UpdateHistogram(uint16_t* data, float maxF, Noesis::MeshGeometry* geometry) const;
        void UpdateStats(double time);
        void RenderStats(double time);

        bool OnDisplayLocationChanged(IrrNsDeviceStub* display, int x, int y);
        bool OnDisplaySizeChanged(IrrNsDeviceStub* displa, uint32_t width, uint32_t height);
        bool OnDisplayFileDropped(IrrNsDeviceStub* display, const char* filename);
        bool OnDisplayActivated(IrrNsDeviceStub* display);
        bool OnDisplayDeactivated(IrrNsDeviceStub* display);
        bool OnDisplayMouseMove(IrrNsDeviceStub* display, int x, int y);
        bool OnDisplayMouseButtonDown(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        bool OnDisplayMouseButtonUp(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        bool OnDisplayMouseDoubleClick(IrrNsDeviceStub* display, int x, int y, Noesis::MouseButton b);
        bool OnDisplayMouseWheel(IrrNsDeviceStub* display, int x, int y, int delta);
        bool OnDisplayMouseHWheel(IrrNsDeviceStub* display, int x, int y, int delta);
        bool OnDisplayScroll(IrrNsDeviceStub* display, float value);
        bool OnDisplayHScroll(IrrNsDeviceStub* display, float value);
        bool OnDisplayTouchDown(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        bool OnDisplayTouchMove(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        bool OnDisplayTouchUp(IrrNsDeviceStub* display, int x, int y, uint64_t id);
        bool OnDisplayKeyDown(IrrNsDeviceStub* display, Noesis::Key key);
        bool OnDisplayKeyUp(IrrNsDeviceStub* display, Noesis::Key key);
        bool OnDisplayChar(IrrNsDeviceStub* display, uint32_t c);
        bool OnDisplayClosing(IrrNsDeviceStub* display);
        bool OnDisplayClosed(IrrNsDeviceStub* display);

        static bool CoerceWindowStyle(const DependencyObject* object, const void* in, void* out);
        static bool CoerceResizeMode(const DependencyObject* object, const void* in, void* out);
        static bool CoerceBackground(const DependencyObject* object, const void* in, void* out);

    private:
		Noesis::Ptr<IrrNsDeviceStub> mDisplay;
		Noesis::Ptr<IrrRenderContext> mRenderContext;
        uint32_t mSamples;
        bool mPPAA;
        uint32_t mWindowFlags;
        uint32_t mViewFlags;

        Noesis::IView* mActiveView;
        Noesis::Ptr<Noesis::IView> mView;
        Noesis::Ptr<Noesis::IView> mViewStats;

        bool mIsCtrlDown;
        bool mIsShiftDown;

        enum StatsMode
        {
            StatsMode_Disabled,
            StatsMode_TitleBar,
            StatsMode_Widget
        };

        StatsMode mStatsMode;
        FrameworkElement* mStats;
        FrameworkElement* mToolbar;

        Noesis::ToggleButton* mWireframeBtn;
        Noesis::ToggleButton* mBatchesBtn;
        Noesis::ToggleButton* mOverdrawBtn;
        Noesis::ToggleButton* mPPAABtn;
        Noesis::ToggleButton* mStatsBtn;
        Noesis::Storyboard* mWaitToHideToolbar;

        char mCachedTitle[256];
        Noesis::TextBlock* mTextTitle;
        Noesis::TextBlock* mTextFps;
        Noesis::TextBlock* mTextMs;
        Noesis::TextBlock* mTextUpdate;
        Noesis::TextBlock* mTextRender;
        Noesis::TextBlock* mTextGPU;
        Noesis::TextBlock* mTextPrimitives;
        Noesis::TextBlock* mTextTriangles;
        Noesis::TextBlock* mTextNodes;
        Noesis::TextBlock* mTextUploads;
        Noesis::TextBlock* mTextMemory;
        Noesis::MeshGeometry* mGeoHistCPU;
        Noesis::MeshGeometry* mGeoHistGPU;

        Noesis::Size mHistSize;
        uint16_t mHistogramCPU[64];
        uint16_t mHistogramGPU[64];
        uint32_t mHistPos;

        double mLastRenderTime;
        uint32_t mNumFrames;
        float mUpdateTime;
        float mRenderTime;
        float mFrameTime;
        float mGPUTime;

        Noesis::EventHandler mActivated;
        Noesis::EventHandler mDeactivated;
        Noesis::EventHandler mClosed;
        Noesis::EventHandler mLocationChanged;
        Noesis::EventHandler mStateChanged;

        NS_DECLARE_REFLECTION(Window, ContentControl)
    };

    NS_WARNING_POP

}

#endif
