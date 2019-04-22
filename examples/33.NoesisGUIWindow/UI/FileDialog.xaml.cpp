#include "FileDialog.xaml.h"
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

#include <regex>

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMPLEMENT_REFLECTION(WorldClient::FileDialog)
{
	NsMeta<Noesis::TypeId>("WorldClient.FileDialog");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_REGISTER_REFLECTION(WorldClient, FileDialog)
{
	NS_REGISTER_COMPONENT(WorldClient::FileDialog)
}

WorldClient::FileDialog::FileDialog()
{
    Initialized() += Noesis::MakeDelegate(this, &FileDialog::InitializeComponent);
    Closed() += Noesis::MakeDelegate(this, &FileDialog::OnHandleClose);

    m_fileSystem.Reset(App::Instance()->GetIrrlichDevice()->getFileSystem());

    if (m_fileSystem)
    {
        m_restoreDirectory = m_fileSystem->getWorkingDirectory().c_str();
        m_currentDirectory = m_fileSystem->getWorkingDirectory().c_str();
    }
}

WorldClient::FileDialog::~FileDialog()
{
    SetDialogOwner(nullptr);
}

void WorldClient::FileDialog::SetFileSystem(irr::io::IFileArchive* filesystem, bool filterVirtualFileSystem)
{
    m_initialized = true;

    if (m_baseArchive)
        m_fileSystem->removeFileArchive(m_baseArchive);

    if (filesystem)
    {
        m_baseArchive = filesystem;
        m_baseArchive->grab();
        m_fileSystem->addFileArchive(filesystem);
        m_fileSystem->setFileListSystem(irr::io::EFileSystemType::FILESYSTEM_VIRTUAL);
    }
    else
    {
        m_baseArchive = nullptr;
        m_fileSystem->setFileListSystem(irr::io::EFileSystemType::FILESYSTEM_NATIVE);
    }

    m_filterVirtualFileSystem = filterVirtualFileSystem;
    m_directoryView->GetItems()->Clear();
    m_fileListView->GetItems()->Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldClient::FileDialog::SetData(bool restoreCWD, const char* startDir, const char* filter)
{
    if (m_fileSystem)
    {
        if (filter)
        {
            m_initialized = false;
            m_filter = filter;
            m_fileType->SetText(filter);
            m_initialized = true;

            updateFilteredDataSource();
        }

        if (restoreCWD)
            m_restoreDirectory = m_fileSystem->getWorkingDirectory().c_str();
        if (startDir)
        {
            if (m_fileSystem->changeWorkingDirectoryTo(startDir))
            {
                m_currentDirectory = startDir;
                m_fileSystem->changeWorkingDirectoryTo("/");
                m_directoryView->GetItems()->Clear();
                updateDirectoryView(0);
                m_fileSystem->changeWorkingDirectoryTo(m_currentDirectory.c_str());

                updateView();
            }
        }
    }
}

void WorldClient::FileDialog::SetDialogOwner(Noesis::Control* owner)
{
    if (m_dialogOwner)
        m_dialogOwner->SetIsEnabled(true);
    m_dialogOwner.Reset(owner);
    if (m_dialogOwner)
        m_dialogOwner->SetIsEnabled(false);
}

void WorldClient::FileDialog::InitializeComponent(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
	m_fileListView = FindName<Noesis::ListView>("PART_FileList");
    m_directoryView = FindName<Noesis::TreeView>("PART_DirectoryList");
    m_filePath = FindName<Noesis::TextBox>("PART_CurrentPath");
    m_fileName = FindName<Noesis::TextBox>("PART_FileName");
    m_fileType = FindName<Noesis::TextBox>("PART_FileType");

    //m_fileSystem->changeWorkingDirectoryTo("/");
    //updateDirectoryView(0);
    //m_fileSystem->changeWorkingDirectoryTo(m_currentDirectory.c_str());
    //
    //updateView();
}

void WorldClient::FileDialog::OnHandleClose(Noesis::BaseComponent*, const Noesis::EventArgs&)
{
    if (m_callback)
        m_callback(this);
    m_callback = nullptr;
    if (!m_restoreDirectory.empty())
        m_fileSystem->changeWorkingDirectoryTo(m_restoreDirectory.c_str());
}

void WorldClient::FileDialog::OnTreeViewItem(Noesis::BaseComponent* element, const Noesis::DependencyPropertyChangedEventArgs& e)
{
    Noesis::TreeViewItem* item = Noesis::DynamicCast<Noesis::TreeViewItem*>(element);
    if (!item || !m_initialized || !item->GetIsVisible())
        return;

    m_fileSystem->changeWorkingDirectoryTo(item->GetName());
    
    if (item)
        updateDirectoryView(item);

    m_fileSystem->changeWorkingDirectoryTo(m_currentDirectory.c_str());
}

void WorldClient::FileDialog::OnSelectTreeViewItem(Noesis::BaseComponent*, const Noesis::RoutedPropertyChangedEventArgs<Noesis::Ptr<Noesis::BaseComponent>>&)
{
    Noesis::TreeViewItem* item = Noesis::DynamicCast<Noesis::TreeViewItem*>(m_directoryView->GetSelectedItem());
    if (!item || !m_initialized)
        return;

    if (m_fileSystem->changeWorkingDirectoryTo(item->GetName()))
    {
        updateView();
    }
}

void WorldClient::FileDialog::OnSelectListViewItem(Noesis::BaseComponent*, const Noesis::SelectionChangedEventArgs&)
{
    FileListElement* item = Noesis::DynamicCast<FileListElement*>(m_fileListView->GetSelectedItem());
    if (!item)
        return;

    m_fileName->SetText(item->GetFileName());
    if (m_selectedFiles.empty())
        m_selectedFiles.resize(1);
    m_selectedFiles[0] = item->GetFileName();

    auto items = m_fileListView->GetSelectedItems();
    if (items && items->Count() > 1)
    {
        m_selectedFiles.resize(items->Count());
        for (uint32_t i = 0; i != items->Count(); ++i)
        {
            FileListElement* item = Noesis::DynamicCast<FileListElement*>(items->Get(i));
            if (item)
                m_selectedFiles[i] = item->GetFileName();
        }
    }
}

void WorldClient::FileDialog::OnDirectoryTextChanged(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    if (!m_fileSystem->getWorkingDirectory().empty() && strlen(m_filePath->GetText()) && m_fileSystem->getWorkingDirectory()[0] != m_filePath->GetText()[0])
    {
        if (m_fileSystem->changeWorkingDirectoryTo(m_filePath->GetText()))
        {
            m_currentDirectory = m_filePath->GetText();
            m_fileSystem->changeWorkingDirectoryTo("/");
            m_directoryView->GetItems()->Clear();
            updateDirectoryView(0);
            m_fileSystem->changeWorkingDirectoryTo(m_currentDirectory.c_str());

            updateView();
        }

    }
    else if (m_fileSystem->changeWorkingDirectoryTo(m_filePath->GetText()))
    {
        m_currentDirectory = m_filePath->GetText();
        updateView();
    }
}

void WorldClient::FileDialog::OnKeyUpFilter(Noesis::BaseComponent*, const Noesis::KeyEventArgs& e)
{
    if (!m_initialized)
        return;

    if (e.key == Noesis::Key::Key_Enter)
    {
        m_filter = m_fileType->GetText();

        if (m_filterVirtualFileSystem)
        {
            updateFilteredDataSource();
            m_fileSystem->changeWorkingDirectoryTo("/");
            m_directoryView->GetItems()->Clear();
            updateDirectoryView(0);
            m_fileSystem->changeWorkingDirectoryTo(m_currentDirectory.c_str());
        }

        updateView();
    }
}

void WorldClient::FileDialog::OnMouseListViewItem(Noesis::BaseComponent*, const Noesis::MouseButtonEventArgs&)
{
    FileListElement* item = Noesis::DynamicCast<FileListElement*>(m_fileListView->GetSelectedItem());
    if (!item || strcmp(item->GetFileExt(), "Directory"))
        return;

    irr::io::path dir = m_fileSystem->getWorkingDirectory();
    if (dir.lastChar() != '/' || dir.lastChar() != '\\')
        dir.append('/');
    dir.append(item->GetFileName());
    if (dir.lastChar() != '/' || dir.lastChar() != '\\')
        dir.append('/');

    if (m_fileSystem->changeWorkingDirectoryTo(dir))
    {
        updateView();
    }
}

void WorldClient::FileDialog::OnKeyUpListViewItem(Noesis::BaseComponent*, const Noesis::KeyEventArgs& e)
{
    if (e.key == Noesis::Key::Key_Enter)
    {
        FileListElement* item = Noesis::DynamicCast<FileListElement*>(m_fileListView->GetSelectedItem());
        if (!item || strcmp(item->GetFileExt(), "Directory"))
            return;

        irr::io::path dir = m_fileSystem->getWorkingDirectory();
        if (dir.lastChar() != '/' || dir.lastChar() != '\\')
            dir.append('/');
        dir.append(item->GetFileName());
        if (dir.lastChar() != '/' || dir.lastChar() != '\\')
            dir.append('/');


        if (m_fileSystem->changeWorkingDirectoryTo(dir))
        {
            updateView();
        }
    }
}

void WorldClient::FileDialog::OnButtonBackDirectory(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    if (m_fileSystem->changeWorkingDirectoryTo(".."))
    {
        updateView();
    }
}

void WorldClient::FileDialog::OnButtonSelect(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    m_success = true;
    Close();
    if (m_callback)
        m_callback(this);
    m_callback = nullptr;
}

void WorldClient::FileDialog::OnButtonCancel(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    m_success = false;
    Close();
    if (m_callback)
        m_callback(this);
    m_callback = nullptr;
}

void WorldClient::FileDialog::updateDirectoryView(Noesis::TreeViewItem* parent, bool isWorkingDirLeaf)
{
    if (m_directoryView)
    {
        auto items = parent ? parent->GetItems() : m_directoryView->GetItems();
        if (items->Count() || (parent && parent->GetTag()))
            return;

        if (parent)
            parent->SetTag("");

        irr::Ptr<irr::io::IFileList> fileList = *m_fileSystem->createFileList(nullptr, true);

        std::string cm = m_currentDirectory;
        if (fileList->isIgnoreCase())
            cm = System::String(m_currentDirectory).ToLower();

        //Noesis::
        //Noesis::ItemCollection* list = new Noesis::ItemCollection;
        //CollectionView

        for (uint32_t i = 0; i < fileList->getFileCount(); ++i)
        {
            bool isDirectory = fileList->isDirectory(i);
            if (!isDirectory)
                break;

            const auto& cs = fileList->getFileName(i);
            if (cs == "." || cs == "..")
                continue;

            const auto& fs = fileList->getFullFileName(i);

            //auto currentItem = items->FindNodeName(fs.c_str());
            //if (currentItem)
            //    continue;

            Noesis::TreeViewItem* treeitem = new Noesis::TreeViewItem;

            treeitem->SetName(fs.c_str());
            treeitem->SetHeader(cs.c_str());
            treeitem->SetToolTip(fs.c_str());
            treeitem->IsVisibleChanged() += Noesis::MakeDelegate(this, &SelfClass::OnTreeViewItem);

            bool isCurrentDirPath = isWorkingDirLeaf;
            if (cm.size() > fs.size() && !std::memcmp(cm.c_str(), fs.c_str(), fs.size()))
            {
                isCurrentDirPath = true;
                treeitem->SetIsExpanded(true);
                if (cm.size() == fs.size())
                    treeitem->SetIsSelected(true);
            }

            if ((!parent || isCurrentDirPath) && m_fileSystem->changeWorkingDirectoryTo(fs.c_str()))
                updateDirectoryView(treeitem, isCurrentDirPath);

            items->Add(treeitem);
        }
    }
}

void WorldClient::FileDialog::updateView()
{
    if (!m_initialized)
        return;

    if (m_fileListView)
    {
        m_fileList = *m_fileSystem->createFileList();
        auto items = m_fileListView->GetItems();
        items->Clear();

        std::regex Filter(m_filter, std::regex::flag_type::icase | std::regex::flag_type::optimize);

        for (uint32_t i = 0; i < m_fileList->getFileCount(); ++i)
        {
            bool isDirectory = m_fileList->isDirectory(i);
            std::string cs = ((irr::core::stringc)m_fileList->getFileName(i)).c_str();
            if (cs == ".")
                continue;

            if (!isDirectory && !m_filter.empty() && !std::regex_match(cs, Filter))
                continue;

            FileListElement* entry = new FileListElement(i, cs.c_str(), isDirectory ? "Directory" : "File",  m_fileList->getFileSize(i));
            items->Add(entry);
        }
    }

    if (m_filePath)
    {
        if (m_currentDirectory != ((irr::core::stringc)m_fileSystem->getWorkingDirectory()).c_str() || !strlen(m_filePath->GetText()))
        {
            m_currentDirectory = ((irr::core::stringc)m_fileSystem->getWorkingDirectory()).c_str();
            m_filePath->SetText(m_currentDirectory.c_str());
        }
    }
}

void WorldClient::FileDialog::updateFilteredDataSource()
{
    //if (m_fileSystem && m_baseArchive)
    //{
    //    if (!m_filteredArchive)
    //        m_filteredArchive = irr::MakePtr<WMDL::DataSource::ContentArchive>();
    //
    //    auto datasource = static_cast<WMDL::DataSource::ContentArchive*>(m_filteredArchive.GetPtr());
    //    datasource->clear();
    //    datasource->reserve(1000000);
    //
    //    if (m_filter.size() > 1)
    //    {
    //        std::set<std::string> directories;
    //        //std::regex regFilter(m_filter, std::regex::flag_type::optimize | std::regex::flag_type::icase);
    //
    //        const irr::io::IFileList* merge = m_baseArchive->getFileList();
    //
    //        try
    //        {
    //            boost::regex regFilter(m_filter, boost::regex::icase | boost::regex::optimize);
    //
    //            for (uint32_t j = 0; j < merge->getFileCount(); ++j)
    //            {
    //                if (merge->isDirectory(j) || !boost::regex_match(merge->getFullFileName(j).c_str(), regFilter))
    //                    continue;
    //
    //                datasource->addItem(merge->getFullFileName(j), merge->getFileOffset(j), merge->getFileSize(j), merge->isDirectory(j), 0);
    //            }
    //        }
    //        catch (...)
    //        {
    //            m_fileSystem->removeFileArchive(m_filteredArchive);
    //            m_fileSystem->removeFileArchive(m_baseArchive);
    //            m_baseArchive->grab();
    //            m_fileSystem->addFileArchive(m_baseArchive);
    //            return;
    //        }
    //
    //        merge = datasource->getFileList();
    //
    //        for (uint32_t j = 0; j < merge->getFileCount(); ++j)
    //        {
    //            irr::io::path _path;
    //            irr::core::splitFilename(merge->getFullFileName(j), &_path);
    //            if (!_path.empty())
    //                directories.insert(_path.c_str());
    //        }
    //
    //        for (const std::string& dir : directories)
    //        {
    //            size_t pos = 0;
    //            while ((pos = dir.find('/', pos)) != std::string::npos && (pos + 1) < dir.size())
    //            {
    //                auto s = dir.substr(0, ++pos);
    //                directories.insert(std::move(s));
    //            }
    //        }
    //
    //        for (const std::string& dir : directories)
    //        {
    //            datasource->addItem(dir.c_str(), 0, 0, true);
    //        }
    //
    //        datasource->sort();
    //
    //        m_fileSystem->removeFileArchive(m_baseArchive);
    //        m_fileSystem->removeFileArchive(m_filteredArchive);
    //        m_filteredArchive->grab();
    //        m_fileSystem->addFileArchive(m_filteredArchive);
    //    }
    //    else if (m_filteredArchive)
    //    {
    //        m_fileSystem->removeFileArchive(m_filteredArchive);
    //        m_fileSystem->removeFileArchive(m_baseArchive);
    //        m_baseArchive->grab();
    //        m_fileSystem->addFileArchive(m_baseArchive);
    //    }
    //}
}

bool WorldClient::FileDialog::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
    NS_REGISTER_UI_EVENT(Noesis::Button,   Click,               OnButtonBackDirectory)
    NS_REGISTER_UI_EVENT(Noesis::Button,   Click,               OnButtonSelect)
    NS_REGISTER_UI_EVENT(Noesis::Button,   Click,               OnButtonCancel)
    NS_REGISTER_UI_EVENT(Noesis::ListView, SelectionChanged,    OnSelectListViewItem)
    NS_REGISTER_UI_EVENT(Noesis::ListView, MouseDoubleClick,    OnMouseListViewItem)
    NS_REGISTER_UI_EVENT(Noesis::ListView, KeyUp,               OnKeyUpListViewItem)
    NS_REGISTER_UI_EVENT(Noesis::TreeView, SelectedItemChanged, OnSelectTreeViewItem)
    NS_REGISTER_UI_EVENT(Noesis::TextBox,  TextChanged,         OnDirectoryTextChanged)
    NS_REGISTER_UI_EVENT(Noesis::TextBox,  KeyUp,               OnKeyUpFilter)
	return true;
}
