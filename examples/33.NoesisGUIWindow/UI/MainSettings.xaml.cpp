#include "MainSettings.xaml.h"
#include <NsApp/IrrWindow.h>
#include <NsGui/IntegrationAPI.h>
#include <NsCore/ReflectionImplement.h>

#include "Config/Config.h"
#include "System/BaseTypes.h"
#include "System/Threading/TaskFactory.h"
#include "System/Threading/Task.h"

#include "UI/FileDialog.xaml.h"

#include "coreutil.h"
#include "irrlicht.h"
#include "IAnimatedMesh.h"

#include "App.xaml.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::MainSettings)
{
	NsMeta<Noesis::TypeId>("WorldClient.UI.MainSettings");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, MainSettings)
{
	NS_REGISTER_COMPONENT(WorldClient::MainSettings)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
WorldClient::MainSettings::MainSettings()
{
	Initialized() += Noesis::MakeDelegate(this, &MainSettings::InitializeComponent);
}

WorldClient::MainSettings::~MainSettings()
{
    m_dataContext->PropertyChanged() -= Noesis::MakeDelegate(this, &SelfClass::OnDataContextPropertyChanged);
}

void WorldClient::MainSettings::InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
    m_dataContext = Noesis::MakePtr<GlobalSettingEntry>();

    //m_dataContext->SetWowLocation(sConfig.GetDBValue("WorldClient.Data.ClientLocation", "").c_str());
    //m_dataContext->SetRenderEngine((irr::video::E_DRIVER_TYPE)sConfig.GetDBValue("WorldClient.Config.RenderDriver", irr::video::E_DRIVER_TYPE::EDT_DIRECT3D11));

    //m_dataContext->m_renderResolution.x = sConfig.GetDBValue("WorldClient.Config.Resolution.x", 1024);
    //m_dataContext->m_renderResolution.y = sConfig.GetDBValue("WorldClient.Config.Resolution.y", 768);

    //m_dataContext->SetColorDepth(sConfig.GetDBValue("WorldClient.Config.Resolution.colordepth", 32));
    //m_dataContext->SetMsaa(sConfig.GetDBValue("WorldClient.Config.Resolution.msaa", 4));
    //m_dataContext->SetWindowMode(sConfig.GetDBValue("WorldClient.Config.Resolution.windowed", true));
    //m_dataContext->SetVSync(sConfig.GetDBValue("WorldClient.Config.Resolution.vsync", true));

    SetDataContext(m_dataContext);

    Noesis::ComboBox* _comboBox = FindName<Noesis::ComboBox>("cbResolution");
    if (_comboBox)
    {
        auto items = _comboBox->GetItems();

        items->Add(new DisplayResolution{ 800,  600, 32 });
        items->Add(new DisplayResolution{ 1024, 768, 32 });
        items->Add(new DisplayResolution{ 1680, 1050, 32 });
        items->Add(new DisplayResolution{ 1600, 1200, 32 });
        items->Add(new DisplayResolution{ 1920, 1200, 32 });

        int8_t selected = 0;
        for (uint8_t i = 0; i < items->Count(); ++i)
        {
            auto element = static_cast<DisplayResolution*>(items->GetItemAt(i).GetPtr());
            if (element->GetWidth() == m_dataContext->m_renderResolution.x &&
                element->GetHeight() == m_dataContext->m_renderResolution.y &&
                element->GetColorDepth() == m_dataContext->GetColorDepth())
            {
                selected = i;
                break;
            }
        }

        _comboBox->SetSelectedIndex(selected);
    }

    _comboBox = FindName<Noesis::ComboBox>("cbDrivers");
    if (_comboBox)
    {
        auto items = _comboBox->GetItems();

        items->Add(new DisplayDriver{ irr::video::E_DRIVER_TYPE::EDT_DIRECT3D11 });
        items->Add(new DisplayDriver{ irr::video::E_DRIVER_TYPE::EDT_VULKAN });
        items->Add(new DisplayDriver{ irr::video::E_DRIVER_TYPE::EDT_OPENGL });

        int8_t selected = 0;
        for (uint8_t i = 0; i < items->Count(); ++i)
        {
            auto element = static_cast<DisplayDriver*>(items->GetItemAt(i).GetPtr());
            if (element->GetDriverID() == m_dataContext->GetRenderEngine())
            {
                selected = i;
                break;
            }
        }

        _comboBox->SetSelectedIndex(selected);
    }

    m_dataContext->PropertyChanged() += Noesis::MakeDelegate(this, &SelfClass::OnDataContextPropertyChanged);
}

void WorldClient::MainSettings::OnSelectionChangedResolution(Noesis::BaseComponent* combobox, const Noesis::SelectionChangedEventArgs& e)
{
    Noesis::ComboBox* _comboBox = static_cast<Noesis::ComboBox*>(combobox);
    if (!_comboBox && _comboBox->GetSelectedIndex() < 0)
        return;

    DisplayResolution* element = static_cast<DisplayResolution*>(_comboBox->GetSelectedItem());
    if (!element)
        return;

    Noesis::Vector2i renderResolution;
    renderResolution.x = element->GetWidth();
    renderResolution.y = element->GetHeight();

    m_dataContext->SetRenderResolution(renderResolution);
    m_dataContext->SetColorDepth(element->GetColorDepth());
}

void WorldClient::MainSettings::OnSelectionChangedDriver(Noesis::BaseComponent* combobox, const Noesis::SelectionChangedEventArgs& e)
{
    Noesis::ComboBox* _comboBox = static_cast<Noesis::ComboBox*>(combobox);
    if (!_comboBox && _comboBox->GetSelectedIndex() < 0)
        return;

    DisplayDriver* element = static_cast<DisplayDriver*>(_comboBox->GetSelectedItem());
    if (!element)
        return;

    m_dataContext->SetRenderEngine(element->GetDriverID());
}

void WorldClient::MainSettings::OnSelectButtonClick(Noesis::BaseComponent* button, const Noesis::RoutedEventArgs& e)
{
    irr::SIrrlichtCreationParameters p = WorldClient::GetDefaultIrrDeviceParam();
    p.WindowSize.Width = 10;
    p.WindowSize.Height = 10;
    auto dialog = Noesis::DynamicCast<WorldClient::FileDialog*>(App::Instance()->CreateNsWindow("pack://application:,,,/33.NoesisGUIWindow;/UI/FileDialog.xaml", &p));

    if (dialog)
    {
        m_dialogFileSelect.Reset(dialog);
        dialog->SetFileSystem(nullptr, false);
        dialog->SetData(true, m_dataContext->GetWowLocation());
        dialog->SetDialogOwner(this);
        dialog->SetCallback([self = Noesis::Ptr<WorldClient::MainSettings>(this)](WorldClient::FileDialog* dialog)
        {
            if (dialog->IsSuccess())
            {
                auto tbwowLocation = self->FindName<Noesis::TextBox>("tbWoWLocation");
                if (tbwowLocation)
                    tbwowLocation->SetText(dialog->m_currentDirectory.c_str());
            }

            dialog->SetDialogOwner(nullptr);
            self->m_dialogFileSelect.Reset();
        });
    }
}

void WorldClient::MainSettings::OnTextChangedWoWLocation(Noesis::BaseComponent* textbox, const Noesis::RoutedEventArgs& e)
{
    auto tbwowLocation = Noesis::DynamicCast<Noesis::TextBox*>(textbox);
    if (tbwowLocation)
        m_dataContext->SetWowLocation(tbwowLocation->GetText());
}

void WorldClient::MainSettings::OnGotFocusWindow(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    BringIntoView();
}

void WorldClient::MainSettings::OnDataContextPropertyChanged(Noesis::BaseComponent*, const Noesis::PropertyChangedEventArgs& e)
{
    //if (!strcmp(e.propertyName.GetStr(), "WowLocation"))
    //    sConfig.SetDBValue("WorldClient.Data.ClientLocation", m_dataContext->GetWowLocation(), true);
    //
    //if (!strcmp(e.propertyName.GetStr(), "RenderEngine"))
    //    sConfig.SetDBValue("WorldClient.Config.RenderDriver", m_dataContext->GetRenderEngine(), true);
    //
    //if (!strcmp(e.propertyName.GetStr(), "RenderResolution"))
    //{
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.x", m_dataContext->GetRenderResolution().x, true);
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.y", m_dataContext->GetRenderResolution().y, true);
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.colordepth", m_dataContext->GetColorDepth(), true);
    //}
    //
    //if (!strcmp(e.propertyName.GetStr(), "Msaa"))
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.msaa", m_dataContext->GetMsaa(), true);
    //if (!strcmp(e.propertyName.GetStr(), "WindowMode"))
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.windowed", m_dataContext->GetWindowMode(), true);
    //if (!strcmp(e.propertyName.GetStr(), "VSync"))
    //    sConfig.SetDBValue("WorldClient.Config.Resolution.vsync", m_dataContext->GetVSync(), true);
}

int32_t WorldClient::MainSettings::InternalAddReference() const
{
    return NoesisApp::IrrWindow::InternalAddReference();
}

int32_t WorldClient::MainSettings::InternalRelease() const
{
    return NoesisApp::IrrWindow::InternalRelease();
}

void WorldClient::MainSettings::SaveSettings()
{
}

bool WorldClient::MainSettings::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
    NS_REGISTER_UI_EVENT(Noesis::Button, Click, OnSelectButtonClick)
    NS_REGISTER_UI_EVENT(Noesis::ComboBox, SelectionChanged, OnSelectionChangedDriver)
    NS_REGISTER_UI_EVENT(Noesis::ComboBox, SelectionChanged, OnSelectionChangedResolution)
    NS_REGISTER_UI_EVENT(Noesis::TextBox, TextChanged, OnTextChangedWoWLocation)
    NS_REGISTER_UI_EVENT(NoesisApp::IrrWindow, GotFocus, OnGotFocusWindow)
	return true;
}
