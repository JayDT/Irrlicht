#include <NsApp/IrrWindow.h>
#include <NsApp/Window.h>
#include <NsGui/UIElementData.h>
#include <NsGui/Canvas.h>
#include <NsGui/Brushes.h>
#include <NsGui/Path.h>
#include <NsGui/TextBlock.h>
#include <NsGui/MeshGeometry.h>
#include <NsGui/IView.h>
#include <NsGui/IRenderer.h>
#include <NsGui/Brushes.h>
#include <NsGui/SolidColorBrush.h>
#include <NsGui/FrameworkPropertyMetadata.h>
#include <NsGui/ResourceKeyType.h>
#include <NsGui/ResourceKeyString.h>
#include <NsGui/ResourceDictionary.h>
#include <NsGui/Keyboard.h>
#include <NsGui/KeyboardNavigation.h>
#include <NsGui/SizeChangedInfo.h>
#include <NsGui/Popup.h>
#include <NsGui/ToggleButton.h>
#include <NsGui/IntegrationAPI.h>
#include <NsGui/VisualTreeHelper.h>
#include <NsGui/Storyboard.h>
#include <NsCore/Kernel.h>
#include <NsCore/TypeId.h>
#include <NsCore/Nullable.h>
#include <NsCore/Memory.h>
#include <NsCore/ReflectionImplement.h>
#include <NsImpl/IrrRenderContext.h>
#include <NsDrawing/Color.h>
#include <NsApp/NsUtility.h>
#include "RenderEngines/General/CIrrShader.h"

using namespace Noesis;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Application, IrrWindow)
{
	NS_REGISTER_COMPONENT(NoesisApp::IrrWindow)
}

NoesisApp::IrrWindow::IrrWindow()
	: mActionModeFlags(0)
    , mIsMaximized(false)
	, mOnDragMove(false)
	, mOnResize(false)
{
	Initialized() += Noesis::MakeDelegate(this, &IrrWindow::InitializeComponent);
}

NoesisApp::IrrWindow::~IrrWindow()
{
	if (GetParent())
		GetParent()->MouseMove() -= Noesis::MakeDelegate(this, &IrrWindow::OnParentMouseMove);
}

bool NoesisApp::IrrWindow::ConnectEvent(Noesis::BaseComponent* source, const char* event, const char* handler)
{
	//NS_REGISTER_UI_EVENT(Noesis::Button, Click, OnLoadWoWClientContent)
	return true;
}

void NoesisApp::IrrWindow::InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
	Noesis::Control* TitleBar = GetTemplateChild<Noesis::Control>("PART_TITLEBAR");
	if (TitleBar)
	{
		TitleBar->MouseLeftButtonUp() += Noesis::MakeDelegate(this, &IrrWindow::OnTitleMouseUp);
		TitleBar->MouseLeftButtonDown() += Noesis::MakeDelegate(this, &IrrWindow::OnTitleMouseDown);
		TitleBar->MouseLeave() += Noesis::MakeDelegate(this, &IrrWindow::OnTitleMouseLeave);
	}
	Noesis::Button* button = GetTemplateChild<Noesis::Button>("PART_CLOSE");
	if (button)
	{
		button->Click() += Noesis::MakeDelegate(this, &IrrWindow::OnClickClose);
	}
	button = GetTemplateChild<Noesis::Button>("PART_MAXIMIZE_RESTORE");
	if (button)
	{
		button->Click() += Noesis::MakeDelegate(this, &IrrWindow::OnClickMaximize);
	}
	button = GetTemplateChild<Noesis::Button>("PART_MINIMIZE");
	if (button)
	{
		button->Click() += Noesis::MakeDelegate(this, &IrrWindow::OnClickMinimize);
	}
}

NoesisApp::ResizeMode NoesisApp::IrrWindow::GetResizeMode() const
{
	return GetValue<NoesisApp::ResizeMode>(ResizeModeProperty);
}

void NoesisApp::IrrWindow::SetResizeMode(ResizeMode value)
{
	SetValue<NoesisApp::ResizeMode>(ResizeModeProperty, value);
}

float NoesisApp::IrrWindow::GetLeft() const
{
	return GetValue<float>(LeftProperty);
}

void NoesisApp::IrrWindow::SetLeft(float value)
{
	SetValue<float>(LeftProperty, value);
}

float NoesisApp::IrrWindow::GetTop() const
{
	return GetValue<float>(TopProperty);
}

void NoesisApp::IrrWindow::SetTop(float value)
{
	SetValue<float>(TopProperty, value);
}

bool NoesisApp::IrrWindow::GetIsActive() const
{
	return GetValue<bool>(IsActiveProperty);
}

void NoesisApp::IrrWindow::SetIsActive(bool value)
{
	SetValue<bool>(IsActiveProperty, value);
}

NoesisApp::WindowState NoesisApp::IrrWindow::GetWindowState() const
{
	return GetValue<WindowState>(WindowStateProperty);
}

void NoesisApp::IrrWindow::SetWindowState(NoesisApp::WindowState value)
{
	SetValue<WindowState>(WindowStateProperty, value);
}

void NoesisApp::IrrWindow::Close()
{
    if (GetParent())
    {
        Panel* panel = Noesis::DynamicCast<Panel*>(GetParent());
        if (panel)
        {
            panel->UnregisterName(GetName());
        }
        NoesisApp::RemoveChildHelper::RemoveChild(GetParent(), this);
    }
}

void NoesisApp::IrrWindow::Hide()
{
	SetVisibility(Noesis::Visibility::Visibility_Hidden);
}

void NoesisApp::IrrWindow::Show()
{
	SetVisibility(Noesis::Visibility::Visibility_Visible);
}

void NoesisApp::IrrWindow::DragMove()
{

}

void NoesisApp::IrrWindow::BringToFront()
{
    if (Panel::GetZIndex(this) != 0x1000000)
    {
        //Focus();
        Panel* panel = Noesis::DynamicCast<Panel*>(GetParent());
        if (panel)
        {
            uint32_t count = panel->GetChildren()->Count();
            for (uint32_t i = 0; i != count; ++i)
            {
                Noesis::DependencyObject* element = panel->GetChildren()->Get(i);
                if (element == this)
                    continue;

                uint32_t index = Panel::GetZIndex(element);
                if (index == 0x1000000)
                    Panel::SetZIndex(element, 5);
            }

            Panel::SetZIndex(this, 0x1000000);
        }
    }
}

void NoesisApp::IrrWindow::OnGotFocus(const Noesis::RoutedEventArgs& e)
{
    Noesis::ContentControl::OnGotFocus(e);
}

void NoesisApp::IrrWindow::OnLostFocus(const Noesis::RoutedEventArgs& e)
{
    Noesis::ContentControl::OnLostFocus(e);
}

void NoesisApp::IrrWindow::OnMouseLeftButtonDown(const Noesis::MouseButtonEventArgs& e)
{
	Noesis::ContentControl::OnMouseLeftButtonDown(e);

	if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL))
	{
		mOnDragMove = false;
		mOnResize = true;

		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_T) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TR) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TL))
		{
			mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_T;
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_B) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BR) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BL))
		{
			mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_B;
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_L) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TL) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BL))
		{
			mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_L;
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_R) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TR) || HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BR))
		{
			mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_R;
		}

		CaptureMouse();
		e.handled = true;
	}
}

void NoesisApp::IrrWindow::OnMouseLeftButtonUp(const Noesis::MouseButtonEventArgs& e)
{
	Noesis::ContentControl::OnMouseLeftButtonUp(e);

	if (mOnDragMove || mOnResize)
	{
		mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_ALL;
		mOnDragMove = false;
		mOnResize = false;

		ReleaseMouseCapture();
	}

    BringToFront();
}

void NoesisApp::IrrWindow::OnMouseMove(const Noesis::MouseEventArgs& e)
{
	Noesis::ContentControl::OnMouseMove(e);

	if (mOnDragMove)
	{
		Noesis::Vector2<float> delta = (e.position - mLastPointerPosition);

		auto pos = GetMargin();
		pos.left += delta.x;
		pos.top += delta.y;
		SetMargin(pos);
		mLastPointerPosition = e.position;

		e.handled = true;
	}
	else if (mOnResize)
	{
		auto pos = GetMargin();

		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_T))
		{
			SetHeight(GetHeight() + (pos.top - e.position.y));
			pos.top = e.position.y;
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_B))
		{
			SetHeight(e.position.y - pos.top);
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_L))
		{
			SetWidth(GetWidth() + (pos.left - e.position.x));
			pos.left = e.position.x;
		}
		if (HasActionModeFlags(EGUI_ACTION_MODE_FLAGS::EAMF_RESIZE_R))
		{
			SetWidth(e.position.x - pos.left);
		}

		SetMargin(pos);
		InvalidateMeasure();
	}
	else if (GetWindowState() != WindowState::WindowState_Minimized && GetResizeMode() == ResizeMode::ResizeMode_CanResize)
	{
		auto pos = GetMargin();
		float _left = pos.left;
		float _top = pos.top;
		float top = _top - e.position.y;
		float bottom = _top + GetHeight() - e.position.y;
		float left = _left - e.position.x;
		float right = _left + GetWidth() - e.position.x;
		float maxdelta = 4;

		if (top < maxdelta && top > -maxdelta && right < maxdelta && right > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TR))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNESW);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TR;
			}
		}
		else if (top < maxdelta && top > -maxdelta && left < maxdelta && left > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TL))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNWSE);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_TL;
			}
		}
		else if (bottom < maxdelta && bottom > -maxdelta && right < maxdelta && right > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BR))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNWSE);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BR;
			}
		}
		else if (bottom < maxdelta && bottom > -maxdelta && left < maxdelta && left > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BL))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNESW);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_BL;
			}
		}
		else if (top < maxdelta && top > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_T))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNS);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
				mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_T;
			}
		}
		else if (bottom < maxdelta && bottom > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_B))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeNS);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_B;
			}
		}
		else if (left < maxdelta && left > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_L))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeWE);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_L;
			}
		}
		else if (right < maxdelta && right > -maxdelta)
		{
			if (ChangeCanResizeState(EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_R))
			{
				SetCursor(Noesis::Cursor::Cursor_SizeWE);
                mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
                mActionModeFlags |= EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_R;
			}
		}
		else if (mActionModeFlags & EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL)
		{
			SetCursor(Noesis::Cursor::Cursor_None);
			mActionModeFlags &= ~EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL;
		}
	}
}

void NoesisApp::IrrWindow::OnTitleMouseUp(BaseComponent* sender, const Noesis::MouseButtonEventArgs& arg)
{
	if (mOnDragMove)
	{
		Noesis::Control* control = Noesis::DynamicCast<Noesis::Control*>(sender);
		mOnDragMove = false;
		control->ReleaseMouseCapture();
		arg.handled = true;
	}
}

void NoesisApp::IrrWindow::OnTitleMouseDown(BaseComponent* sender, const Noesis::MouseButtonEventArgs& arg)
{
	if (!mOnDragMove)
	{
		Noesis::Control* control = Noesis::DynamicCast<Noesis::Control*>(sender);
		mOnDragMove = true;
		mLastPointerPosition = arg.position;
		control->CaptureMouse();
		arg.handled = true;
	}
}

void NoesisApp::IrrWindow::OnTitleMouseLeave(BaseComponent* sender, const Noesis::MouseEventArgs& arg)
{
	if (mOnDragMove)
	{
		Noesis::Control* control = Noesis::DynamicCast<Noesis::Control*>(sender);
		mOnDragMove = false;
		control->ReleaseMouseCapture();
		arg.handled = true;
	}
}

void NoesisApp::IrrWindow::OnParentMouseMove(BaseComponent* sender, const Noesis::MouseEventArgs& arg)
{
}

void NoesisApp::IrrWindow::OnClickClose(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    Close();
}

void NoesisApp::IrrWindow::OnClickMinimize(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    auto pos = GetMargin();
    Noesis::FrameworkElement* root = this;
    while (root->GetParent())
        root = root->GetParent();

    if (GetWindowState() != NoesisApp::WindowState::WindowState_Minimized)
    {
        Noesis::Control* TitleBar = GetTemplateChild<Noesis::Control>("PART_TITLEBAR");

        if (GetWindowState() == NoesisApp::WindowState::WindowState_Normal)
        {
            mOriginalLastPosition.x = pos.left;
            mOriginalLastPosition.y = pos.top;
            mOriginalDimension.x = GetWidth();
            mOriginalDimension.y = GetHeight();
        }

        SetWindowState(NoesisApp::WindowState::WindowState_Minimized);

        //pos.left = 0;
        //pos.top = 0;

        //SetMargin(pos);
        //SetWidth(-1);
        SetHeight(TitleBar ? TitleBar->GetHeight() : 40);
    }
    else
    {
        if (mIsMaximized)
        {
            SetWindowState(NoesisApp::WindowState::WindowState_Maximized);

            pos.left = 0;
            pos.top = 0;

            SetMargin(pos); 
            SetWidth(root->GetWidth());
            SetHeight(root->GetHeight());
        }
        else
        {
            SetWindowState(NoesisApp::WindowState::WindowState_Normal);

            //pos.left = mOriginalLastPosition.x;
            //pos.top = mOriginalLastPosition.y;

            //SetMargin(pos);
            SetWidth(mOriginalDimension.x);
            SetHeight(mOriginalDimension.y);
        }
    }
}

void NoesisApp::IrrWindow::OnClickMaximize(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    auto pos = GetMargin();
    Noesis::FrameworkElement* root = this;
    while (root->GetParent())
        root = root->GetParent();

    if (GetWindowState() != NoesisApp::WindowState::WindowState_Maximized)
    {
        if (GetWindowState() == NoesisApp::WindowState::WindowState_Normal)
        {
            mOriginalLastPosition.x = pos.left;
            mOriginalLastPosition.y = pos.top;
            mOriginalDimension.x = GetWidth();
            mOriginalDimension.y = GetHeight();
        }

        SetWindowState(NoesisApp::WindowState::WindowState_Maximized);
        mIsMaximized = true;

        pos.left = 0;
        pos.top = 0;

        SetMargin(pos);
        SetWidth(root->GetWidth());
        SetHeight(root->GetHeight());
    }
    else
    {
        mIsMaximized = false;

        SetWindowState(NoesisApp::WindowState::WindowState_Normal);

        pos.left = mOriginalLastPosition.x;
        pos.top = mOriginalLastPosition.y;

        SetMargin(pos);
        SetWidth(mOriginalDimension.x);
        SetHeight(mOriginalDimension.y);
    }
}

bool NoesisApp::IrrWindow::ChangeCanResizeState(uint32_t flag)
{
    if ((~mActionModeFlags & flag) == flag)
    {
        uint32_t remainingflags = mActionModeFlags & EGUI_ACTION_MODE_FLAGS::EAMF_CAN_RESIZE_ALL & ~flag;
        if (remainingflags)
            mActionModeFlags &= ~remainingflags;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(NoesisApp::IrrWindow)
{
	NsMeta<TypeId>("IrrWindow");

	const TypeClass* type = TypeOf<SelfClass>();
	Ptr<ResourceKeyType> defaultStyleKey = ResourceKeyType::Create(type);

	// register properties and events
	UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());

	data->RegisterProperty<bool>(IsActiveProperty, "IsActive",
		FrameworkPropertyMetadata::Create(false));
	data->AddOwner<float>(LeftProperty, "Left", Canvas::LeftProperty);
	data->RegisterProperty<ResizeMode>(ResizeModeProperty, "ResizeMode",
		FrameworkPropertyMetadata::Create(ResizeMode_CanResize));
	data->RegisterProperty<NsString>(TitleProperty, "Title",
		FrameworkPropertyMetadata::Create(NsString()));
	data->AddOwner<float>(TopProperty, "Top", Canvas::TopProperty);
	data->RegisterProperty<WindowState>(WindowStateProperty, "WindowState",
		FrameworkPropertyMetadata::Create(WindowState_Normal));

	data->OverrideMetadata<bool>(Control::IsTabStopProperty, "IsTabStop",
		FrameworkPropertyMetadata::Create(false));
	data->OverrideMetadata<KeyboardNavigationMode>(KeyboardNavigation::DirectionalNavigationProperty,
		"DirectionalNavigation", FrameworkPropertyMetadata::Create(KeyboardNavigationMode_Cycle));
	data->OverrideMetadata<KeyboardNavigationMode>(KeyboardNavigation::TabNavigationProperty,
		"TabNavigation", FrameworkPropertyMetadata::Create(KeyboardNavigationMode_Cycle));
	data->OverrideMetadata<KeyboardNavigationMode>(KeyboardNavigation::ControlTabNavigationProperty,
		"ControlTabNavigation", FrameworkPropertyMetadata::Create(KeyboardNavigationMode_Cycle));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const Noesis::DependencyProperty* NoesisApp::IrrWindow::LeftProperty;
const Noesis::DependencyProperty* NoesisApp::IrrWindow::TopProperty;
const Noesis::DependencyProperty* NoesisApp::IrrWindow::ResizeModeProperty;
const Noesis::DependencyProperty* NoesisApp::IrrWindow::IsActiveProperty;
const Noesis::DependencyProperty* NoesisApp::IrrWindow::TitleProperty;
const Noesis::DependencyProperty* NoesisApp::IrrWindow::WindowStateProperty;
