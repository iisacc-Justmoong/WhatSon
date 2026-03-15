#include "BookmarksHierarchyViewModel.hpp"

#include "calendar/SystemCalendarStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"

#include <QFileInfo>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kScope = "bookmarks.viewmodel";
    constexpr auto kDraftFolderLabel = "Draft";
    constexpr int kMaxNoteListSummaryLines = 5;

    QStringList bookmarkListFolders(const LibraryNoteRecord& note);

    QString truncateToMaxLines(const QString& value, int maxLines)
    {
        if (maxLines <= 0)
        {
            return {};
        }

        const QStringList lines = value.split(QLatin1Char('\n'));
        if (lines.size() <= maxLines)
        {
            return value;
        }

        QStringList truncated;
        truncated.reserve(maxLines);
        for (int index = 0; index < maxLines; ++index)
        {
            truncated.push_back(lines.at(index));
        }
        return truncated.join(QLatin1Char('\n'));
    }

    QString bookmarkLabelText(const LibraryNoteRecord& note)
    {
        QString primary = note.bodyFirstLine.trimmed();
        if (!primary.isEmpty())
        {
            return primary;
        }

        primary = note.noteId.trimmed();
        if (!primary.isEmpty())
        {
            return primary;
        }

        if (!note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
            if (!primary.isEmpty())
            {
                return primary;
            }
        }

        return {};
    }

    QString bookmarkPrimaryText(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return bookmarkLabelText(note);
    }

    QString bookmarkSearchableText(const LibraryNoteRecord& note)
    {
        QStringList parts;

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            parts.push_back(noteId);
        }

        const QString firstLine = note.bodyFirstLine.trimmed();
        if (!firstLine.isEmpty())
        {
            parts.push_back(firstLine);
        }

        const QString bodyPlainText = note.bodyPlainText.trimmed();
        if (!bodyPlainText.isEmpty())
        {
            parts.push_back(bodyPlainText);
        }

        const QStringList folderLabels = bookmarkListFolders(note);
        for (const QString& folder : folderLabels)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        return parts.join(QLatin1Char('\n'));
    }

    QStringList bookmarkListFolders(const LibraryNoteRecord& note)
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
        folders.removeDuplicates();
        if (folders.isEmpty())
        {
            folders.push_back(QString::fromLatin1(kDraftFolderLabel));
        }
        return folders;
    }

    QStringList bookmarkListTags(const LibraryNoteRecord& note)
    {
        QStringList tags;
        tags.reserve(note.tags.size());
        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                tags.push_back(trimmed);
            }
        }
        tags.removeDuplicates();
        return tags;
    }

    QString bookmarkColorHexForNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

    int indexOfBookmarkedNoteById(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < notes.size(); ++index)
        {
            if (notes.at(index).noteId.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }

        return -1;
    }

    QString colorHexForLabel(const QString& label)
    {
        const QString normalized = label.trimmed().toCaseFolded();
        for (const auto& colorDef : WhatSon::Bookmarks::kBookmarkColorDefinitions)
        {
            if (normalized == QString::fromLatin1(colorDef.name))
            {
                return QString::fromLatin1(colorDef.hex);
            }
        }
        return {};
    }

    QVector<BookmarksHierarchyItem> buildColorFolderItems()
    {
        QVector<BookmarksHierarchyItem> items;
        items.reserve(static_cast<int>(WhatSon::Bookmarks::kBookmarkColorDefinitions.size()));

        for (const auto& colorDef : WhatSon::Bookmarks::kBookmarkColorDefinitions)
        {
            BookmarksHierarchyItem item;
            item.depth = 0;
            item.accent = false;
            item.expanded = false;
            item.label = QString::fromLatin1(colorDef.name);
            item.showChevron = false;
            items.push_back(std::move(item));
        }

        return items;
    }
} // namespace

BookmarksHierarchyViewModel::BookmarksHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &BookmarksHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    QObject::connect(
        &m_noteListModel,
        &BookmarksNoteListModel::itemCountChanged,
        this,
        [this](int)
        {
            updateNoteItemCount();
        });

    rebuildColorFolders();
    refreshNoteListForSelection();
}

BookmarksHierarchyViewModel::~BookmarksHierarchyViewModel() = default;

BookmarksHierarchyModel* BookmarksHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

BookmarksNoteListModel* BookmarksHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

void BookmarksHierarchyViewModel::setSystemCalendarStore(SystemCalendarStore* store)
{
    if (m_systemCalendarStore == store)
    {
        return;
    }

    if (m_systemCalendarStoreChangedConnection)
    {
        QObject::disconnect(m_systemCalendarStoreChangedConnection);
    }

    m_systemCalendarStore = store;
    if (m_systemCalendarStore)
    {
        m_systemCalendarStoreChangedConnection = QObject::connect(
            m_systemCalendarStore,
            &SystemCalendarStore::systemInfoChanged,
            this,
            [this]()
            {
                refreshNoteListForSelection();
            });
    }
    else
    {
        m_systemCalendarStoreChangedConnection = {};
    }

    refreshNoteListForSelection();
}

SystemCalendarStore* BookmarksHierarchyViewModel::systemCalendarStore() const noexcept
{
    return m_systemCalendarStore;
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
    const int maxIndex = m_itemModel.rowCount() - 1;
    int clamped = index;
    if (maxIndex < 0)
    {
        clamped = -1;
    }
    else
    {
        clamped = std::clamp(index, -1, maxIndex);
    }

    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void BookmarksHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    Q_UNUSED(depthItems);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 noteCount=%2").arg(m_items.size()).arg(
                                  m_noteListModel.rowCount()));
}

QVariantList BookmarksHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList BookmarksHierarchyViewModel::depthItems() const
{
    QVariantList serialized;
    serialized.reserve(m_items.size());

    for (int index = 0; index < m_items.size(); ++index)
    {
        const BookmarksHierarchyItem& item = m_items.at(index);
        serialized.push_back(QVariantMap{
            {QStringLiteral("itemId"), index},
            {QStringLiteral("key"), QStringLiteral("bookmarks:%1").arg(index)},
            {QStringLiteral("label"), item.label},
            {QStringLiteral("depth"), item.depth},
            {QStringLiteral("accent"), item.accent},
            {QStringLiteral("expanded"), item.expanded},
            {QStringLiteral("showChevron"), item.showChevron}
        });
    }

    return serialized;
}

QString BookmarksHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool BookmarksHierarchyViewModel::canRenameItem(int index) const
{
    Q_UNUSED(index);
    return false;
}

bool BookmarksHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
    return false;
}

void BookmarksHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

void BookmarksHierarchyViewModel::deleteSelectedFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

bool BookmarksHierarchyViewModel::removeNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0)
    {
        return false;
    }

    const bool removedCurrentVisibleNote = m_noteListModel.currentNoteId().trimmed() == normalizedNoteId;
    const int removedCurrentVisibleIndex = removedCurrentVisibleNote ? m_noteListModel.currentIndex() : -1;

    m_bookmarkedNotes.removeAt(noteIndex);
    refreshNoteListForSelection();
    if (removedCurrentVisibleNote)
    {
        const int nextIndex = m_noteListModel.items().isEmpty()
                                  ? -1
                                  : std::min(
                                      removedCurrentVisibleIndex,
                                      static_cast<int>(m_noteListModel.items().size()) - 1);
        m_noteListModel.setCurrentIndex(nextIndex);
    }

    return true;
}

bool BookmarksHierarchyViewModel::saveBodyTextForNote(const QString& noteId, const QString& text)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0)
    {
        return false;
    }

    LibraryNoteRecord& note = m_bookmarkedNotes[noteIndex];
    QString normalizedBodyText;
    QString lastModifiedAt;
    QString saveError;
    if (!WhatSon::NoteBodyPersistence::persistBodyPlainText(
        note.noteId,
        note.noteDirectoryPath,
        note.noteHeaderPath,
        text,
        &normalizedBodyText,
        &lastModifiedAt,
        &saveError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("saveCurrentBodyText.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, saveError));
        return false;
    }

    note.bodyPlainText = normalizedBodyText;
    note.bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(normalizedBodyText);
    if (!lastModifiedAt.isEmpty())
    {
        note.lastModifiedAt = lastModifiedAt;
    }

    refreshNoteListForSelection();
    return true;
}

bool BookmarksHierarchyViewModel::saveCurrentBodyText(const QString& text)
{
    return saveBodyTextForNote(m_noteListModel.currentNoteId(), text);
}

bool BookmarksHierarchyViewModel::renameEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::viewOptionsEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    LibraryAll libraryAll;
    QString indexError;
    if (!libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.index"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        updateLoadState(false, indexError);
        return false;
    }

    const QVector<LibraryNoteRecord>& notes = libraryAll.notes();

    QVector<LibraryNoteRecord> bookmarkedNotes;
    bookmarkedNotes.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        if (!note.bookmarked)
        {
            continue;
        }

        bookmarkedNotes.push_back(note);
    }

    m_bookmarkedNotes = std::move(bookmarkedNotes);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 source=wsnhead count=%2")
                              .arg(wshubPath)
                              .arg(m_bookmarkedNotes.size()));
    updateLoadState(true);
    return true;
}

void BookmarksHierarchyViewModel::applyRuntimeSnapshot(
    QVector<LibraryNoteRecord> bookmarkedNotes,
    bool loadSucceeded,
    QString errorMessage)
{
    if (!loadSucceeded)
    {
        m_bookmarkedNotes.clear();
        rebuildColorFolders();
        setSelectedIndex(-1);
        refreshNoteListForSelection();
        updateLoadState(false, errorMessage);
        return;
    }

    m_bookmarkedNotes = std::move(bookmarkedNotes);
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();
    updateLoadState(true);
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

void BookmarksHierarchyViewModel::rebuildColorFolders()
{
    m_items = buildColorFolderItems();
    syncModel();
}

BookmarksNoteListItem BookmarksHierarchyViewModel::buildBookmarksListItem(const LibraryNoteRecord& note) const
{
    BookmarksNoteListItem item;
    item.id = note.noteId.trimmed();
    item.primaryText = bookmarkPrimaryText(note);
    item.searchableText = bookmarkSearchableText(note);
    item.bodyText = note.bodyPlainText;
    item.image = note.bodyHasResource;
    item.imageSource = note.bodyFirstResourceThumbnailUrl;
    item.displayDate = m_systemCalendarStore
                           ? m_systemCalendarStore->formatNoteDate(note.lastModifiedAt, note.createdAt)
                           : SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
    item.folders = bookmarkListFolders(note);
    item.tags = bookmarkListTags(note);
    item.bookmarked = true;
    item.bookmarkColor = bookmarkColorHexForNote(note);
    return item;
}

void BookmarksHierarchyViewModel::refreshNoteListForSelection()
{
    const QString selectedColorHex = colorHexForLabel(selectedColorLabel());
    if (selectedColorHex.isEmpty())
    {
        QVector<BookmarksNoteListItem> allItems;
        allItems.reserve(m_bookmarkedNotes.size());
        for (const LibraryNoteRecord& note : std::as_const(m_bookmarkedNotes))
        {
            allItems.push_back(buildBookmarksListItem(note));
        }
        m_noteListModel.setItems(std::move(allItems));
        updateNoteItemCount();
        return;
    }

    QVector<BookmarksNoteListItem> filtered;
    filtered.reserve(m_bookmarkedNotes.size());
    for (const LibraryNoteRecord& note : std::as_const(m_bookmarkedNotes))
    {
        const BookmarksNoteListItem item = buildBookmarksListItem(note);
        if (item.bookmarkColor.compare(selectedColorHex, Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        filtered.push_back(item);
    }

    m_noteListModel.setItems(filtered);
    updateNoteItemCount();
}

QString BookmarksHierarchyViewModel::selectedColorLabel() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return {};
    }

    return m_items.at(m_selectedIndex).label.trimmed();
}

void BookmarksHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}
