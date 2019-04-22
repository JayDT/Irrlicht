#include "RootWindow.xaml.h"
#include <NsApp/IrrWindow.h>
#include <NsGui/IntegrationAPI.h>
#include <NsCore/ReflectionImplement.h>

#include "Config/Config.h"
#include "System/BaseTypes.h"
#include "System/Threading/TaskFactory.h"
#include "System/Threading/Task.h"

#include "coreutil.h"
#include "irrlicht.h"
#include "IAnimatedMesh.h"

#include "App.xaml.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::RootWindow)
{
	NsMeta<Noesis::TypeId>("WorldClient.RootWindow");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, RootWindow)
{
	NS_REGISTER_COMPONENT(WorldClient::RootWindow)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
WorldClient::RootWindow::RootWindow()
{
	Initialized() += Noesis::MakeDelegate(this, &RootWindow::InitializeComponent);
}

WorldClient::RootWindow::~RootWindow()
{
}

void WorldClient::RootWindow::InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
}

bool WorldClient::RootWindow::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
	return true;
}
