#include "MainWindow.xaml.h"
#include "FileDialog.xaml.h"
#include <NsGui/IntegrationAPI.h>
#include <NsCore/ReflectionImplement.h>
#include <NsApp/Window.h>

#include "Config/Config.h"
#include "System/BaseTypes.h"
#include "System/Threading/TaskFactory.h"
#include "System/Threading/Task.h"

#include "coreutil.h"
#include "irrlicht.h"
#include "IAnimatedMesh.h"
#include "CFileSystem.h"

#include "App.xaml.h"

using namespace irr;

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::MainWindow)
{
	NsMeta<Noesis::TypeId>("WorldClient.MainWindow");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, MainWindow)
{
	NS_REGISTER_COMPONENT(WorldClient::MainWindow)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
WorldClient::MainWindow::MainWindow()
{
	Initialized() += Noesis::MakeDelegate(this, &MainWindow::InitializeComponent);
}

WorldClient::MainWindow::~MainWindow()
{
}

void WorldClient::MainWindow::InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
    NS_REGISTER_UI_CONTROL(m_tbProgressStatus, "tbProgressStatus");
    NS_REGISTER_UI_CONTROL(m_miWoWDataContent, "miWoWDataContent");
    NS_REGISTER_UI_CONTROL(m_pbProgress, "pbProgress");
    NS_REGISTER_UI_CONTROL(m_tbMapCoord, "tbMapCoord");
    NS_REGISTER_UI_CONTROL(m_lbHistoryList, "lbHistoryList");
    
    //if (m_lbHistoryList)
    //{
    //    System::String location = sConfig.GetDBValue("WorldClient.Data.FileHistory", "");
	//
    //    System::String::Tokens parts;
    //    location.Tokenize(parts, '|');
	//
    //    for (auto part : parts)
    //    {
    //        m_lbHistoryList->GetItems()->Add(new HistoryElement(part));
    //    }
    //}
	//
    //if (m_tbMapCoord)
    //{
    //    System::String mapCoord = sConfig.GetDBValue("WorldClient.Data.History.LastMapCoord", "0 0 0 0");
    //    m_tbMapCoord->SetText(mapCoord.c_str());
    //}
	//
    //std::string location = sConfig.GetDBValue("WorldClient.Data.ClientLocation", "");
    //if (!location.empty())
    //    LoadWoWDataContent();
}

bool WorldClient::MainWindow::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
    NS_REGISTER_UI_EVENT(Noesis::MenuItem, Click, OnOpenWoWClientContent)
    NS_REGISTER_UI_EVENT(Noesis::MenuItem, Click, OnLoadWoWClientContent)
	NS_REGISTER_UI_EVENT(Noesis::MenuItem, Click, OnWoWSettings)
    NS_REGISTER_UI_EVENT(NoesisApp::IrrWindow, GotFocus, OnGotFocusWindow)
    NS_REGISTER_UI_EVENT(Noesis::Button, Click, OnClientMode)
    NS_REGISTER_UI_EVENT(Noesis::Button, Click, OnHistoryStartClient)
	return true;
}

void WorldClient::MainWindow::OnOpenWoWClientContent(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
}

void WorldClient::MainWindow::OnLoadWoWClientContent(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
}

void WorldClient::MainWindow::OnWoWSettings(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    Noesis::Grid* holder = FindName<Noesis::Grid>("PART_CONTENTHOLDER");
    if (holder)
    {
        Noesis::Ptr<BaseComponent> root = Noesis::GUI::LoadXaml("pack://application:,,,/33.NoesisGUIWindow;/UI/MainSettings.xaml");
        auto mMainWindow = Noesis::DynamicPtrCast<NoesisApp::IrrWindow>(root);
        holder->GetChildren()->Add(mMainWindow);
    }
}

void WorldClient::MainWindow::OnGotFocusWindow(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    BringIntoView();
}

void WorldClient::MainWindow::OnClientMode(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
}

void WorldClient::MainWindow::OnHistoryStartClient(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    auto Item = static_cast<HistoryElement*>(m_lbHistoryList->GetSelectedItem());
    if (!Item)
        return;
}
