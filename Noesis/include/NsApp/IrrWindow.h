#ifndef __APP_IRRWINDOW_H__
#define __APP_IRRWINDOW_H__

#include <NsApp/IrrNoesis.h>
#include <NsCore/Noesis.h>
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

	class NS_IRR_NOESIS_API IrrWindow : public Noesis::ContentControl
	{
	private:

		enum EGUI_ACTION_MODE_FLAGS
		{
			// moving the selected element
			EAMF_MOVE		= 0x00000001,   
			// resizing the selected element
			EAMF_RESIZE_TL	= 0x00000002,
			EAMF_RESIZE_T	= 0x00000004,
			EAMF_RESIZE_TR	= 0x00000008,
			EAMF_RESIZE_R	= 0x00000010,
			EAMF_RESIZE_BR	= 0x00000020,
			EAMF_RESIZE_B	= 0x00000040,
			EAMF_RESIZE_BL	= 0x00000080,
			EAMF_RESIZE_L	= 0x00000100,

			// can resize the selected element
			EAMF_CAN_RESIZE_TL	= 0x00000200,
			EAMF_CAN_RESIZE_T	= 0x00000400,
			EAMF_CAN_RESIZE_TR	= 0x00000800,
			EAMF_CAN_RESIZE_R	= 0x00001000,
			EAMF_CAN_RESIZE_BR	= 0x00002000,
			EAMF_CAN_RESIZE_B	= 0x00004000,
			EAMF_CAN_RESIZE_BL	= 0x00008000,
			EAMF_CAN_RESIZE_L	= 0x00010000,

			EAMF_RESIZE_ALL		= 0x000001FE,
			EAMF_CAN_RESIZE_ALL = 0x0001FE00,
			EAMF_ALL_CONTROL	= 0x000001FF
		};

	public:
		IrrWindow();
		virtual ~IrrWindow();

		bool ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler);
		void InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&);

		/// Gets or sets the resize mode
		//@{
		ResizeMode GetResizeMode() const;
		void SetResizeMode(ResizeMode value);
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

		/// Gets or sets the position of the window's top edge, in relation to the desktop
		//@{
		float GetTop() const;
		void SetTop(float value);
		//@}

		/// Gets or sets a value that indicates whether window is restored, minimized, or maximized
		//@{
		WindowState GetWindowState() const;
		void SetWindowState(WindowState value);
		//@}

		/// <summary>
		/// Closes the window.
		/// </summary>
		virtual void Close();

		/// <summary>
		/// Hides the window but does not close it.
		/// </summary>
        virtual void Hide();

		/// <summary>
		/// Shows the window.
		/// </summary>
        virtual void Show();

		void DragMove();

        void BringToFront();

		virtual void OnGotFocus(const Noesis::RoutedEventArgs& e) override;
        virtual void OnLostFocus(const Noesis::RoutedEventArgs& e) override;
		virtual void OnMouseLeftButtonDown(const Noesis::MouseButtonEventArgs& e) override;
		virtual void OnMouseLeftButtonUp(const Noesis::MouseButtonEventArgs& e) override;
		virtual void OnMouseMove(const Noesis::MouseEventArgs& e) override;

		void OnTitleMouseUp(BaseComponent* sender, const Noesis::MouseButtonEventArgs& arg);
		void OnTitleMouseDown(BaseComponent* sender, const Noesis::MouseButtonEventArgs& arg);
		void OnTitleMouseLeave(BaseComponent* sender, const Noesis::MouseEventArgs& arg);
		void OnParentMouseMove(BaseComponent* sender, const Noesis::MouseEventArgs& arg);
		void OnClickClose(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
		void OnClickMinimize(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
		void OnClickMaximize(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);

	public:

		static const Noesis::DependencyProperty* ResizeModeProperty;
		static const Noesis::DependencyProperty* IsActiveProperty;
		static const Noesis::DependencyProperty* TitleProperty;
		static const Noesis::DependencyProperty* LeftProperty;
		static const Noesis::DependencyProperty* TopProperty;
		static const Noesis::DependencyProperty* WindowStateProperty;

	private:

        bool ChangeCanResizeState(uint32_t flag);

		bool HasActionModeFlags(uint32_t flag) const
		{
			return (mActionModeFlags & flag) != 0;
		}

	private:

        Noesis::Vector2<float> mOriginalLastPosition;
        Noesis::Vector2<float> mOriginalDimension;
		Noesis::Vector2<float> mLastPointerPosition;
		uint32_t mActionModeFlags;
        bool mIsMaximized;
        bool mOnDragMove;
		bool mOnResize;

		NS_DECLARE_REFLECTION(IrrWindow, ContentControl)
	};
}

#endif