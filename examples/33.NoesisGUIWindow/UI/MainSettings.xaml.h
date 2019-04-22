#ifndef __C_ROOTWINDOW_H__
#define __C_ROOTWINDOW_H__

#include <NsCore/Noesis.h>
#include <NsApp/Window.h>
#include <NsApp/IrrWindow.h>
#include <NsGui/TextBox.h>
#include <NsGui/ComboBox.h>
#include <NsCore/Vector.h>
#include <NsApp/DispatcherTimer.h>
#include <NsApp/NotifyPropertyChangedBase.h>

namespace irr::scene
{
	class ISceneNode;
}

namespace Noesis
{
	class Button;
	class ComboBox;
	class ListBox;
	class TextBlock;
	class TextBox;
	class ProgressBar;
}

namespace WorldClient
{
    class GlobalSettingEntry : public NoesisApp::NotifyPropertyChangedBase
    {
    public:
        std::string m_wowLocation;
        Noesis::Vector2i m_renderResolution;
        uint16 m_msaa;
        uint8 m_renderEngine;
        uint8 m_colorDepth;
        bool m_vSync;
        bool m_windowMode;

        void SetWowLocation(const char* value)
        {
            if (m_wowLocation != value)
            {
                m_wowLocation = value;
                OnPropertyChanged("WowLocation");
            }
        }

        const char* GetWowLocation() const
        {
            return m_wowLocation.c_str();
        }

        void SetRenderResolution(const Noesis::Vector2i& value)
        {
            if (m_renderResolution != value)
            {
                m_renderResolution = value;
                OnPropertyChanged("RenderResolution");
            }
        }

        const Noesis::Vector2i& GetRenderResolution() const
        {
            return m_renderResolution;
        }

        void SetMsaa(uint16_t value)
        {
            if (m_msaa != value)
            {
                m_msaa = value;
                OnPropertyChanged("Msaa");
            }
        }

        uint16_t GetMsaa() const
        {
            return m_msaa;
        }

        void SetRenderEngine(uint8_t value)
        {
            if (m_renderEngine != value)
            {
                m_renderEngine = value;
                OnPropertyChanged("RenderEngine");
            }
        }

        uint8_t GetRenderEngine() const
        {
            return m_renderEngine;
        }

        void SetColorDepth(uint8_t value)
        {
            if (m_colorDepth != value)
            {
                m_colorDepth = value;
                OnPropertyChanged("ColorDepth");
            }
        }

        uint8_t GetColorDepth() const
        {
            return m_colorDepth;
        }

        void SetVSync(bool value)
        {
            if (m_vSync != value)
            {
                m_vSync = value;
                OnPropertyChanged("VSync");
            }
        }

        bool GetVSync() const
        {
            return m_vSync;
        }

        void SetWindowMode(bool value)
        {
            if (m_windowMode != value)
            {
                m_windowMode = value;
                OnPropertyChanged("WindowMode");
            }
        }

        bool GetWindowMode() const
        {
            return m_windowMode;
        }

    private:

        NS_IMPLEMENT_INLINE_REFLECTION(GlobalSettingEntry, NoesisApp::NotifyPropertyChangedBase)
        {
            NsMeta<Noesis::TypeId>("WorldClient.GlobalSettingEntry");
            NsProp("WowLocation", &SelfClass::GetWowLocation, &SelfClass::SetWowLocation);
            NsProp("RenderResolution", &SelfClass::GetRenderResolution, &SelfClass::SetRenderResolution);
            NsProp("Msaa", &SelfClass::GetMsaa, &SelfClass::SetMsaa);
            NsProp("RenderEngine", &SelfClass::GetRenderEngine, &SelfClass::SetRenderEngine);
            NsProp("ColorDepth", &SelfClass::GetColorDepth, &SelfClass::SetColorDepth);
            NsProp("VSync", &SelfClass::GetVSync, &SelfClass::SetVSync);
            NsProp("WindowMode", &SelfClass::GetWindowMode, &SelfClass::SetWindowMode);
        }
    };

    struct DisplayResolution : public Noesis::BaseComponent
    {
        DisplayResolution(uint16_t width, uint16_t height, uint8_t color)
        {
            m_renderResolution.x = height;
            m_renderResolution.y = width;
            m_colorDepth = color;

            m_string = System::String::format("%u x %u : %u", width, height, color);
        }

        virtual ~DisplayResolution() = default;

        std::string m_string;
        Noesis::Vector2i m_renderResolution;
        uint8 m_colorDepth;

        uint16_t GetHeight() const
        {
            return m_renderResolution.x;
        }

        uint16_t GetWidth() const
        {
            return m_renderResolution.y;
        }

        uint8_t GetColorDepth() const
        {
            return m_colorDepth;
        }

        const char* GetString() const
        {
            return m_string.c_str();
        }

        NS_IMPLEMENT_INLINE_REFLECTION(DisplayResolution, Noesis::BaseComponent)
        {
            NsMeta<Noesis::TypeId>("WorldClient.DisplayResolution");
            NsProp("String", &SelfClass::GetString);
            NsProp("Height", &SelfClass::GetHeight);
            NsProp("Width", &SelfClass::GetWidth);
            NsProp("ColorDepth", &SelfClass::GetColorDepth);
        }
    };

    struct DisplayDriver : public Noesis::BaseComponent
    {
        DisplayDriver(uint8_t driver)
        {
            m_driver = driver;
            if (driver == irr::video::E_DRIVER_TYPE::EDT_DIRECT3D11)
                m_string = "DirectX 11";
            else if (driver == irr::video::E_DRIVER_TYPE::EDT_VULKAN)
                m_string = "Vulkan";
            else if (driver == irr::video::E_DRIVER_TYPE::EDT_OPENGL)
                m_string = "OpenGL";
        }

        virtual ~DisplayDriver() = default;

        std::string m_string;
        uint8_t m_driver;

        uint8_t GetDriverID() const
        {
            return m_driver;
        }

        const char* GetString() const
        {
            return m_string.c_str();
        }

        NS_IMPLEMENT_INLINE_REFLECTION(DisplayDriver, Noesis::BaseComponent)
        {
            NsMeta<Noesis::TypeId>("WorldClient.DisplayDriver");
            NsProp("String", &SelfClass::GetString);
            NsProp("DriverID", &SelfClass::GetDriverID);
        }
    };

	class MainSettings final : public NoesisApp::IrrWindow
	{
        Noesis::Ptr<GlobalSettingEntry> m_dataContext;
        Noesis::Ptr<NoesisApp::Window> m_dialogFileSelect;
	public:

		MainSettings();
		virtual ~MainSettings();

		bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;
		void InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&);

        void OnSelectionChangedResolution(Noesis::BaseComponent*, const Noesis::SelectionChangedEventArgs&);
        void OnSelectionChangedDriver(Noesis::BaseComponent*, const Noesis::SelectionChangedEventArgs&);
        void OnSelectButtonClick(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnTextChangedWoWLocation(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnGotFocusWindow(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);

        void OnDataContextPropertyChanged(Noesis::BaseComponent*, const Noesis::PropertyChangedEventArgs&);

	private:

        /// Increments reference count. Can be reimplemented by child classes
        virtual int32_t InternalAddReference() const override;

        /// Decrements reference count. Can be reimplemented by child classes
        virtual int32_t InternalRelease() const override;

        void SaveSettings();

		NS_DECLARE_REFLECTION(MainSettings, IrrWindow)
	};
}

#endif
