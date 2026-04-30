#include "app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.hpp"

#include "app/models/calendar/ISystemCalendarStore.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/calendar/SystemCalendarStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hierarchy/library/WhatSonLibraryIndexedState.hpp"
#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/statistic/WhatSonNoteFileStatSupport.hpp"
#include "app/models/file/note/WhatSonNoteFolderBindingRepository.hpp"
#include "app/models/file/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kScope = "bookmarks.controller";
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

    int noteCountForBookmarkLabel(const QVector<LibraryNoteRecord>& notes, const QString& label)
    {
        const QString selectedColorHex = colorHexForLabel(label);
        if (selectedColorHex.isEmpty())
        {
            return notes.size();
        }

        int noteCount = 0;
        for (const LibraryNoteRecord& note : notes)
        {
            if (bookmarkColorHexForNote(note).compare(selectedColorHex, Qt::CaseInsensitive) == 0)
            {
                ++noteCount;
            }
        }
        return noteCount;
    }

    QString bookmarkHierarchyIconSource(const QString& colorValue)
    {
        const QString colorHex = WhatSon::Bookmarks::bookmarkColorToHex(colorValue);
        if (colorHex.isEmpty())
        {
            return {};
        }

        const QString svg = QStringLiteral(
                                "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16' fill='none'>"
                                "<path fill='%1' d='M4 1.5h8a.5.5 0 0 1 .5.5v11l-4.5-3-4.5 3V2a.5.5 0 0 1 .5-.5Z'/>"
                                "</svg>")
                                .arg(colorHex);
        return QStringLiteral("data:image/svg+xml,%1").arg(QString::fromLatin1(QUrl::toPercentEncoding(svg)));
    }

    void syncNoteRecordFromDocument(LibraryNoteRecord* note, const WhatSonLocalNoteDocument& document)
    {
        if (note == nullptr)
        {
            return;
        }

        const LibraryNoteRecord updatedRecord = document.toLibraryNoteRecord();
        if (!updatedRecord.noteId.trimmed().isEmpty())
        {
            note->noteId = updatedRecord.noteId;
        }
        note->storageKind = updatedRecord.storageKind;
        note->bodyPlainText = updatedRecord.bodyPlainText;
        note->bodySourceText = updatedRecord.bodySourceText;
        note->bodyFirstLine = updatedRecord.bodyFirstLine;
        note->bodyHasResource = updatedRecord.bodyHasResource;
        note->bodyFirstResourceThumbnailUrl = updatedRecord.bodyFirstResourceThumbnailUrl;
        note->createdAt = updatedRecord.createdAt;
        note->lastModifiedAt = updatedRecord.lastModifiedAt;
        note->author = updatedRecord.author;
        note->modifiedBy = updatedRecord.modifiedBy;
        note->project = updatedRecord.project;
        note->folders = updatedRecord.folders;
        note->folderUuids = updatedRecord.folderUuids;
        note->bookmarkColors = updatedRecord.bookmarkColors;
        note->tags = updatedRecord.tags;
        note->progress = updatedRecord.progress;
        note->bookmarked = updatedRecord.bookmarked;
        note->preset = updatedRecord.preset;
        if (!updatedRecord.noteDirectoryPath.isEmpty())
        {
            note->noteDirectoryPath = updatedRecord.noteDirectoryPath;
        }
        note->noteHeaderPath = updatedRecord.noteHeaderPath;
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
            item.label = QString::fromLatin1(colorDef.displayName);
            item.iconSource = bookmarkHierarchyIconSource(QString::fromLatin1(colorDef.hex));
            item.showChevron = false;
            items.push_back(std::move(item));
        }

        return items;
    }
} // namespace

BookmarksHierarchyController::BookmarksHierarchyController(QObject* parent)
    : IHierarchyController(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
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

BookmarksHierarchyController::~BookmarksHierarchyController() = default;

BookmarksHierarchyModel* BookmarksHierarchyController::itemModel() noexcept
{
    return &m_itemModel;
}

BookmarksNoteListModel* BookmarksHierarchyController::noteListModel() noexcept
{
    return &m_noteListModel;
}

void BookmarksHierarchyController::setSystemCalendarStore(ISystemCalendarStore* store)
{
    if (store != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::Controller,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("BookmarksHierarchyController::setSystemCalendarStore")))
    {
        return;
    }

    if (store == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("BookmarksHierarchyController::setSystemCalendarStore")))
    {
        return;
    }

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
            &ISystemCalendarStore::systemInfoChanged,
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

ISystemCalendarStore* BookmarksHierarchyController::systemCalendarStore() const noexcept
{
    return m_systemCalendarStore;
}

int BookmarksHierarchyController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int BookmarksHierarchyController::itemCount() const noexcept
{
    return m_itemCount;
}

int BookmarksHierarchyController::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool BookmarksHierarchyController::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString BookmarksHierarchyController::lastLoadError() const
{
    return m_lastLoadError;
}

void BookmarksHierarchyController::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(
        index,
        m_itemModel.rowCount());

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

void BookmarksHierarchyController::setDepthItems(const QVariantList& depthItems)
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

QVariantList BookmarksHierarchyController::hierarchyModel() const
{
    return depthItems();
}

QVariantList BookmarksHierarchyController::depthItems() const
{
    QVariantList serialized;
    serialized.reserve(m_items.size());

    for (int index = 0; index < m_items.size(); ++index)
    {
        const BookmarksHierarchyItem& item = m_items.at(index);
        const int noteCount = std::max(0, noteCountForBookmarkLabel(m_bookmarkedNotes, item.label));
        serialized.push_back(QVariantMap{
            {QStringLiteral("itemId"), index},
            {QStringLiteral("key"), QStringLiteral("bookmarks:%1").arg(index)},
            {QStringLiteral("label"), item.label},
            {QStringLiteral("depth"), item.depth},
            {QStringLiteral("accent"), item.accent},
            {QStringLiteral("expanded"), item.expanded},
            {QStringLiteral("showChevron"), item.showChevron},
            {QStringLiteral("iconName"), bookmarksHierarchyIconName(item)},
            {QStringLiteral("iconSource"), item.iconSource},
            {QStringLiteral("count"), noteCount}
        });
    }

    return serialized;
}

QString BookmarksHierarchyController::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool BookmarksHierarchyController::canRenameItem(int index) const
{
    Q_UNUSED(index);
    return false;
}

bool BookmarksHierarchyController::renameItem(int index, const QString& displayName)
{
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
    return false;
}

void BookmarksHierarchyController::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

void BookmarksHierarchyController::deleteSelectedFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=bookmarks hierarchy is read-only"));
}

bool BookmarksHierarchyController::removeNoteById(const QString& noteId)
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
    emit hierarchyModelChanged();
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

bool BookmarksHierarchyController::saveBodyTextForNote(const QString& noteId, const QString& text)
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

    const LibraryNoteRecord& note = m_bookmarkedNotes.at(noteIndex);
    QString normalizedBodyText;
    QString normalizedBodySourceText;
    QString lastModifiedAt;
    QString saveError;
    if (!WhatSon::NoteBodyPersistence::persistBodyPlainText(
        note.noteId,
        note.noteDirectoryPath,
        note.noteHeaderPath,
        text,
        &normalizedBodyText,
        &normalizedBodySourceText,
        &lastModifiedAt,
        &saveError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("saveCurrentBodyText.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, saveError));
        return false;
    }

    return applyPersistedBodyStateForNote(
        normalizedNoteId,
        normalizedBodyText,
        normalizedBodySourceText,
        lastModifiedAt);
}

bool BookmarksHierarchyController::saveCurrentBodyText(const QString& text)
{
    return saveBodyTextForNote(m_noteListModel.currentNoteId(), text);
}

bool BookmarksHierarchyController::applyPersistedBodyStateForNote(
    const QString& noteId,
    const QString& normalizedBodyText,
    const QString& normalizedBodySourceText,
    const QString& lastModifiedAt)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_bookmarkedNotes.size())
    {
        return false;
    }

    LibraryNoteRecord& note = m_bookmarkedNotes[noteIndex];
    note.bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(normalizedBodyText);
    note.bodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(normalizedBodySourceText);
    if (note.bodySourceText.isEmpty())
    {
        note.bodySourceText = note.bodyPlainText;
    }
    note.bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(note.bodyPlainText);
    if (!lastModifiedAt.trimmed().isEmpty())
    {
        note.lastModifiedAt = lastModifiedAt.trimmed();
    }

    refreshNoteListForSelection();
    emit hubFilesystemMutated();
    return true;
}

bool BookmarksHierarchyController::requestTrackedStatisticsRefreshForNote(
    const QString& noteId,
    const bool incrementOpenCount)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QString noteDirectoryPath = noteDirectoryPathForNoteId(normalizedNoteId);
    if (noteDirectoryPath.isEmpty())
    {
        return false;
    }

    QString statRefreshError;
    if (!WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
            normalizedNoteId,
            noteDirectoryPath,
            incrementOpenCount,
            &statRefreshError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("requestTrackedStatisticsRefreshForNote.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, statRefreshError));
        return false;
    }

    return reloadNoteMetadataForNoteId(normalizedNoteId);
}

QString BookmarksHierarchyController::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_bookmarkedNotes.size())
    {
        return {};
    }

    return m_bookmarkedNotes.at(noteIndex).noteDirectoryPath.trimmed();
}

QString BookmarksHierarchyController::noteBodySourceTextForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_bookmarkedNotes.size())
    {
        return {};
    }

    const LibraryNoteRecord& note = m_bookmarkedNotes.at(noteIndex);
    if (!note.bodySourceText.isEmpty())
    {
        return note.bodySourceText;
    }

    return note.bodyPlainText;
}

bool BookmarksHierarchyController::reloadNoteMetadataForNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfBookmarkedNoteById(m_bookmarkedNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_bookmarkedNotes.size())
    {
        return false;
    }

    WhatSonNoteFolderBindingRepository noteRepository;
    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    if (!noteRepository.readDocument(m_bookmarkedNotes.at(noteIndex), &noteDocument, &ioError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("reloadNoteMetadataForNoteId.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, ioError));
        return false;
    }

    syncNoteRecordFromDocument(&m_bookmarkedNotes[noteIndex], noteDocument);
    if (!m_bookmarkedNotes.at(noteIndex).bookmarked)
    {
        m_bookmarkedNotes.removeAt(noteIndex);
    }
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

bool BookmarksHierarchyController::renameEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyController::createFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyController::deleteFolderEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyController::viewOptionsEnabled() const noexcept
{
    return false;
}

bool BookmarksHierarchyController::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_wshubPath.clear();
    const QString normalizedWshubPath = QDir::cleanPath(wshubPath.trimmed());

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(normalizedWshubPath));
    WhatSonLibraryIndexedState indexedState;
    QString indexError;
    if (!indexedState.indexFromWshub(normalizedWshubPath, &indexError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.index"),
                                  QStringLiteral("path=%1 reason=%2").arg(normalizedWshubPath, indexError));
        updateLoadState(false, indexError);
        return false;
    }

    m_wshubPath = normalizedWshubPath;
    m_bookmarkedNotes = WhatSonLibraryIndexedState::collectBookmarkedNotes(indexedState.allNotes());
    rebuildColorFolders();
    setSelectedIndex(-1);
    refreshNoteListForSelection();

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 source=wsnhead count=%2")
                              .arg(normalizedWshubPath)
                              .arg(m_bookmarkedNotes.size()));
    updateLoadState(true);
    return true;
}

void BookmarksHierarchyController::applyRuntimeSnapshot(
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

void BookmarksHierarchyController::requestControllerHook()
{
    if (m_wshubPath.trimmed().isEmpty())
    {
        emit controllerHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromWshubPath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit controllerHookRequested();
        return;
    }

    updateLoadState(true);
    emit controllerHookRequested();
}

bool BookmarksHierarchyController::reloadFromWshubPath(QString* errorMessage)
{
    const QString normalizedWshubPath = QDir::cleanPath(m_wshubPath.trimmed());
    if (normalizedWshubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    const QString preservedColorLabel = selectedColorLabel();

    WhatSonLibraryIndexedState indexedState;
    QString indexError;
    if (!indexedState.indexFromWshub(normalizedWshubPath, &indexError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        return false;
    }

    m_bookmarkedNotes = WhatSonLibraryIndexedState::collectBookmarkedNotes(indexedState.allNotes());
    rebuildColorFolders();

    int restoredSelectionIndex = -1;
    if (!preservedColorLabel.trimmed().isEmpty())
    {
        for (int index = 0; index < m_items.size(); ++index)
        {
            if (m_items.at(index).label.compare(preservedColorLabel, Qt::CaseInsensitive) == 0)
            {
                restoredSelectionIndex = index;
                break;
            }
        }
    }

    const int previousSelectedIndex = m_selectedIndex;
    setSelectedIndex(restoredSelectionIndex);
    if (m_selectedIndex == previousSelectedIndex)
    {
        refreshNoteListForSelection();
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void BookmarksHierarchyController::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void BookmarksHierarchyController::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void BookmarksHierarchyController::updateLoadState(bool succeeded, QString errorMessage)
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

void BookmarksHierarchyController::rebuildColorFolders()
{
    m_items = buildColorFolderItems();
    syncModel();
}

BookmarksNoteListItem BookmarksHierarchyController::buildBookmarksListItem(const LibraryNoteRecord& note) const
{
    BookmarksNoteListItem item;
    item.id = note.noteId.trimmed();
    item.primaryText = bookmarkPrimaryText(note);
    item.searchableText = bookmarkSearchableText(note);
    item.bodyText = !note.bodySourceText.isEmpty()
        ? note.bodySourceText
        : note.bodyPlainText;
    item.createdAt = note.createdAt;
    item.lastModifiedAt = note.lastModifiedAt;
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

void BookmarksHierarchyController::refreshNoteListForSelection()
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

QString BookmarksHierarchyController::selectedColorLabel() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return {};
    }

    return m_items.at(m_selectedIndex).label.trimmed();
}

void BookmarksHierarchyController::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}
