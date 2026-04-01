#include "ProgressHierarchyViewModel.hpp"

#include "calendar/SystemCalendarStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteFolderBindingRepository.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "progress.viewmodel";
    constexpr int kMaxNoteListSummaryLines = 5;

    QStringList defaultProgressStates()
    {
        return {
            QStringLiteral("Ready"),
            QStringLiteral("Pending"),
            QStringLiteral("InProgress"),
            QStringLiteral("Done")
        };
    }

    QStringList sanitizeProgressStatesOrDefault(QStringList values)
    {
        values = WhatSon::Hierarchy::ProgressSupport::sanitizeStringList(std::move(values));
        if (values.isEmpty())
        {
            return defaultProgressStates();
        }
        return values;
    }

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

    QString notePrimaryText(const LibraryNoteRecord& note)
    {
        const QString firstLine = note.bodyFirstLine.trimmed();
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!firstLine.isEmpty())
        {
            if (bodyPlainText.isEmpty())
            {
                return firstLine;
            }

            if (!bodyPlainText.startsWith(firstLine))
            {
                return firstLine + QLatin1Char('\n') + bodyPlainText;
            }
        }

        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }

        return note.noteId.trimmed();
    }

    QStringList noteListFolders(const LibraryNoteRecord& note)
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
            folders.push_back(QStringLiteral("Draft"));
        }
        return folders;
    }

    QStringList noteListTags(const LibraryNoteRecord& note)
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

    QString noteSearchableText(const LibraryNoteRecord& note, const QStringList& folderLabels)
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

    QString bookmarkColorHexFromNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

    int noteCountForProgressValue(const QVector<LibraryNoteRecord>& notes, int progressValue)
    {
        int noteCount = 0;
        for (const LibraryNoteRecord& note : notes)
        {
            if (note.progress == progressValue)
            {
                ++noteCount;
            }
        }
        return noteCount;
    }

    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
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

    QString resolveWshubPathFromProgressFile(const QString& progressFilePath)
    {
        QFileInfo info(progressFilePath.trimmed());
        QString currentPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
        while (!currentPath.isEmpty())
        {
            const QFileInfo currentInfo(currentPath);
            if (currentInfo.fileName().endsWith(QStringLiteral(".wshub")) && currentInfo.isDir())
            {
                return currentInfo.absoluteFilePath();
            }

            const QDir dir(currentPath);
            const QString parentPath = dir.absolutePath() == dir.rootPath() ? QString() : dir.filePath(QStringLiteral(".."));
            const QString normalizedParentPath = QFileInfo(parentPath).absoluteFilePath();
            if (normalizedParentPath.isEmpty() || normalizedParentPath == currentPath)
            {
                break;
            }
            currentPath = normalizedParentPath;
        }

        return {};
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
} // namespace

ProgressHierarchyViewModel::ProgressHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &ProgressHierarchyModel::itemCountChanged,
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
    setProgressState(0, {
                         QStringLiteral("Ready"),
                         QStringLiteral("Pending"),
                         QStringLiteral("InProgress"),
                         QStringLiteral("Done")
                     });
}

ProgressHierarchyViewModel::~ProgressHierarchyViewModel() = default;

ProgressHierarchyModel* ProgressHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* ProgressHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

int ProgressHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int ProgressHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

int ProgressHierarchyViewModel::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool ProgressHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString ProgressHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void ProgressHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::ProgressSupport::clampSelectionIndex(index, m_itemModel.rowCount());
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

void ProgressHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant count=%1").arg(depthItems.size()));
    Q_UNUSED(depthItems);
}

QVariantList ProgressHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList ProgressHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::ProgressSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        const int noteCount = std::max(0, noteCountForProgressValue(m_allNotes, index));
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), QStringLiteral("progress:%1").arg(index));
        entry.insert(QStringLiteral("progressValue"), index);
        entry.insert(QStringLiteral("count"), noteCount);
        serialized[index] = entry;
    }
    return serialized;
}

QString ProgressHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ProgressHierarchyViewModel::canRenameItem(int index) const
{
    Q_UNUSED(index);
    return false;
}

bool ProgressHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant index=%1 label=%2").arg(index).arg(
                                  displayName));
    Q_UNUSED(index);
    Q_UNUSED(displayName);
    return false;
}

bool ProgressHierarchyViewModel::setItemExpanded(int index, bool expanded)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    if (!m_items.at(index).showChevron)
    {
        return false;
    }

    if (m_items.at(index).expanded == expanded)
    {
        return true;
    }

    m_items[index].expanded = expanded;
    syncModel();
    return true;
}

void ProgressHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant"));
    return;
}

void ProgressHierarchyViewModel::deleteSelectedFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.rejected"),
                              QStringLiteral("reason=progress hierarchy is constant"));
    return;
}

void ProgressHierarchyViewModel::setProgressState(int progressValue, QStringList progressStates)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProgressState.begin"),
                              QStringLiteral("value=%1 rawCount=%2").arg(progressValue).arg(progressStates.size()));
    m_progressStates = sanitizeProgressStatesOrDefault(std::move(progressStates));
    rebuildItems();
    const int maxProgressValue = std::max(0, static_cast<int>(m_items.size()) - 1);
    m_progressValue = std::clamp(progressValue, 0, maxProgressValue);
    syncProgressStore();
    m_createdFolderSequence = WhatSon::Hierarchy::ProgressSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    const int nextSelectedIndex = WhatSon::Hierarchy::ProgressSupport::clampSelectionIndex(m_selectedIndex, m_itemModel.rowCount());
    if (m_selectedIndex != nextSelectedIndex)
    {
        m_selectedIndex = nextSelectedIndex;
        emit selectedIndexChanged();
    }
    refreshNoteListForSelection();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProgressState.success"),
                              QStringLiteral("value=%1 states=%2 itemCount=%3").arg(m_progressValue).arg(
                                  m_progressStates.size()).arg(m_items.size()));
}

int ProgressHierarchyViewModel::progressValue() const noexcept
{
    return m_progressValue;
}

QStringList ProgressHierarchyViewModel::progressStates() const
{
    return m_progressStates;
}

bool ProgressHierarchyViewModel::renameEnabled() const noexcept
{
    return false;
}

bool ProgressHierarchyViewModel::createFolderEnabled() const noexcept
{
    return false;
}

bool ProgressHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return false;
}

bool ProgressHierarchyViewModel::saveBodyTextForNote(const QString& noteId, const QString& text)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfNoteRecordById(m_allNotes, normalizedNoteId);
    if (noteIndex < 0)
    {
        return false;
    }

    LibraryNoteRecord& note = m_allNotes[noteIndex];
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

    note.bodyPlainText = normalizedBodyText;
    note.bodySourceText = normalizedBodySourceText;
    note.bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(normalizedBodyText);
    if (!lastModifiedAt.isEmpty())
    {
        note.lastModifiedAt = lastModifiedAt;
    }

    refreshNoteListForSelection();
    emit hubFilesystemMutated();
    return true;
}

bool ProgressHierarchyViewModel::saveCurrentBodyText(const QString& text)
{
    return saveBodyTextForNote(m_noteListModel.currentNoteId(), text);
}

QString ProgressHierarchyViewModel::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const int noteIndex = indexOfNoteRecordById(m_allNotes, noteId);
    if (noteIndex < 0 || noteIndex >= m_allNotes.size())
    {
        return {};
    }

    return m_allNotes.at(noteIndex).noteDirectoryPath.trimmed();
}

bool ProgressHierarchyViewModel::reloadNoteMetadataForNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfNoteRecordById(m_allNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_allNotes.size())
    {
        return false;
    }

    WhatSonNoteFolderBindingRepository noteRepository;
    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    if (!noteRepository.readDocument(m_allNotes.at(noteIndex), &noteDocument, &ioError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("reloadNoteMetadataForNoteId.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, ioError));
        return false;
    }

    syncNoteRecordFromDocument(&m_allNotes[noteIndex], noteDocument);
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

bool ProgressHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_progressFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProgressSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    WhatSonProgressHierarchyParser parser;

    bool fileFound = false;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Progress.wsprogress"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        fileFound = true;
        m_progressFilePath = filePath;

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProgressSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.read"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, readError));
            updateLoadState(false, readError);
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.parse"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, parseError));
            updateLoadState(false, parseError);
            return false;
        }
        break;
    }

    if (m_progressFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_progressFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Progress.wsprogress"));
    }

    if (!fileFound)
    {
        QString parseError;
        if (!parser.parse(QString(), &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.defaultParse"),
                                      QStringLiteral("path=%1 reason=%2").arg(wshubPath, parseError));
            updateLoadState(false, parseError);
            return false;
        }
    }

    setProgressState(m_store.progressValue(), m_store.progressStates());

    QString noteLoadError;
    if (!refreshIndexedNotesFromWshub(wshubPath, &noteLoadError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = noteLoadError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.index"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, noteLoadError));
        updateLoadState(false, noteLoadError);
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 fileFound=%2 value=%3 states=%4")
                              .arg(wshubPath)
                              .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_progressValue)
                              .arg(m_progressStates.size()));

    updateLoadState(true);
    return true;
}

void ProgressHierarchyViewModel::applyRuntimeSnapshot(
    int progressValue,
    QStringList progressStates,
    QString progressFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_progressFilePath = progressFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    setProgressState(progressValue, std::move(progressStates));
    QString noteLoadError;
    if (!refreshIndexedNotesFromProgressFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        return;
    }
    updateLoadState(true);
}

void ProgressHierarchyViewModel::requestViewModelHook()
{
    if (m_progressFilePath.trimmed().isEmpty())
    {
        emit viewModelHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromProgressFilePath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit viewModelHookRequested();
        return;
    }

    updateLoadState(true);
    emit viewModelHookRequested();
}

bool ProgressHierarchyViewModel::reloadFromProgressFilePath(QString* errorMessage)
{
    const QString normalizedFilePath = m_progressFilePath.trimmed();
    if (normalizedFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    WhatSonProgressHierarchyParser parser;
    WhatSonProgressHierarchyStore refreshedStore;

    if (QFileInfo(normalizedFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProgressSupport::readUtf8File(normalizedFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }
    }
    else
    {
        QString parseError;
        if (!parser.parse(QString(), &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }
    }

    setProgressState(refreshedStore.progressValue(), refreshedStore.progressStates());
    QString noteLoadError;
    if (!refreshIndexedNotesFromProgressFilePath(&noteLoadError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = noteLoadError;
        }
        return false;
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void ProgressHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void ProgressHierarchyViewModel::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }

    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void ProgressHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

LibraryNoteListItem ProgressHierarchyViewModel::buildNoteListItem(const LibraryNoteRecord& note) const
{
    const QStringList folderLabels = noteListFolders(note);

    LibraryNoteListItem item;
    item.id = note.noteId.trimmed();
    item.primaryText = notePrimaryText(note);
    item.searchableText = noteSearchableText(note, folderLabels);
    item.bodyText = note.bodySourceText.isEmpty() ? note.bodyPlainText : note.bodySourceText;
    item.createdAt = note.createdAt;
    item.lastModifiedAt = note.lastModifiedAt;
    item.image = note.bodyHasResource;
    item.imageSource = note.bodyFirstResourceThumbnailUrl;
    item.displayDate = SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
    item.folders = folderLabels;
    item.tags = noteListTags(note);
    item.bookmarked = note.bookmarked;
    item.bookmarkColor = bookmarkColorHexFromNote(note);
    return item;
}

void ProgressHierarchyViewModel::refreshNoteListForSelection()
{
    const int selectedProgressValue = selectedProgressFilterValue();

    QVector<LibraryNoteListItem> items;
    items.reserve(m_allNotes.size());
    for (const LibraryNoteRecord& note : std::as_const(m_allNotes))
    {
        if (selectedProgressValue >= 0 && note.progress != selectedProgressValue)
        {
            continue;
        }

        items.push_back(buildNoteListItem(note));
    }

    m_noteListModel.setItems(std::move(items));
    updateNoteItemCount();
}

bool ProgressHierarchyViewModel::refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage)
{
    LibraryAll libraryAll;
    if (!libraryAll.indexFromWshub(wshubPath, errorMessage))
    {
        m_allNotes.clear();
        m_noteListModel.setItems({});
        updateNoteItemCount();
        emit hierarchyModelChanged();
        return false;
    }

    m_allNotes = libraryAll.notes();
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

bool ProgressHierarchyViewModel::refreshIndexedNotesFromProgressFilePath(QString* errorMessage)
{
    const QString wshubPath = resolveWshubPathFromProgressFile(m_progressFilePath);
    if (wshubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve .wshub path from Progress.wsprogress.");
        }
        m_allNotes.clear();
        m_noteListModel.setItems({});
        updateNoteItemCount();
        emit hierarchyModelChanged();
        return false;
    }

    return refreshIndexedNotesFromWshub(wshubPath, errorMessage);
}

int ProgressHierarchyViewModel::selectedProgressFilterValue() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return -1;
    }

    return m_selectedIndex;
}

void ProgressHierarchyViewModel::rebuildItems()
{
    const QVector<ProgressHierarchyItem> previousItems = m_items;
    m_items = WhatSon::Hierarchy::ProgressSupport::buildSupportedTypeItems(m_progressStates);

    for (int index = 0; index < m_items.size() && index < previousItems.size(); ++index)
    {
        if (m_items.at(index).label == previousItems.at(index).label
            && m_items.at(index).showChevron == previousItems.at(index).showChevron)
        {
            m_items[index].expanded = previousItems.at(index).expanded;
        }
    }
}

void ProgressHierarchyViewModel::syncProgressStore()
{
    m_store.setProgressValue(m_progressValue);
    m_store.setProgressStates(m_progressStates);
}

void ProgressHierarchyViewModel::syncProgressStatesFromItems()
{
    m_progressStates.clear();
    for (int index = 0; index < m_items.size(); ++index)
    {
        const QString label = m_items.at(index).label.trimmed();
        if (label.isEmpty() || m_progressStates.contains(label))
        {
            continue;
        }
        m_progressStates.push_back(label);
    }
    if (m_progressStates.isEmpty())
    {
        m_progressStates = defaultProgressStates();
    }
    if (m_progressValue >= m_progressStates.size())
    {
        m_progressValue = std::max(0, static_cast<int>(m_progressStates.size()) - 1);
    }
}

void ProgressHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}
