#ifndef __APP_APPLICATION_H__
#define __APP_APPLICATION_H__

#include <NsApp/IrrNoesis.h>
#include <NsCore/Noesis.h>
#include <NsGui/IUITreeNode.h>
#include <NsApp/CommandLine.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/ReflectionDeclare.h>
#include <NsCore/Delegate.h>
#include <NsGui/UIElementEvents.h>
#include <NsGui/ContentControl.h>
#include <NsCore/Package.h>

#include "include/IReferenceCounted.h"
#include <list>

namespace irr
{
	class IrrlichtDevice;
}

namespace Noesis
{
    class ResourceDictionary;
}

namespace NoesisApp
{

    class IrrNsDeviceStub;
    class Window;
    class IrrRenderContext;

    NS_WARNING_PUSH
    NS_MSVC_WARNING_DISABLE(4251 4275)

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Encapsulates a NoesisGUI application.
    ///
    /// http://msdn.microsoft.com/en-us/library/system.windows.application.aspx
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class NS_IRR_NOESIS_API Application : public Noesis::BaseComponent, public Noesis::IUITreeNode
    {
    public:
			
		Application();
        virtual ~Application();

        /// Gets the current application object
        static Application* Current();

        /// Gets or sets a collection of application-scope resources, such as styles and brushes
        //@{
        Noesis::ResourceDictionary* GetResources() const;
        void SetResources(Noesis::ResourceDictionary* resources);
        //@}

        /// Retrieves command line arguments
        const CommandLine& GetArguments() const;

        /// Initializes the application with on the given display and command line arguments
        void Init(IrrNsDeviceStub* display, const CommandLine& arguments);

        /// Ticks application
        void Tick(double time);

		void Idle(double time);

        /// Shuts down an application that returns the specified exit code to the operating system
        void Shutdown(int exitCode = 0);

        /// Occurs when an application becomes the foreground application
        Noesis::EventHandler& Activated();

        /// Occurs when an application stops being the foreground application
        Noesis::EventHandler& Deactivated();

        /// Occurs when the Application is initialized
        Noesis::EventHandler& StartUp();

        /// Occurs just before an application shuts down
        Noesis::EventHandler& Exit();

		/// Add root child element
		void AddChildren(NoesisApp::Window* child, NoesisApp::IrrRenderContext* context = nullptr);

		/// Remove root child element
		void RemoveChildren(NoesisApp::Window* child);

		NoesisApp::Window* CreateNsWindow(const char* uri, irr::SIrrlichtCreationParameters* deviceDesc = nullptr);

        BaseComponent* FindName(const char* name) const;

        /// From IUITreeNode
        //@{
        IUITreeNode* GetNodeParent() const override;
        void SetNodeParent(IUITreeNode* parent) override;
        Noesis::BaseComponent* FindNodeResource(Noesis::IResourceKey* key, bool fullSearch) const override;
        Noesis::BaseComponent* FindNodeName(const char* name) const override;
        Noesis::ObjectWithNameScope FindNodeNameAndScope(const char* name) const override;
        //@}

        irr::IrrlichtDevice* GetIrrlichDevice() const { return mIrrDevice; }

        NS_IMPLEMENT_INTERFACE_FIXUP

	protected:

		void SetIrrlichDevice(irr::IrrlichtDevice* device);
        void SetIntegrationAPICallbacks(IrrNsDeviceStub* display);

    private:
        virtual const char* GetTitleOverride(Noesis::UIElement* root) const;
        virtual Noesis::Ptr<IrrRenderContext> GetRenderContextOverride() const;
        virtual bool GetPPAAOverride() const;
        virtual uint32_t GetSamplesOverride() const;
        virtual bool GetVSyncOverride() const;
        virtual bool GetsRGBOverride() const;

        void EnsureResources() const;
        bool OnActivated(IrrNsDeviceStub* display);
        bool OnDeactivated(IrrNsDeviceStub* display);
        bool OnWindowClosed(IrrNsDeviceStub* display);

		void Shutdown(NoesisApp::Window* child) noexcept;

    protected:
        mutable Noesis::Ptr<Noesis::ResourceDictionary> mResources;
		IUITreeNode* mOwner;
		std::list<Noesis::Ptr<NoesisApp::Window>> mChildrens;

        CommandLine mArguments;

		irr::Ptr<irr::IrrlichtDevice> mIrrDevice;
		Noesis::Ptr<NoesisApp::Window> mMainWindow;
		Noesis::Ptr<NoesisApp::IrrNsDeviceStub> mMainDisplay;
		Noesis::Ptr<NoesisApp::IrrRenderContext> mRenderContext;

        Noesis::EventHandler mActivated;
        Noesis::EventHandler mDeactivated;
        Noesis::EventHandler mStartUp;
        Noesis::EventHandler mExit;

		NS_DECLARE_REFLECTION(Application, BaseComponent)
    };

    NS_WARNING_POP

}

#endif
