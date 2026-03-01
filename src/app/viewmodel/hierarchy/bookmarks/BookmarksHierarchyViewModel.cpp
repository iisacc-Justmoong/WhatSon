#include "BookmarksHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "viewmodel/hierarchy/common/HierarchyViewModelSupport.hpp"

namespace
{
    constexpr auto kScope = "bookmarks.viewmodel";

    QString bookmarkListTitle(const LibraryNoteRecord& note)
    {
        const QString title = note.title.trimmed();
        if (!title.isEmpty())
        {
            return title;
        }

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        return QStringLiteral("Untitled Note");
    }

    QString bookmarkListSummary(const LibraryNoteRecord& note)
    {
        const QString bodySummary = note.bodySummary.trimmed();
        if (!bodySummary.isEmpty())
        {
            return bodySummary;
        }
        return QStringLiteral("No contents");
    }

    QString bookmarkListFolders(const LibraryNoteRecord& note)
    {
        QStringList folders;
        folders.reserve(note.folders.size());
        for (const QString& folder : note.folders)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                folders.push_back(trimmed);
            }
        }

        if (folders.isEmpty())
        {
            return QStringLiteral("No Folder");
        }
        return folders.join(QStringLiteral(", "));
    }

    QString bookmarkColorHexForNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }
}

BookmarksHierarchyViewModel::BookmarksHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::trace(QString::fromLatin1(kScope), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &FlatHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    QObject::connect(
        &m_noteListModel,
        &LibraryNoteListModel::itemCountChanged,
        this,
        [this](int)
        {
            updateNoteItemCount();
        });
    setBookmarkIds({});
}

BookmarksHierarchyViewModel::~BookmarksHierarchyViewModel() = default;

FlatHierarchyModel* BookmarksHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* BookmarksHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

int BookmarksHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int BookmarksHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

int BookmarksHierarchyViewModel::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool BookmarksHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString BookmarksHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void BookmarksHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::Support::clampSelectionIndex(index, m_itemModel.rowCount());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void BookmarksHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    m_items = WhatSon::Hierarchy::Support::parseDepthItems(depthItems, QStringLiteral("Bookmark"));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
}

QVariantList BookmarksHierarchyViewModel::depthItems() const
{
    return WhatSon::Hierarchy::Support::serializeDepthItems(m_items);
}

QString BookmarksHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool BookmarksHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled())
    {
        return false;
    }
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }
    if (WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(index)))
    {
        return false;
    }

    if (!WhatSon::Hierarchy::Support::renameFlatItem(&m_items, index, displayName))
    {
        return false;
    }

    syncDomainStoreFromItems();
    syncModel();
    return true;
}

void BookmarksHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
        return;
    }

    const int insertIndex = WhatSon::Hierarchy::Support::createFlatFolder(
        &m_items, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        return;
    }

    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(insertIndex);
}

void BookmarksHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int nextSelectedIndex = WhatSon::Hierarchy::Support::deleteFlatSubtree(&m_items, m_selectedIndex);
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(nextSelectedIndex);
}

void BookmarksHierarchyViewModel::setBookmarkIds(QStringList bookmarkIds)
{
    m_bookmarkIds = WhatSon::Hierarchy::Support::sanitizeStringList(std::move(bookmarkIds));
    m_store.setBookmarkIds(m_bookmarkIds);
    m_items = WhatSon::Hierarchy::Support::buildBucketItems(
        QStringLiteral("Bookmarks"),
        m_bookmarkIds,
        QStringLiteral("Bookmark"));
    m_createdFolderSequence = WhatSon::Hierarchy::Support::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
}

QStringList BookmarksHierarchyViewModel::bookmarkIds() const
{
    return m_bookmarkIds;
}

bool BookmarksHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool BookmarksHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool BookmarksHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !WhatSon::Hierarchy::Support::isBucketHeaderItem(m_items.at(m_selectedIndex));
}

bool BookmarksHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    LibraryAll libraryAll;
    QString indexError;
    if (!libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        updateLoadState(false, indexError);
        return false;
    }

    const QVector<LibraryNoteRecord>& notes = libraryAll.notes();
    QStringList bookmarkLabels;
    bookmarkLabels.reserve(notes.size());

    QVector<LibraryNoteListItem> bookmarkListItems;
    bookmarkListItems.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        if (!note.bookmarked)
        {
            continue;
        }

        const QString title = bookmarkListTitle(note);
        if (!title.isEmpty())
        {
            bookmarkLabels.push_back(title);
        }

        LibraryNoteListItem item;
        item.noteId = note.noteId.trimmed();
        item.titleText = title;
        item.summaryText = bookmarkListSummary(note);
        item.foldersText = bookmarkListFolders(note);
        item.bookmarked = true;
        item.bookmarkColorHex = bookmarkColorHexForNote(note);
        item.highlighted = false;
        bookmarkListItems.push_back(std::move(item));
    }

    setBookmarkIds(bookmarkLabels);
    m_noteListModel.setItems(std::move(bookmarkListItems));
    updateNoteItemCount();

    WhatSon::Debug::trace(
        QString::fromLatin1(kScope),
        QStringLiteral("loadFromWshub"),
        QStringLiteral("path=%1 source=wsnhead count=%2")
        .arg(wshubPath)
        .arg(m_bookmarkIds.size()));
    updateLoadState(true);
    return true;
}

void BookmarksHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void BookmarksHierarchyViewModel::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void BookmarksHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    const QString normalizedError = succeeded ? QString() : errorMessage;
    const bool shouldEmit = (m_loadSucceeded != succeeded) || (m_lastLoadError != normalizedError);
    m_loadSucceeded = succeeded;
    m_lastLoadError = normalizedError;
    if (shouldEmit)
    {
        emit loadStateChanged();
    }
}

void BookmarksHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
}

void BookmarksHierarchyViewModel::syncDomainStoreFromItems()
{
    m_bookmarkIds = WhatSon::Hierarchy::Support::extractDomainLabelsFromItems(m_items);
    m_store.setBookmarkIds(m_bookmarkIds);
}
