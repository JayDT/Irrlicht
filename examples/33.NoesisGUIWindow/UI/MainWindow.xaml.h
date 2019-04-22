#ifndef __C_MAINWINDOW_H__
#define __C_MAINWINDOW_H__

#include <NsCore/Noesis.h>
#include <NsApp/IrrWindow.h>
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
    class MenuItem;
    class TextBlock;
    class TextBox;
    class ProgressBar;
}

namespace WorldClient
{
    struct HistoryElement : public Noesis::BaseComponent
    {
        HistoryElement(const std::string& path)
            : m_path(path)
        {
        }

        virtual ~HistoryElement() = default;

        std::string m_path;

        const char* GetString() const
        {
            return m_path.c_str();
        }

        NS_IMPLEMENT_INLINE_REFLECTION(HistoryElement, Noesis::BaseComponent)
        {
            NsMeta<Noesis::TypeId>("WorldClient.HistoryElement");
            NsProp("String", &SelfClass::GetString);
        }
    };

    class MainWindow final : public NoesisApp::IrrWindow
    {
    private:

        Noesis::Ptr<Noesis::BaseComponent> m_dialog;
        Noesis::MenuItem*    m_miWoWDataContent  = nullptr;
        Noesis::TextBlock*   m_tbProgressStatus  = nullptr;
        Noesis::ProgressBar* m_pbProgress        = nullptr;
        Noesis::ListBox*     m_lbHistoryList     = nullptr;
        Noesis::TextBox*     m_tbMapCoord        = nullptr;

        std::shared_ptr<NoesisApp::DispatcherTimer> m_checkTimer;
        System::Events::EventHook<System::Events::EventArg>* m_activeCheckTimerHook = nullptr;
        //std::atomic<int> m_progress;

        std::list<std::string> m_fileList;

        irr::scene::ISceneNode* Model = nullptr;

    public:

		MainWindow();
		virtual ~MainWindow();

		bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;
        void InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&);

        void OnOpenWoWClientContent(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnLoadWoWClientContent(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
		void OnWoWSettings(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnGotFocusWindow(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnClientMode(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnHistoryStartClient(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);

	private:
		NS_DECLARE_REFLECTION(MainWindow, IrrWindow)
    };
}

#endif
