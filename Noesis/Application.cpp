#include <NsApp/Application.h>
#include <NsApp/Window.h>
#include <NsGui/DependencyProperty.h>
#include <NsGui/TypeConverterMetaData.h>
#include <NsGui/ResourceDictionary.h>
#include <NsGui/IntegrationAPI.h>
#include <NsCore/ReflectionImplement.h>
#include <NsCore/TypeId.h>
#include <NsCore/HighResTimer.h>
#include <NsImpl/IrrRenderContext.h>
#include <EASTL/algorithm.h>
#include "NsImpl/IrrRenderContext.h"
#include "NsImpl/IrrOsDeviceContext.h"

#include "RenderEngines/General/CIrrShader.h"

using namespace Noesis;
using namespace NoesisApp;

#undef PlaySound

namespace
{
    Application* gInstance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(Application, Application)
{
	NS_REGISTER_COMPONENT(Application)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Application::Application(): mOwner(0)
{
    // More than one instantation is possible, for example when dropping App.xaml into XamlPlayer
    if (gInstance == 0)
    {
        gInstance = this;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Application::~Application()
{
    mExit(this, EventArgs::Empty);

	Shutdown(mMainWindow.GetPtr());

	for (const auto& child : mChildrens)
	{
		Shutdown(child.GetPtr());
	}

	mChildrens.clear();
    mRenderContext.Reset();

    if (mResources != 0)
    {
        mResources->RemoveDictionaryChangedListeners();
        mResources.Reset();
    }

    if (gInstance == this)
    {
        gInstance = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Application* Application::Current()
{
    NS_ASSERT(gInstance != 0);
    return gInstance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ResourceDictionary* Application::GetResources() const
{
    EnsureResources();
    return mResources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::SetResources(ResourceDictionary* resources)
{
    if (mResources != resources)
    {
        DisconnectNode(mResources, this);
        mResources.Reset(resources);
        ConnectNode(mResources, this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const CommandLine& Application::GetArguments() const
{
    return mArguments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::Init(IrrNsDeviceStub* display, const CommandLine& arguments)
{
    mArguments = arguments;

    // Application resources
    GUI::SetApplicationResources(GetResources());
    SetIntegrationAPICallbacks(display);
    
    display->Activated() += MakeDelegate(this, &Application::OnActivated);
    display->Deactivated() += MakeDelegate(this, &Application::OnDeactivated);
    display->Closed() += MakeDelegate(this, &Application::OnWindowClosed);

    mRenderContext = GetRenderContextOverride();
    bool ppaa = GetPPAAOverride();
    uint32_t samples = GetSamplesOverride();
    bool vSync = GetVSyncOverride();
    bool sRGB = GetsRGBOverride();
    
    mRenderContext->Init(display->GetNativeHandle(), samples, vSync, sRGB);
    mRenderContext->GetDevice()->SetOffscreenSampleCount(samples);
    
    mStartUp(this, EventArgs::Empty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::Tick(double time)
{
	NS_PROFILE_CPU("App/Tick");

	mMainWindow->Render(time);
}

void NoesisApp::Application::Idle(double time)
{
	NS_PROFILE_CPU("App/Idle");

	for (const auto& child : mChildrens)
	{
		if (child->GetIrrRenderContext()->GetIrrDevice()->run())
		{
			child->GetIrrRenderContext()->GetIrrDevice()->getVideoDriver()->beginScene();
			child->Render(time);
			child->GetIrrRenderContext()->GetIrrDevice()->getVideoDriver()->endScene();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::Shutdown(int exitCode)
{
    //((DisplayLauncher*)Launcher::Current())->Quit(exitCode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EventHandler& Application::Activated()
{
    return mActivated;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EventHandler& Application::Deactivated()
{
    return mDeactivated;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EventHandler& Application::StartUp()
{
    return mStartUp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EventHandler& Application::Exit()
{
    return mExit;
}

/// Add root child element
void NoesisApp::Application::AddChildren(NoesisApp::Window* child, NoesisApp::IrrRenderContext* context)
{
	if (context == mRenderContext)
	{
		mMainWindow = *child;
	}
	else
	{
		mChildrens.push_back(*child);
	}
}

/// Remove root child element
void NoesisApp::Application::RemoveChildren(NoesisApp::Window* child)
{
	for (auto itr = mChildrens.begin(); itr != mChildrens.end(); ++itr)
	{
		if ((*itr) == child)
		{
			mChildrens.erase(itr);
			break;
		}
	}
}

Window* NoesisApp::Application::CreateNsWindow(const char* uri, irr::SIrrlichtCreationParameters* deviceDesc /*= nullptr*/)
{
	Noesis::Ptr<IrrNsDeviceStub>  display;
	Noesis::Ptr<IrrRenderContext> renderContext;
	Noesis::Ptr<Window> wnd;

	if (deviceDesc)
	{
		auto _device = irr::createDeviceEx(*deviceDesc);

		display = NoesisApp::CreateDisplay(_device);

		renderContext = NoesisApp::CreateRenderContext("Irr");
		renderContext->SetIrrlichtDevice(_device);
        SetIntegrationAPICallbacks(display);

		display->Activated() += MakeDelegate(this, &Application::OnActivated);
		display->Deactivated() += MakeDelegate(this, &Application::OnDeactivated);
        display->Closed() += MakeDelegate(this, &Application::OnWindowClosed);

		bool ppaa = GetPPAAOverride();
		uint32_t samples = GetSamplesOverride();
		bool vSync = GetVSyncOverride();
		bool sRGB = GetsRGBOverride();

		renderContext->Init(display->GetNativeHandle(), samples, vSync, sRGB);
		renderContext->GetDevice()->SetOffscreenSampleCount(samples);
	}
	else
	{
		display = mMainDisplay;
		renderContext = mRenderContext;
	}

	if (display && renderContext)
	{
		Noesis::Ptr<BaseComponent> root = Noesis::GUI::LoadXaml(uri);
		wnd = Noesis::DynamicPtrCast<NoesisApp::Window>(root);
		wnd->Init(display.GetPtr(), renderContext.GetPtr(), 1, false);
		AddChildren(wnd, renderContext.GetPtr());
	}

	return wnd.GetPtr();
}

Noesis::BaseComponent* NoesisApp::Application::FindName(const char* name) const
{
    if (mMainWindow)
    {
        if (!Noesis::String::Compare(name, mMainWindow->GetName()))
            return mMainWindow.GetPtr();

        Noesis::BaseComponent* component = mMainWindow->FindName(name);
        if (component)
            return component;
    }

    for (const auto& child : mChildrens)
    {
        if (!Noesis::String::Compare(name, child->GetName()))
            return child.GetPtr();

        Noesis::BaseComponent* component = child->FindName(name);
        if (component)
            return component;
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
IUITreeNode* Application::GetNodeParent() const
{
    return mOwner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::SetNodeParent(IUITreeNode* parent)
{
    mOwner = parent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BaseComponent* Application::FindNodeResource(IResourceKey* key, bool fullElementSearch) const
{
    Ptr<BaseComponent> resource = 0;
    if (mResources && mResources->Find(key, resource))
    {
        return resource;
    }

    if (mOwner)
    {
        return mOwner->FindNodeResource(key, fullElementSearch);
    }

    return DependencyProperty::GetUnsetValue();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BaseComponent* Application::FindNodeName(const char* name) const
{
    if (mOwner)
    {
        return mOwner->FindNodeName(name);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectWithNameScope Application::FindNodeNameAndScope(const char* name) const
{
    if (mOwner)
    {
        return mOwner->FindNodeNameAndScope(name);
    }
    
    return ObjectWithNameScope();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::SetIrrlichDevice(irr::IrrlichtDevice* device)
{
	mIrrDevice = irr::Ptr(device);
}

void NoesisApp::Application::SetIntegrationAPICallbacks(IrrNsDeviceStub* display)
{
    // Redirect integration callbacks to display
    Noesis::GUI::SetSoftwareKeyboardCallback(display, [](void* user, UIElement * focused, bool open)
    {
        if (open)
        {
            ((IrrNsDeviceStub*)user)->OpenSoftwareKeyboard(focused);
        }
        else
        {
            ((IrrNsDeviceStub*)user)->CloseSoftwareKeyboard();
        }
    });

    Noesis::GUI::SetCursorCallback(display, [](void* user, IView*, Cursor cursor)
    {
        ((IrrNsDeviceStub*)user)->SetCursor(cursor);
    });

    Noesis::GUI::SetOpenUrlCallback(display, [](void* user, const char* url)
    {
        ((IrrNsDeviceStub*)user)->OpenUrl(url);
    });

    Noesis::GUI::SetPlaySoundCallback(display, [](void* user, const char* filename, float volume)
    {
        ((IrrNsDeviceStub*)user)->PlaySound(filename, volume);
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const char* Application::GetTitleOverride(UIElement* root) const
{
    Window* window = Noesis::DynamicCast<Window*>(root);
    return window ? window->GetTitle() : "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Ptr<IrrRenderContext> Application::GetRenderContextOverride() const
{
	if (!mRenderContext)
	{
		auto renderContext = NoesisApp::CreateRenderContext("Irr");
		renderContext->SetIrrlichtDevice(mIrrDevice);
		return renderContext;
		//SetIrrlichtDevice
	}
    return mRenderContext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Application::GetPPAAOverride() const
{
    return !mArguments.HasOption("samples") && atoi(mArguments.FindOption("ppaa", "1")) != 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Application::GetSamplesOverride() const
{
    return atoi(mArguments.FindOption("samples", "1"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Application::GetVSyncOverride() const
{
    return atoi(mArguments.FindOption("vsync", "1")) != 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Application::GetsRGBOverride() const
{
    return mArguments.HasOption("linear");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Application::EnsureResources() const
{
    if (mResources == 0)
    {
        mResources = *new ResourceDictionary();
        ConnectNode(mResources, this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Application::OnActivated(IrrNsDeviceStub*)
{
    mActivated(this, EventArgs::Empty);
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Application::OnDeactivated(IrrNsDeviceStub*)
{
    mDeactivated(this, EventArgs::Empty);
    return false;
}

bool NoesisApp::Application::OnWindowClosed(IrrNsDeviceStub* display)
{
    display->Activated() -= MakeDelegate(this, &Application::OnActivated);
    display->Deactivated() -= MakeDelegate(this, &Application::OnDeactivated);
    display->Closed() -= MakeDelegate(this, &Application::OnWindowClosed);
    return false;
}

//void Application::Render(double time, RootChild& child) noexcept
//{
//	// Update
//	child.mView->Update(time);
//
//	auto renderer = child.mView->GetRenderer();
//	auto context = static_cast<NoesisApp::IrrRenderContext*>(child.mRenderContext);
//	const auto& screenSize = context->GetIrrDevice()->getVideoDriver()->getScreenSize();
//	renderer->UpdateRenderTree();
//
//	if (renderer->NeedsOffscreen())
//		renderer->RenderOffscreen();
//
//	mRenderContext->SetDefaultRenderTarget(screenSize.Width, screenSize.Height);
//	renderer->Render();
//}

void Application::Shutdown(NoesisApp::Window* child) noexcept
{
	if (!child)
		return;

	child->GetView()->GetRenderer()->Shutdown();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(Application)
{
	NsMeta<TypeId>("Application");

	NsImpl<IUITreeNode>();

	NsProp("Resources", &Application::GetResources, &Application::SetResources);

	NsEvent("Activated", &Application::mActivated);
	NsEvent("Deactivated", &Application::mDeactivated);
	NsEvent("StartUp", &Application::mStartUp);
	NsEvent("Exit", &Application::mExit);
}