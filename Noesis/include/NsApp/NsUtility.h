#ifndef __APP_ITOPLEVEL_H__
#define __APP_ITOPLEVEL_H__

#include <NsApp/IrrNoesis.h>
#include <NsApp/IrrWindow.h>
#include <NsApp/Window.h>
#include <NsCore/Noesis.h>

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

    class Extensions final
    {
    public:
        static NoesisApp::Window* FindRootWindow(Noesis::FrameworkElement* element)
        {
            while (element->GetParent())
                element = element->GetParent();
            return Noesis::DynamicCast<NoesisApp::Window*>(element);
        }
    };

	class RemoveChildHelper final
	{
	public:
		static void RemoveChild(Noesis::DependencyObject* parent, Noesis::UIElement* child)
		{
			auto panel = Noesis::DynamicCast<Noesis::Panel*>(parent);
			if (panel != nullptr)
			{
				panel->GetChildren()->Remove(child);
				return;
			}

			auto decorator = Noesis::DynamicCast<Noesis::Decorator*>(parent);
			if (decorator != nullptr)
			{
				if (decorator->GetChild() == child)
				{
					decorator->SetChild(nullptr);
				}
				return;
			}

			auto contentPresenter = Noesis::DynamicCast<Noesis::ContentPresenter*>(parent);
			if (contentPresenter != nullptr)
			{
				if (contentPresenter->GetContent() == child)
				{
					contentPresenter->SetContent((Noesis::UIElement*)nullptr);
				}
				return;
			}

			auto contentControl = Noesis::DynamicCast<Noesis::ContentControl*>(parent);
			if (contentControl != nullptr)
			{
				if (contentControl->GetContent() == child)
				{
					contentControl->SetContent((Noesis::UIElement*)nullptr);
				}
				return;
			}

			// maybe more
		}
	};
}

#endif