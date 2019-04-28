////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsImpl/IrrOsDeviceContext.h>
#include <NsCore/TypeId.h>
#include <NsCore/Kernel.h>
#include <NsCore/ComponentFactory.h>
#include <NsCore/NsFactory.h>
#include <NsCore/ReflectionImplementEnum.h>
#include <NsCore/ReflectionImplementEmpty.h>
#include <NsCore/Package.h>
#include <NsCore/UTF8.h>
#include <NsCore/Category.h>

#include "include/irrlicht.h"
#include "CIrrDeviceStub.h"

using namespace Noesis;
using namespace NoesisApp;


////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Render, IrrRenderDevice)
{
	NS_REGISTER_COMPONENT(IrrNsDeviceStub)
}

namespace
{
    void* gPrivateData;

    static irr::gui::ECURSOR_ICON s_CursorNsToIrrEnum[] =
    {
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_None = 0,
        irr::gui::ECURSOR_ICON::ECI_NO, // Cursor_No = 1,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_Arrow = 2,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_AppStarting = 
        irr::gui::ECURSOR_ICON::ECI_CROSS, // Cursor_Cross = 4,
        irr::gui::ECURSOR_ICON::ECI_HELP, // Cursor_Help = 5,
        irr::gui::ECURSOR_ICON::ECI_IBEAM, // Cursor_IBeam = 6,
        irr::gui::ECURSOR_ICON::ECI_SIZEALL, // Cursor_SizeAll = 7,
        irr::gui::ECURSOR_ICON::ECI_SIZENESW, // Cursor_SizeNESW = 8,
        irr::gui::ECURSOR_ICON::ECI_SIZENS, // Cursor_SizeNS = 9,
        irr::gui::ECURSOR_ICON::ECI_SIZENWSE, // Cursor_SizeNWSE = 10,
        irr::gui::ECURSOR_ICON::ECI_SIZEWE, // Cursor_SizeWE = 11,
        irr::gui::ECURSOR_ICON::ECI_UP, // Cursor_UpArrow = 12,
        irr::gui::ECURSOR_ICON::ECI_WAIT, // Cursor_Wait = 13,
        irr::gui::ECURSOR_ICON::ECI_HAND, // Cursor_Hand = 14,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_Pen = 15,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollNS = 16,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollWE = 17,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollAll = 18
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollN = 19,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollS = 20,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollW = 21,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollE = 22,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollNW = 23,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollNE = 24,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollSW = 25,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ScrollSE = 26,
        irr::gui::ECURSOR_ICON::ECI_NORMAL, // Cursor_ArrowCD = 27
        irr::gui::ECURSOR_ICON::ECI_NORMAL, //
    };

    static Noesis::Key s_OISKeyMappings[] =
    {
        Key_None,  // 0, Key_None
        Key_None,  // 1, KEY_LBUTTON
        Key_None,  // 2, KEY_RBUTTON
        Key_Cancel,  // 3, KEY_CANCEL
        Key_None,  // 4, KEY_MBUTTON
        Key_None,  // 5, KEY_XBUTTON1
        Key_None,  // 6, KEY_XBUTTON2
        Key_None,  // 7, Key_None
        Key_Back,  // 8, KEY_BACK
        Key_Tab,  // 9, KEY_TAB
        Key_None,  // 10, Key_None
        Key_None,  // 11, Key_None
        Key_Clear,  // 12, KEY_CLEAR
        Key_Return,  // 13, KEY_RETURN
        Key_None,  // 14, Key_None
        Key_None,  // 15, Key_None
        Key_None,  // 16, KEY_SHIFT
        Key_None,  // 17, KEY_CONTROL
        Key_GamepadMenu,  // 18, KEY_MENU
        Key_Pause,  // 19, KEY_PAUSE
        Key_Capital,  // 20, KEY_CAPITAL
        Key_KanaMode,  // 21, KEY_KANA
        Key_None,  // 22, Key_None
        Key_JunjaMode,  // 23, KEY_JUNJA
        Key_FinalMode,  // 24, KEY_FINAL
        Key_HanjaMode,  // 25, KEY_HANJA
        Key_None,  // 26, Key_None
        Key_Escape,  // 27, KEY_ESCAPE
        Key_ImeConvert,  // 28, KEY_CONVERT
        Key_ImeNonConvert,  // 29, KEY_NONCONVERT
        Key_GamepadAccept,  // 30, KEY_ACCEPT
        Key_GamepadCancel,  // 31, KEY_MODECHANGE
        Key_Space,  // 32, KEY_SPACE
        Key_Prior,  // 33, KEY_PRIOR
        Key_Next,  // 34, KEY_NEXT
        Key_End,  // 35, KEY_END
        Key_Home,  // 36, KEY_HOME
        Key_Left,  // 37, KEY_LEFT
        Key_Up,  // 38, KEY_UP
        Key_Right,  // 39, KEY_RIGHT
        Key_Down,  // 40, KEY_DOWN
        Key_Select,  // 41, KEY_SELECT
        Key_Print,  // 42, KEY_PRINT
        Key_Execute,  // 43, KEY_EXECUT
        Key_Snapshot,  // 44, KEY_SNAPSHOT
        Key_Insert,  // 45, KEY_INSERT
        Key_Delete,  // 46, KEY_DELETE
        Key_Help,  // 47, KEY_HELP
        Key_D0,  // 48, KEY_KEY_0
        Key_D1,  // 49, KEY_KEY_1
        Key_D2,  // 50, KEY_KEY_2
        Key_D3,  // 51, KEY_KEY_3
        Key_D4,  // 52, KEY_KEY_4
        Key_D5,  // 53, KEY_KEY_5
        Key_D6,  // 54, KEY_KEY_6
        Key_D7,  // 55, KEY_KEY_7
        Key_D8,  // 56, KEY_KEY_8
        Key_D9,  // 57, KEY_KEY_9
        Key_None,  // 58, Key_None
        Key_None,  // 59, Key_None
        Key_None,  // 60, Key_None
        Key_None,  // 61, Key_None
        Key_None,  // 62, Key_None
        Key_None,  // 63, Key_None
        Key_None,  // 64, Key_None
        Key_A,  // 65, KEY_KEY_A
        Key_B,  // 66, KEY_KEY_B
        Key_C,  // 67, KEY_KEY_C
        Key_D,  // 68, KEY_KEY_D
        Key_E,  // 69, KEY_KEY_E
        Key_F,  // 70, KEY_KEY_F
        Key_G,  // 71, KEY_KEY_G
        Key_H,  // 72, KEY_KEY_H
        Key_I,  // 73, KEY_KEY_I
        Key_J,  // 74, KEY_KEY_J
        Key_K,  // 75, KEY_KEY_K
        Key_L,  // 76, KEY_KEY_L
        Key_M,  // 77, KEY_KEY_M
        Key_N,  // 78, KEY_KEY_N
        Key_O,  // 79, KEY_KEY_O
        Key_P,  // 80, KEY_KEY_P
        Key_Q,  // 81, KEY_KEY_Q
        Key_R,  // 82, KEY_KEY_R
        Key_S,  // 83, KEY_KEY_S
        Key_T,  // 84, KEY_KEY_T
        Key_U,  // 85, KEY_KEY_U
        Key_V,  // 86, KEY_KEY_V
        Key_W,  // 87, KEY_KEY_W
        Key_X,  // 88, KEY_KEY_X
        Key_Y,  // 89, KEY_KEY_Y
        Key_Z,  // 90, KEY_KEY_Z
        Key_LWin,  // 91, KEY_LWIN
        Key_RWin,  // 92, KEY_RWIN
        Key_Apps,  // 93, KEY_APPS
        Key_None,  // 94, Key_None
        Key_Sleep,  // 95, KEY_SLEEP
        Key_NumPad0,  // 96, KEY_NUMPAD0
        Key_NumPad1,  // 97, KEY_NUMPAD1
        Key_NumPad2,  // 98, KEY_NUMPAD2
        Key_NumPad3,  // 99, KEY_NUMPAD3
        Key_NumPad4,  // 100, KEY_NUMPAD4
        Key_NumPad5,  // 101, KEY_NUMPAD5
        Key_NumPad6,  // 102, KEY_NUMPAD6
        Key_NumPad7,  // 103, KEY_NUMPAD7
        Key_NumPad8,  // 104, KEY_NUMPAD8
        Key_NumPad9,  // 105, KEY_NUMPAD9
        Key_Multiply,  // 106, KEY_MULTIPLY
        Key_Add,  // 107, KEY_ADD
        Key_Separator,  // 108, KEY_SEPARATOR
        Key_Subtract,  // 109, KEY_SUBTRACT
        Key_Decimal,  // 110, KEY_DECIMAL
        Key_Divide,  // 111, KEY_DIVIDE
        Key_F1,  // 112, KEY_F1
        Key_F2,  // 113, KEY_F2
        Key_F3,  // 114, KEY_F3
        Key_F4,  // 115, KEY_F4
        Key_F5,  // 116, KEY_F5
        Key_F6,  // 117, KEY_F6
        Key_F7,  // 118, KEY_F7
        Key_F8,  // 119, KEY_F8
        Key_F9,  // 120, KEY_F9
        Key_F10,  // 121, KEY_F10
        Key_F11,  // 122, KEY_F11
        Key_F12,  // 123, KEY_F12
        Key_F13,  // 124, KEY_F13
        Key_F14,  // 125, KEY_F14
        Key_F15,  // 126, KEY_F15
        Key_F16,  // 127, KEY_F16
        Key_F17,  // 128, KEY_F17
        Key_F18,  // 129, KEY_F18
        Key_F19,  // 130, KEY_F19
        Key_F20,  // 131, KEY_F20
        Key_F21,  // 132, KEY_F21
        Key_F22,  // 133, KEY_F22
        Key_F23,  // 134, KEY_F23
        Key_F24,  // 135, KEY_F24
        Key_None,  // 136, Key_None
        Key_None,  // 137, Key_None
        Key_None,  // 138, Key_None
        Key_None,  // 139, Key_None
        Key_None,  // 140, Key_None
        Key_None,  // 141, Key_None
        Key_None,  // 142, Key_None
        Key_None,  // 143, Key_None
        Key_NumLock,  // 144, KEY_NUMLOCK
        Key_Scroll,  // 145, KEY_SCROLL
        Key_None,  // 146, Key_None
        Key_None,  // 147, Key_None
        Key_None,  // 148, Key_None
        Key_None,  // 149, Key_None
        Key_None,  // 150, Key_None
        Key_None,  // 151, Key_None
        Key_None,  // 152, Key_None
        Key_None,  // 153, Key_None
        Key_None,  // 154, Key_None
        Key_None,  // 155, Key_None
        Key_None,  // 156, Key_None
        Key_None,  // 157, Key_None
        Key_None,  // 158, Key_None
        Key_None,  // 159, Key_None
        Key_LeftShift,  // 160, KEY_LSHIFT
        Key_RightShift,  // 161, KEY_RSHIFT
        Key_LeftCtrl,  // 162, KEY_LCONTROL
        Key_RightCtrl,  // 163, KEY_RCONTROL
        Key_LeftAlt,  // 164, KEY_LMENU
        Key_RightAlt,  // 165, KEY_RMENU
        Key_None,  // 166, Key_None
        Key_None,  // 167, Key_None
        Key_None,  // 168, Key_None
        Key_None,  // 169, Key_None
        Key_None,  // 170, Key_None
        Key_None,  // 171, Key_None
        Key_None,  // 172, Key_None
        Key_None,  // 173, Key_None
        Key_None,  // 174, Key_None
        Key_None,  // 175, Key_None
        Key_None,  // 176, Key_None
        Key_None,  // 177, Key_None
        Key_None,  // 178, Key_None
        Key_None,  // 179, Key_None
        Key_None,  // 180, Key_None
        Key_None,  // 181, Key_None
        Key_None,  // 182, Key_None
        Key_None,  // 183, Key_None
        Key_None,  // 184, Key_None
        Key_None,  // 185, Key_None
        Key_Oem1,  // 186, KEY_OEM_1
        Key_OemPlus,  // 187, KEY_PLUS
        Key_OemComma,  // 188, KEY_COMMA
        Key_OemMinus,  // 189, KEY_MINUS
        Key_OemPeriod,  // 190, KEY_PERIOD
        Key_Oem2,  // 191, KEY_OEM_2
        Key_Oem3,  // 192, KEY_OEM_3
        Key_None,  // 193, Key_None
        Key_None,  // 194, Key_None
        Key_None,  // 195, Key_None
        Key_None,  // 196, Key_None
        Key_None,  // 197, Key_None
        Key_None,  // 198, Key_None
        Key_None,  // 199, Key_None
        Key_None,  // 200, Key_None
        Key_None,  // 201, Key_None
        Key_None,  // 202, Key_None
        Key_None,  // 203, Key_None
        Key_None,  // 204, Key_None
        Key_None,  // 205, Key_None
        Key_None,  // 206, Key_None
        Key_None,  // 207, Key_None
        Key_None,  // 208, Key_None
        Key_None,  // 209, Key_None
        Key_None,  // 210, Key_None
        Key_None,  // 211, Key_None
        Key_None,  // 212, Key_None
        Key_None,  // 213, Key_None
        Key_None,  // 214, Key_None
        Key_None,  // 215, Key_None
        Key_None,  // 216, Key_None
        Key_None,  // 217, Key_None
        Key_None,  // 218, Key_None
        Key_Oem4,  // 219, KEY_OEM_4
        Key_Oem5,  // 220, KEY_OEM_5
        Key_Oem6,  // 221, KEY_OEM_6
        Key_Oem7,  // 222, KEY_OEM_7
        Key_Oem8,  // 223, KEY_OEM_8
        Key_None,  // 224, Key_None
        Key_None,  // 225, KEY_OEM_AX
        Key_Oem102,  // 226, KEY_OEM_102
        Key_None,  // 227, Key_None
        Key_None,  // 228, Key_None
        Key_None,  // 229, Key_None
        Key_None,  // 230, Key_None
        Key_None,  // 231, Key_None
        Key_None,  // 232, Key_None
        Key_None,  // 233, Key_None
        Key_None,  // 234, Key_None
        Key_None,  // 235, Key_None
        Key_None,  // 236, Key_None
        Key_None,  // 237, Key_None
        Key_None,  // 238, Key_None
        Key_None,  // 239, Key_None
        Key_None,  // 240, Key_None
        Key_None,  // 241, Key_None
        Key_None,  // 242, Key_None
        Key_None,  // 243, Key_None
        Key_None,  // 244, Key_None
        Key_None,  // 245, Key_None
        Key_Attn,  // 246, KEY_ATTN
        Key_CrSel,  // 247, KEY_CRSEL
        Key_ExSel,  // 248, KEY_EXSEL
        Key_EraseEof,  // 249, KEY_EREOF
        Key_Play,  // 250, KEY_PLAY
        Key_Zoom,  // 251, KEY_ZOOM
        Key_None,  // 252, Key_None
        Key_Pa1,  // 253, KEY_PA1
        Key_OemClear,  // 254, KEY_OEM_CLEAR
    };

}

NoesisApp::IrrNsDeviceStub::IrrNsDeviceStub()
{
}

NoesisApp::IrrNsDeviceStub::~IrrNsDeviceStub()
{
}

void NoesisApp::IrrNsDeviceStub::SetIrrlichtDevice(irr::IrrlichtDevice* contextDevice)
{
	mContextDevice = irr::Ptr(contextDevice);
    if (mContextDevice)
    {
        mContextDevice->setEventReceiver(static_cast<irr::IEventReceiver*>(this));
        //static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->o
    }
	else
		mContextDevice->setEventReceiver(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetPrivateData(void* data)
{
    gPrivateData = data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* IrrNsDeviceStub::GetPrivateData()
{
    return gPrivateData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetTitle(const char* title)
{
    uint16_t titleU16[256];
    uint32_t numChars = UTF8::UTF8To16(title, titleU16, 256);
    NS_ASSERT(numChars <= 256);

    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->setWindowCaption((wchar_t*)titleU16);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetLocation(int x, int y)
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->setWindowPosition(irr::core::position2di(x, y));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetSize(uint32_t x, uint32_t y)
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->setWindowSize(irr::core::dimension2du(x, y));
}

void NoesisApp::IrrNsDeviceStub::GetSize(uint32_t& width, uint32_t& height)
{
	width = static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->getWindowSize().Width;
	height = static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->getWindowSize().Height;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetWindowStyle(WindowStyle)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetWindowState(WindowState)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetResizeMode(ResizeMode)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetShowInTaskbar(bool)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetTopmost(bool)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetAllowFileDrop(bool)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::SetWindowStartupLocation(WindowStartupLocation)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::Capture()
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->capture();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::ReleaseCapture()
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->uncapture();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::Focus()
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->focus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::Show()
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->show();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IrrNsDeviceStub::Close()
{
    static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->hide();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::OpenSoftwareKeyboard(Noesis::UIElement* focused)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::CloseSoftwareKeyboard()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::SetCursor(Noesis::Cursor cursor)
{
    mContextDevice->getCursorControl()->setActiveIcon(s_CursorNsToIrrEnum[cursor]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::OpenUrl(const char* url)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void* NoesisApp::IrrNsDeviceStub::GetNativeHandle() const
{
    return (void*)mContextDevice->getHandle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t NoesisApp::IrrNsDeviceStub::GetClientWidth() const
{
    return static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->getClientSize().Width;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t NoesisApp::IrrNsDeviceStub::GetClientHeight() const
{
    return static_cast<irr::CIrrDeviceStub*>(mContextDevice.GetPtr())->getClientSize().Height;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::PlaySound(const char* filename, float volume)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::PauseAudio()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void NoesisApp::IrrNsDeviceStub::ResumeAudio()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::LocationChangedT>& IrrNsDeviceStub::LocationChanged()
{
    return mLocationChanged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::SizeChangedT>& IrrNsDeviceStub::SizeChanged()
{
    return mSizeChanged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::StateChangedT>& IrrNsDeviceStub::StateChanged()
{
    return mStateChanged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::FileDroppedT>& IrrNsDeviceStub::FileDropped()
{
    return mFileDropped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::ActivatedT>& IrrNsDeviceStub::Activated()
{
    return mActivated;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::DeactivatedT>& IrrNsDeviceStub::Deactivated()
{
    return mDeactivated;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Delegate<IrrNsDeviceStub::RenderT>& IrrNsDeviceStub::Render()
{
    return mRender;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseMoveT>& IrrNsDeviceStub::MouseMove()
{
    return mMouseMove;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseButtonDownT>& IrrNsDeviceStub::MouseButtonDown()
{
    return mMouseButtonDown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseButtonUpT>& IrrNsDeviceStub::MouseButtonUp()
{
    return mMouseButtonUp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseDoubleClickT>& IrrNsDeviceStub::MouseDoubleClick()
{
    return mMouseDoubleClick;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseWheelT>& IrrNsDeviceStub::MouseWheel()
{
    return mMouseWheel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::MouseWheelT>& IrrNsDeviceStub::MouseHWheel()
{
    return mMouseHWheel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::ScrollT>& IrrNsDeviceStub::Scroll()
{
    return mScroll;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::HScrollT>& IrrNsDeviceStub::HScroll()
{
    return mHScroll;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::KeyDownT>& IrrNsDeviceStub::KeyDown()
{
    return mKeyDown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::KeyUpT>& IrrNsDeviceStub::KeyUp()
{
    return mKeyUp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::CharT>& IrrNsDeviceStub::Char()
{
    return mChar;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::TouchDownT>& IrrNsDeviceStub::TouchDown()
{
    return mTouchDown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::TouchMoveT>& IrrNsDeviceStub::TouchMove()
{
    return mTouchMove;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Delegate<IrrNsDeviceStub::TouchUpT>& IrrNsDeviceStub::TouchUp()
{
    return mTouchUp;
}

Noesis::Delegate<IrrNsDeviceStub::ClosingT>& NoesisApp::IrrNsDeviceStub::Closing()
{
    return mClosing;
}

Noesis::Delegate<IrrNsDeviceStub::ClosedT>& NoesisApp::IrrNsDeviceStub::Closed()
{
    return mClosed;
}

bool NoesisApp::IrrNsDeviceStub::OnEvent(const irr::SEvent& event)
{
    switch (event.EventType)
    {
        case irr::EET_WINDOW_EVENT:
        {
            switch (event.WindowEvent.Event)
            {
                case irr::SEvent::SWindowEvent::EventType::IWE_MOVE:
                {
                    return mLocationChanged(this, event.WindowEvent.FParam0, event.WindowEvent.FParam1);
                }
                case irr::SEvent::SWindowEvent::EventType::IWE_SIZE:
                {
                    switch (event.WindowEvent.Param0)
                    {
                        case irr::SEvent::SWindowEvent::WndState::WNS_RESTORED:
                        {
                            mStateChanged(this, WindowState_Normal);
                            mSizeChanged(this, event.WindowEvent.FParam0, event.WindowEvent.FParam1);
                            break;
                        }
                        case irr::SEvent::SWindowEvent::WndState::WNS_MAXIMIZED:
                        {
                            mStateChanged(this, WindowState_Maximized);
                            mSizeChanged(this, event.WindowEvent.FParam0, event.WindowEvent.FParam1);
                            break;
                        }
                        case irr::SEvent::SWindowEvent::WndState::WNS_MINIMAIZE:
                        {
                            mStateChanged(this, WindowState_Minimized);
                            break;
                        }
                    }
                    break;
                }
                case irr::SEvent::SWindowEvent::EventType::IWE_ACTIVATE:
                {
                    if (event.WindowEvent.Param0 == 1)
                        mDeactivated(this);
                    else
                        mActivated(this);
                    break;
                }
                case irr::SEvent::SWindowEvent::EventType::IWE_DROPFILES:
                {
                    char filename[256];
                    uint32_t numChars = UTF8::UTF16To8((uint16_t*)event.WindowEvent.UnicodeParam, filename, 256);
                    NS_ASSERT(numChars <= 256);

                    return mFileDropped(this, filename);
                }
                case irr::SEvent::SWindowEvent::EventType::IWE_CLOSE:
                {
                    return mClosing(this);
                }
                case irr::SEvent::SWindowEvent::EventType::IWE_DESTROY:
                {
                    return mClosed(this);
                }
            }
            break;
        }
        case irr::EET_TOUCH_INPUT_EVENT:
        {
            switch (event.TouchEvent.States)
            {
                case irr::ETOUCH_INPUT_EVENT::ETIE_TOUCH_DOWN:  return mTouchDown(this, event.TouchEvent.X, event.TouchEvent.Y, event.TouchEvent.Id); break;
                case irr::ETOUCH_INPUT_EVENT::ETIE_NONE:        return mTouchMove(this, event.TouchEvent.X, event.TouchEvent.Y, event.TouchEvent.Id); break;
                case irr::ETOUCH_INPUT_EVENT::ETIE_TOUCH_UP:    return mTouchUp  (this, event.TouchEvent.X, event.TouchEvent.Y, event.TouchEvent.Id); break;
            }
            break;
        }
        case irr::EET_MOUSE_INPUT_EVENT:
        {
            if (event.MouseInput.Event == irr::EMOUSE_INPUT_EVENT::EMIE_MOUSE_WHEEL)
            {
                mMouseWheel(this, event.MouseInput.X, event.MouseInput.Y, event.MouseInput.Wheel * WHEEL_DELTA); // win hack
            }
            else
            {
                switch (event.MouseInput.Event)
                {
                    case irr::EMOUSE_INPUT_EVENT::EMIE_MOUSE_MOVED:         return mMouseMove       (this, event.MouseInput.X, event.MouseInput.Y); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_PRESSED_DOWN: return mMouseButtonDown (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Left); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_LEFT_UP:      return mMouseButtonUp   (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Left); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_PRESSED_DOWN: return mMouseButtonDown (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Right); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_LEFT_UP:      return mMouseButtonUp   (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Right); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_PRESSED_DOWN: return mMouseButtonDown (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Middle); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_LEFT_UP:      return mMouseButtonUp   (this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Middle); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_DOUBLE_CLICK: return mMouseDoubleClick(this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Left); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_DOUBLE_CLICK: return mMouseDoubleClick(this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Right); break;
                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_DOUBLE_CLICK: return mMouseDoubleClick(this, event.MouseInput.X, event.MouseInput.Y, Noesis::MouseButton_Middle); break;
                }
            }
            break;
        }
        case irr::EET_TEXT_INPUT_EVENT:
        {
            return mChar(this, event.KeyInput.Char);
        }
        case irr::EET_KEY_INPUT_EVENT:
        {
            if (event.KeyInput.Key < 0xFF && s_OISKeyMappings[event.KeyInput.Key] != 0)
            {
                if (event.KeyInput.PressedDown)
                    return mKeyDown(this, s_OISKeyMappings[event.KeyInput.Key]);
                return mKeyUp(this, s_OISKeyMappings[event.KeyInput.Key]);
            }
            break;
        }
    }
    return false; // is absorbed
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Ptr<IrrNsDeviceStub> NoesisApp::CreateDisplay(irr::IrrlichtDevice* context)
{
    ComponentFactory::Vector v;
    ComponentFactory* factory = NsGetKernel()->GetComponentFactory();
    factory->EnumComponents(NsSymbol("Display"), v);
    
    if (!v.empty())
    {
        auto display = NsCreateComponent<IrrNsDeviceStub>(v[0]);
		display->SetIrrlichtDevice(context);
		return display;
    }

    NS_FATAL("IrrNsDeviceStub implementation not found");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(IrrNsDeviceStub)
{
	NsMeta<TypeId>("IrrNsDeviceStub");
	NsMeta<Category>("Display");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(ResizeMode)
{
    NsMeta<TypeId>("ResizeMode");

    NsVal("CanMinimize", ResizeMode_CanMinimize);
    NsVal("CanResize", ResizeMode_CanResize);
    NsVal("CanResizeWithGrip", ResizeMode_CanResizeWithGrip);
    NsVal("NoResize", ResizeMode_NoResize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(WindowState)
{
    NsMeta<TypeId>("WindowState");

    NsVal("Maximized", WindowState_Maximized);
    NsVal("Minimized", WindowState_Minimized);
    NsVal("Normal", WindowState_Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(WindowStyle)
{
    NsMeta<TypeId>("WindowStyle");

    NsVal("None", WindowStyle_None);
    NsVal("SingleBorderWindow", WindowStyle_SingleBorderWindow);
    NsVal("ThreeDBorderWindow", WindowStyle_ThreeDBorderWindow);
    NsVal("ToolWindow", WindowStyle_ToolWindow);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION_ENUM(WindowStartupLocation)
{
    NsMeta<TypeId>("WindowStartupLocation");

    NsVal("Manual", WindowStartupLocation_Manual);
    NsVal("CenterScreen", WindowStartupLocation_CenterScreen);
    NsVal("CenterOwner", WindowStartupLocation_CenterOwner);
};
