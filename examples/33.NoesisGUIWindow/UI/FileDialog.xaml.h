#ifndef __C_FILE_DIAGLOG_WINDOW_H__
#define __C_FILE_DIAGLOG_WINDOW_H__

#include <NsCore/Noesis.h>
#include <NsApp/Window.h>
#include <NsGui/TextBox.h>
#include <NsGui/TreeView.h>
#include <NsGui/ComboBox.h>
#include <NsGui/TreeViewItem.h>
#include <NsApp/DispatcherTimer.h>
#include <irrlicht.h>

namespace irr::scene
{
	class ISceneNode;
}

namespace Noesis
{
	class Button;
	class ComboBox;
	class ListBox;
    class ListView;
    class TreeView;
	class TextBlock;
	class TextBox;
	class ProgressBar;
}

namespace WorldClient
{
    struct FileListElement : public Noesis::BaseComponent
    {
        FileListElement(uint32_t id, const std::string& name, const std::string& ext, uint32_t size)
            : m_id(id)
            , m_name(std::move(name))
            , m_ext(std::move(ext))
            , m_size(size)
        {
        }

        virtual ~FileListElement()
        {
        }

        uint32_t m_id;
        std::string m_name;
        std::string m_ext;
        uint32_t m_size;

        const char* GetFileName() const
        {
            return m_name.c_str();
        }

        const char* GetFileExt() const
        {
            return m_ext.c_str();
        }

        uint32_t GetFileSize() const
        {
            return m_size;
        }

    private:

        NS_IMPLEMENT_INLINE_REFLECTION(FileListElement, Noesis::BaseComponent)
        {
            NsMeta<Noesis::TypeId>("WorldClient.FileListElement");
            NsProp("FileName", &SelfClass::GetFileName);
            NsProp("FileExt", &SelfClass::GetFileExt);
            NsProp("FileSize", &SelfClass::GetFileSize);
        }
    };

	class FileDialog final : public NoesisApp::Window
	{
        Noesis::ListView* m_fileListView;
        Noesis::TreeView* m_directoryView;
        Noesis::TextBox*  m_filePath;
        Noesis::TextBox*  m_fileName;
        Noesis::TextBox*  m_fileType;

        Noesis::Ptr<Noesis::Control>   m_dialogOwner;
        irr::io::IFileArchive*         m_baseArchive = nullptr;
        irr::Ptr<irr::io::IFileArchive> m_filteredArchive;
        irr::Ptr<irr::io::IFileSystem> m_fileSystem;
        irr::Ptr<irr::io::IFileList>   m_fileList;
        std::string m_filter;

        std::function<void(FileDialog*)> m_callback;

        bool m_filterVirtualFileSystem = false;
        bool m_initialized = false;

    public:
        std::string m_restoreDirectory;
        std::string m_currentDirectory;
        std::vector<std::string> m_selectedFiles;
        bool m_success = false;

	public:

        FileDialog();
		virtual ~FileDialog();

		bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;
		void InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&);
        void OnHandleClose(Noesis::BaseComponent*, const Noesis::EventArgs&);
        void OnTreeViewItem(Noesis::BaseComponent*, const Noesis::DependencyPropertyChangedEventArgs&);
        void OnSelectTreeViewItem(Noesis::BaseComponent*, const Noesis::RoutedPropertyChangedEventArgs<Noesis::Ptr<Noesis::BaseComponent>>&);
        void OnSelectListViewItem(Noesis::BaseComponent*, const Noesis::SelectionChangedEventArgs&);
        void OnDirectoryTextChanged(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnKeyUpFilter(Noesis::BaseComponent*, const Noesis::KeyEventArgs&);
        void OnMouseListViewItem(Noesis::BaseComponent*, const Noesis::MouseButtonEventArgs&);
        void OnKeyUpListViewItem(Noesis::BaseComponent*, const Noesis::KeyEventArgs&);
        void OnButtonBackDirectory(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnButtonSelect(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);
        void OnButtonCancel(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&);

        bool IsSuccess() const { return m_success; }
        const std::string& GetFilter() const { return m_filter; }

        void SetFileSystem(irr::io::IFileArchive* filesystem, bool filterVirtualFileSystem);
        void SetData(bool restoreCWD, const char* startDir, const char* filter = nullptr);
        void SetDialogOwner(Noesis::Control* owner);

        void SetCallback(const std::function<void(FileDialog*)>& cb)
        {
            m_callback = cb;
        }

	private:

        void updateDirectoryView(Noesis::TreeViewItem* parent, bool isWorkingDirLeaf = false);
        void updateView();
        void updateFilteredDataSource();

		NS_DECLARE_REFLECTION(FileDialog, Window)
	};
}

#endif
