#ifndef __C_ROOTWINDOW_H__
#define __C_ROOTWINDOW_H__

#include <NsCore/Noesis.h>
#include <NsApp/Window.h>
#include <NsGui/TextBox.h>
#include <NsGui/ComboBox.h>
#include <NsApp/DispatcherTimer.h>

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
	class RootWindow final : public NoesisApp::Window
	{
	public:

		RootWindow();
		virtual ~RootWindow();

		bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;
		void InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&);

	private:
		NS_DECLARE_REFLECTION(RootWindow, Window)
	};
}

#endif
