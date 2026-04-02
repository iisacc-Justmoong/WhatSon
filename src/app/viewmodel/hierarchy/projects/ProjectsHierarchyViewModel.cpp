#include "ProjectsHierarchyViewModel.hpp"

#include "calendar/SystemCalendarStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteFolderBindingRepository.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModelSupport.hpp"
#include "viewmodel/sidebar/SidebarHierarchyLvrsSupport.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "projects.viewmodel";
    constexpr int kMaxNoteListSummaryLines = 5;

    QVector<WhatSonFolderDepthEntry> projectEntriesFromItems(const QVector<ProjectsHierarchyItem>& items)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(items.size());

        for (const ProjectsHierarchyItem& item : items)
        {
            const QString label = item.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }
            if (item.accent && item.depth == 0)
            {
                continue;
            }

            WhatSonFolderDepthEntry entry;
            entry.id = label;
            entry.label = label;
            entry.depth = 0;
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    QVector<ProjectsHierarchyItem> itemsFromProjectEntries(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        QVector<ProjectsHierarchyItem> items;
        items.reserve(entries.size());

        for (const WhatSonFolderDepthEntry& entry : entries)
        {
            const QString label = entry.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }

            ProjectsHierarchyItem item;
            item.depth = 0;
            item.label = label;
            item.accent = false;
            item.expanded = false;
            item.showChevron = false;
            items.push_back(std::move(item));
        }

        WhatSon::Hierarchy::ProjectsSupport::applyChevronByDepth(&items);
        return items;
    }

    QString normalizedProjectKeySegment(const QString& label, int index)
    {
        const QString normalizedLabel = label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }
        return QStringLiteral("item:%1").arg(index);
    }

    QString projectsHierarchyItemKey(const QVector<ProjectsHierarchyItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedProjectKeySegment(items.at(index).label, index));

        int expectedDepth = items.at(index).depth;
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const ProjectsHierarchyItem& candidate = items.at(cursor);
            if (candidate.depth != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedProjectKeySegment(candidate.label, cursor));
            expectedDepth = candidate.depth;
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    int selectedProjectIndexForKey(const QVector<ProjectsHierarchyItem>& items, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (projectsHierarchyItemKey(items, index) == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }

    bool folderDepthEntriesEqual(
        const QVector<WhatSonFolderDepthEntry>& lhs,
        const QVector<WhatSonFolderDepthEntry>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            const WhatSonFolderDepthEntry& left = lhs.at(index);
            const WhatSonFolderDepthEntry& right = rhs.at(index);
            if (left.id.trimmed() != right.id.trimmed()
                || left.label.trimmed() != right.label.trimmed()
                || std::max(0, left.depth) != std::max(0, right.depth)
                || left.uuid.trimmed() != right.uuid.trimmed())
            {
                return false;
            }
        }

        return true;
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

        const QString project = note.project.trimmed();
        if (!project.isEmpty())
        {
            parts.push_back(project);
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

    int noteCountForProjectLabel(const QVector<LibraryNoteRecord>& notes, const QString& projectLabel)
    {
        const QString normalizedProjectLabel = projectLabel.trimmed();
        if (normalizedProjectLabel.isEmpty())
        {
            return 0;
        }

        int noteCount = 0;
        for (const LibraryNoteRecord& note : notes)
        {
            if (note.project.trimmed().compare(normalizedProjectLabel, Qt::CaseInsensitive) == 0)
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

    QString resolveCanonicalHeaderPathForProjects(const LibraryNoteRecord& note)
    {
        QString normalizedDirectoryPath = QDir::cleanPath(note.noteDirectoryPath.trimmed());
        const QString normalizedHeaderPath = QDir::cleanPath(note.noteHeaderPath.trimmed());
        if (normalizedDirectoryPath.isEmpty() && !normalizedHeaderPath.isEmpty())
        {
            normalizedDirectoryPath = QFileInfo(normalizedHeaderPath).absolutePath();
        }
        if (normalizedDirectoryPath.isEmpty())
        {
            return normalizedHeaderPath;
        }

        const QDir noteDirectory(normalizedDirectoryPath);
        if (!noteDirectory.exists())
        {
            return normalizedHeaderPath;
        }

        const QString noteStem = QFileInfo(normalizedDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDirectory.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDirectory.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDirectory.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }

            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        return normalizedHeaderPath;
    }

    QString resolveCanonicalNoteDirectoryPathForProjects(const LibraryNoteRecord& note)
    {
        const QString normalizedDirectoryPath = QDir::cleanPath(note.noteDirectoryPath.trimmed());
        if (!normalizedDirectoryPath.isEmpty())
        {
            return normalizedDirectoryPath;
        }

        const QString canonicalHeaderPath = resolveCanonicalHeaderPathForProjects(note);
        if (!canonicalHeaderPath.isEmpty())
        {
            const QString headerDirectoryPath = QFileInfo(canonicalHeaderPath).absolutePath().trimmed();
            if (!headerDirectoryPath.isEmpty())
            {
                return QDir::cleanPath(headerDirectoryPath);
            }
        }

        const QString indexedHeaderPath = QDir::cleanPath(note.noteHeaderPath.trimmed());
        if (!indexedHeaderPath.isEmpty())
        {
            const QString indexedHeaderDirectoryPath = QFileInfo(indexedHeaderPath).absolutePath().trimmed();
            if (!indexedHeaderDirectoryPath.isEmpty())
            {
                return QDir::cleanPath(indexedHeaderDirectoryPath);
            }
        }

        return {};
    }

    void synchronizeIndexedProjectLabelsFromHeaders(QVector<LibraryNoteRecord>* notes)
    {
        if (notes == nullptr)
        {
            return;
        }

        WhatSonNoteFolderBindingRepository noteRepository;
        for (int index = 0; index < notes->size(); ++index)
        {
            LibraryNoteRecord& note = (*notes)[index];
            WhatSonLocalNoteDocument noteDocument;
            QString ioError;
            const QString normalizedNoteId = note.noteId.trimmed();
            const QString preferredHeaderPath = resolveCanonicalHeaderPathForProjects(note);
            const QString normalizedDirectoryPath = resolveCanonicalNoteDirectoryPathForProjects(note);

            bool loaded = false;
            if (!normalizedNoteId.isEmpty() && !preferredHeaderPath.isEmpty())
            {
                loaded = noteRepository.readDocument(
                    normalizedNoteId,
                    normalizedDirectoryPath,
                    preferredHeaderPath,
                    &noteDocument,
                    &ioError);
            }

            if (!loaded)
            {
                loaded = noteRepository.readDocument(note, &noteDocument, &ioError);
            }

            if (loaded)
            {
                syncNoteRecordFromDocument(&note, noteDocument);
                continue;
            }

            // Projects membership is header-authoritative.
            // If we cannot load a header for this index-only row, exclude it from Projects projection.
            note.project.clear();
        }
    }

    QString resolveWshubPathFromProjectsFile(const QString& projectsFilePath)
    {
        QFileInfo info(projectsFilePath.trimmed());
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

    enum class FolderDropPlacement
    {
        Before,
        After,
        Child,
        RootTop
    };

    struct FolderMoveOperation final
    {
        int sourceEndIndex = -1;
        int sourceCount = 0;
        int normalizedInsertIndex = -1;
        int newBaseDepth = 0;
    };

    bool isProtectedRootItem(const ProjectsHierarchyItem& item)
    {
        return item.accent && item.depth == 0;
    }

    int firstEditableInsertIndex(const QVector<ProjectsHierarchyItem>& items)
    {
        int index = 0;
        while (index < items.size() && isProtectedRootItem(items.at(index)))
        {
            ++index;
        }
        return index;
    }

    bool isEditableFolderItem(const QVector<ProjectsHierarchyItem>& items, int index)
    {
        return index >= 0 && index < items.size() && !isProtectedRootItem(items.at(index));
    }

    int subtreeEndIndexExclusive(const QVector<ProjectsHierarchyItem>& items, int startIndex)
    {
        if (startIndex < 0 || startIndex >= items.size())
        {
            return startIndex;
        }

        const int baseDepth = items.at(startIndex).depth;
        int endIndex = startIndex + 1;
        while (endIndex < items.size() && items.at(endIndex).depth > baseDepth)
        {
            ++endIndex;
        }
        return endIndex;
    }

    bool indexInsideSubtree(int index, int subtreeStart, int subtreeEndExclusive)
    {
        return index >= subtreeStart && index < subtreeEndExclusive;
    }

    bool resolveFolderMoveOperation(
        const QVector<ProjectsHierarchyItem>& items,
        int sourceIndex,
        int targetIndex,
        FolderDropPlacement placement,
        FolderMoveOperation* outOperation = nullptr)
    {
        if (!isEditableFolderItem(items, sourceIndex))
        {
            return false;
        }

        const int sourceEndIndex = subtreeEndIndexExclusive(items, sourceIndex);
        const int sourceCount = sourceEndIndex - sourceIndex;
        if (sourceCount <= 0)
        {
            return false;
        }

        int rawInsertIndex = firstEditableInsertIndex(items);
        int newBaseDepth = 0;

        switch (placement)
        {
        case FolderDropPlacement::RootTop:
            rawInsertIndex = firstEditableInsertIndex(items);
            newBaseDepth = 0;
            break;
        case FolderDropPlacement::Before:
            if (!isEditableFolderItem(items, targetIndex) || sourceIndex == targetIndex)
            {
                return false;
            }
            if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
            {
                return false;
            }
            rawInsertIndex = targetIndex;
            newBaseDepth = items.at(targetIndex).depth;
            break;
        case FolderDropPlacement::After:
            if (!isEditableFolderItem(items, targetIndex) || sourceIndex == targetIndex)
            {
                return false;
            }
            if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
            {
                return false;
            }
            rawInsertIndex = subtreeEndIndexExclusive(items, targetIndex);
            newBaseDepth = items.at(targetIndex).depth;
            break;
        case FolderDropPlacement::Child:
            return false;
        }

        int normalizedInsertIndex = rawInsertIndex;
        if (normalizedInsertIndex > sourceIndex)
        {
            normalizedInsertIndex -= sourceCount;
        }
        const int maxInsertIndex = std::max(0, static_cast<int>(items.size()) - sourceCount);
        normalizedInsertIndex = std::clamp(normalizedInsertIndex, 0, maxInsertIndex);

        if (normalizedInsertIndex == sourceIndex && newBaseDepth == items.at(sourceIndex).depth)
        {
            return false;
        }

        if (outOperation != nullptr)
        {
            outOperation->sourceEndIndex = sourceEndIndex;
            outOperation->sourceCount = sourceCount;
            outOperation->normalizedInsertIndex = normalizedInsertIndex;
            outOperation->newBaseDepth = newBaseDepth;
        }
        return true;
    }

    QVector<ProjectsHierarchyItem> stageFolderMoveItems(
        const QVector<ProjectsHierarchyItem>& items,
        int sourceIndex,
        const FolderMoveOperation& operation)
    {
        const int depthDelta = operation.newBaseDepth - items.at(sourceIndex).depth;

        QVector<ProjectsHierarchyItem> movedItems;
        movedItems.reserve(operation.sourceCount);
        for (int index = sourceIndex; index < operation.sourceEndIndex; ++index)
        {
            ProjectsHierarchyItem item = items.at(index);
            item.depth = std::max(0, item.depth + depthDelta);
            movedItems.push_back(std::move(item));
        }

        QVector<ProjectsHierarchyItem> stagedItems = items;
        stagedItems.remove(sourceIndex, operation.sourceCount);
        for (int offset = 0; offset < movedItems.size(); ++offset)
        {
            stagedItems.insert(operation.normalizedInsertIndex + offset, std::move(movedItems[offset]));
        }

        WhatSon::Hierarchy::ProjectsSupport::applyChevronByDepth(&stagedItems);
        return stagedItems;
    }
}

ProjectsHierarchyViewModel::ProjectsHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &ProjectsHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setProjectNames({});
}

ProjectsHierarchyViewModel::~ProjectsHierarchyViewModel() = default;

ProjectsHierarchyModel* ProjectsHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* ProjectsHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

bool ProjectsHierarchyViewModel::supportsHierarchyNodeReorder() const noexcept
{
    return true;
}

bool ProjectsHierarchyViewModel::reloadNoteMetadataForNoteId(const QString& noteId)
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

    const LibraryNoteRecord indexedNote = m_allNotes.at(noteIndex);
    const QString previousProjectLabel = indexedNote.project.trimmed();
    const QString preferredHeaderPath = resolveCanonicalHeaderPathForProjects(indexedNote);
    const QString resolvedDirectoryPath = resolveCanonicalNoteDirectoryPathForProjects(indexedNote);

    WhatSonNoteFolderBindingRepository noteRepository;
    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    bool loaded = false;
    if (!normalizedNoteId.isEmpty() && !preferredHeaderPath.isEmpty())
    {
        loaded = noteRepository.readDocument(
            normalizedNoteId,
            resolvedDirectoryPath,
            preferredHeaderPath,
            &noteDocument,
            &ioError);
    }
    if (!loaded)
    {
        loaded = noteRepository.readDocument(indexedNote, &noteDocument, &ioError);
    }
    if (!loaded)
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("reloadNoteMetadataForNoteId.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, ioError));
        // Drop stale projection membership immediately when note metadata can no longer be read.
        m_allNotes[noteIndex].project.clear();
        const bool projectMembershipChanged =
            previousProjectLabel.compare(m_allNotes.at(noteIndex).project.trimmed(), Qt::CaseInsensitive) != 0;
        refreshNoteListForSelection();
        emit hierarchyModelChanged();
        if (projectMembershipChanged && !m_projectsFilePath.trimmed().isEmpty())
        {
            QString refreshError;
            if (!refreshIndexedNotesFromProjectsFilePath(&refreshError))
            {
                WhatSon::Debug::traceSelf(
                    this,
                    QString::fromLatin1(kScope),
                    QStringLiteral("reloadNoteMetadataForNoteId.projectionRefreshFailed"),
                    QStringLiteral("noteId=%1 reason=%2").arg(normalizedNoteId, refreshError));
            }
        }
        return false;
    }

    syncNoteRecordFromDocument(&m_allNotes[noteIndex], noteDocument);
    const bool projectMembershipChanged =
        previousProjectLabel.compare(m_allNotes.at(noteIndex).project.trimmed(), Qt::CaseInsensitive) != 0;
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    if (projectMembershipChanged && !m_projectsFilePath.trimmed().isEmpty())
    {
        QString refreshError;
        if (!refreshIndexedNotesFromProjectsFilePath(&refreshError))
        {
            WhatSon::Debug::traceSelf(
                this,
                QString::fromLatin1(kScope),
                QStringLiteral("reloadNoteMetadataForNoteId.projectionRefreshFailed"),
                QStringLiteral("noteId=%1 reason=%2").arg(normalizedNoteId, refreshError));
        }
    }
    return true;
}

QString ProjectsHierarchyViewModel::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const int noteIndex = indexOfNoteRecordById(m_allNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_allNotes.size())
    {
        return {};
    }

    return resolveCanonicalNoteDirectoryPathForProjects(m_allNotes.at(noteIndex));
}

int ProjectsHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int ProjectsHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool ProjectsHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString ProjectsHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void ProjectsHierarchyViewModel::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::ProjectsSupport::clampSelectionIndex(index, m_itemModel.rowCount());
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

void ProjectsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = itemsFromProjectEntries(
        projectEntriesFromItems(
            WhatSon::Hierarchy::ProjectsSupport::parseDepthItems(depthItems, QStringLiteral("Project"))));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList ProjectsHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList ProjectsHierarchyViewModel::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::ProjectsSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size() && index < m_items.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        const int noteCount = std::max(0, noteCountForProjectLabel(m_allNotes, m_items.at(index).label));
        entry.insert(QStringLiteral("draggable"), canMoveFolder(index));
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), projectsHierarchyItemKey(m_items, index));
        entry.insert(QStringLiteral("count"), noteCount);
        serialized[index] = entry;
    }
    return serialized;
}

QString ProjectsHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool ProjectsHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const ProjectsHierarchyItem& item = m_items.at(index);
    return !(item.accent && item.depth == 0);
}

bool ProjectsHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    QVector<ProjectsHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::ProjectsSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=support rejected index=%1 label=%2").arg(index).arg(
                                      displayName));
        return false;
    }

    WhatSonProjectsHierarchyStore stagedStore = m_store;
    stagedStore.setFolderEntries(projectEntriesFromItems(stagedItems));

    if (!m_projectsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_projectsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_projectsFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_projectNames = m_store.projectNames();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(displayName).arg(
                                  m_items.size()));
    return true;
}

bool ProjectsHierarchyViewModel::setItemExpanded(int index, bool expanded)
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

bool ProjectsHierarchyViewModel::setAllItemsExpanded(bool expanded)
{
    bool changed = false;
    for (ProjectsHierarchyItem& item : m_items)
    {
        if (!item.showChevron || item.expanded == expanded)
        {
            continue;
        }

        item.expanded = expanded;
        changed = true;
    }

    if (!changed)
    {
        return true;
    }

    syncModel();
    return true;
}

void ProjectsHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    if (!createFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=createFolderEnabled false"));
        return;
    }

    QVector<ProjectsHierarchyItem> stagedItems = m_items;
    const int insertIndex = WhatSon::Hierarchy::ProjectsSupport::createHierarchyFolder(
        &stagedItems, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=insertIndex invalid"));
        return;
    }

    if (!commitHierarchyUpdate(std::move(stagedItems), insertIndex))
    {
        return;
    }
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.success"),
                              QStringLiteral("insertIndex=%1 itemCount=%2").arg(insertIndex).arg(m_items.size()));
}

void ProjectsHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (!deleteFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=deleteFolderEnabled false selectedIndex=%1").arg(startIndex));
        return;
    }

    QVector<ProjectsHierarchyItem> stagedItems = m_items;
    const int nextSelectedIndex =
        WhatSon::Hierarchy::ProjectsSupport::deleteHierarchySubtree(&stagedItems, m_selectedIndex);
    if (!commitHierarchyUpdate(std::move(stagedItems), nextSelectedIndex))
    {
        return;
    }
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 nextIndex=%2 itemCount=%3").arg(startIndex).arg(
                                  nextSelectedIndex).arg(m_items.size()));
}

bool ProjectsHierarchyViewModel::canMoveFolder(int index) const
{
    return isEditableFolderItem(m_items, index);
}

bool ProjectsHierarchyViewModel::canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        nullptr);
}

bool ProjectsHierarchyViewModel::moveFolderBefore(int sourceIndex, int targetIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolderBefore.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("moveFolderBefore.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool ProjectsHierarchyViewModel::canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const
{
    if (asChild)
    {
        return false;
    }
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::After,
        nullptr);
}

bool ProjectsHierarchyViewModel::moveFolder(int sourceIndex, int targetIndex, bool asChild)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolder.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                              .arg(sourceIndex)
                              .arg(targetIndex)
                              .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));
    if (asChild)
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("moveFolder.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=1 reason=projects are flat")
                                  .arg(sourceIndex)
                                  .arg(targetIndex));
        return false;
    }

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::After,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("moveFolder.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                                  .arg(sourceIndex)
                                  .arg(targetIndex)
                                  .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool ProjectsHierarchyViewModel::canMoveFolderToRoot(int sourceIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        nullptr);
}

bool ProjectsHierarchyViewModel::moveFolderToRoot(int sourceIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolderToRoot.begin"),
                              QStringLiteral("sourceIndex=%1").arg(sourceIndex));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        &operation))
    {
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool ProjectsHierarchyViewModel::applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
{
    const QVector<WhatSon::Sidebar::Lvrs::FlatNode> flattened =
        WhatSon::Sidebar::Lvrs::flattenHierarchyNodes(hierarchyNodes);
    if (flattened.isEmpty())
    {
        return false;
    }

    QVector<ProjectsHierarchyItem> stagedItems;
    stagedItems.reserve(flattened.size());

    int selectedIndex = -1;
    const QString normalizedActiveKey = activeItemKey.trimmed();
    for (int flatIndex = 0; flatIndex < flattened.size(); ++flatIndex)
    {
        const WhatSon::Sidebar::Lvrs::FlatNode& node = flattened.at(flatIndex);
        if (node.key == normalizedActiveKey)
        {
            selectedIndex = flatIndex;
        }

        ProjectsHierarchyItem item;
        item.depth = 0;
        item.label = node.label.trimmed();
        item.accent = node.accent;
        item.expanded = false;
        item.showChevron = false;
        stagedItems.push_back(std::move(item));
    }

    WhatSon::Hierarchy::ProjectsSupport::applyChevronByDepth(&stagedItems);
    return commitHierarchyUpdate(std::move(stagedItems), selectedIndex);
}

void ProjectsHierarchyViewModel::setProjectNames(QStringList projectNames)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProjectNames.begin"),
                              QStringLiteral("rawCount=%1").arg(projectNames.size()));
    m_projectNames = WhatSon::Hierarchy::ProjectsSupport::sanitizeStringList(std::move(projectNames));
    m_store.setProjectNames(m_projectNames);
    m_items = WhatSon::Hierarchy::ProjectsSupport::buildBucketItems(
        QStringLiteral("Projects"),
        m_projectNames,
        QStringLiteral("Project"));
    m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setProjectNames.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_projectNames.size()).arg(
                                  m_items.size()));
}

QStringList ProjectsHierarchyViewModel::projectNames() const
{
    return m_projectNames;
}

bool ProjectsHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool ProjectsHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool ProjectsHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    const ProjectsHierarchyItem& selectedItem = m_items.at(m_selectedIndex);
    return !(selectedItem.accent && selectedItem.depth == 0);
}

bool ProjectsHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_projectsFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProjectsSupport::resolveContentsDirectories(
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

    QVector<WhatSonFolderDepthEntry> aggregatedEntries;
    bool fileFound = false;

    WhatSonProjectsHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("ProjectLists.wsproj"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        fileFound = true;
        if (m_projectsFilePath.isEmpty())
        {
            m_projectsFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProjectsSupport::readUtf8File(filePath, &rawText, &readError))
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
        WhatSonProjectsHierarchyStore parsedStore;
        if (!parser.parse(rawText, &parsedStore, &parseError))
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

        const QVector<WhatSonFolderDepthEntry> parsedEntries = parsedStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            aggregatedEntries.push_back(entry);
        }
    }

    if (m_projectsFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_projectsFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("ProjectLists.wsproj"));
    }

    if (aggregatedEntries.isEmpty())
    {
        setProjectNames({});
    }
    else
    {
        m_store.setFolderEntries(std::move(aggregatedEntries));
        m_projectNames = m_store.projectNames();
        m_items = itemsFromProjectEntries(m_store.folderEntries());
        m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
    }

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
                              QStringLiteral("path=%1 fileFound=%2 count=%3 entryCount=%4")
                              .arg(wshubPath)
                              .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_projectNames.size())
                              .arg(m_store.folderEntries().size()));

    if (WhatSon::Debug::isEnabled())
    {
        qWarning().noquote()
            << QStringLiteral("[projects:index] path=%1 count=%2 values=[%3]")
               .arg(wshubPath)
               .arg(m_projectNames.size())
               .arg(m_projectNames.join(QStringLiteral(", ")));
    }

    updateLoadState(true);
    return true;
}

void ProjectsHierarchyViewModel::applyRuntimeSnapshot(
    QVector<WhatSonFolderDepthEntry> projectEntries,
    QString projectsFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? projectsHierarchyItemKey(m_items, m_selectedIndex)
            : QString();
    m_projectsFilePath = projectsFilePath.trimmed();

    if (!loadSucceeded)
    {
        updateLoadState(false, std::move(errorMessage));
        return;
    }

    if (!folderDepthEntriesEqual(projectEntriesFromItems(m_items), projectEntries))
    {
        if (projectEntries.isEmpty())
        {
            m_projectNames.clear();
            m_store.setFolderEntries({});
            m_items.clear();
            m_createdFolderSequence = 1;
            syncModel();
            setSelectedIndex(-1);
        }
        else
        {
            m_store.setFolderEntries(projectEntries);
            m_projectNames = m_store.projectNames();
            m_items = itemsFromProjectEntries(m_store.folderEntries());
            m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
            syncModel();
            setSelectedIndex(selectedProjectIndexForKey(m_items, preservedSelectionKey));
        }
    }

    QString noteLoadError;
    if (!refreshIndexedNotesFromProjectsFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        return;
    }

    updateLoadState(true);
}

void ProjectsHierarchyViewModel::requestViewModelHook()
{
    if (m_projectsFilePath.trimmed().isEmpty())
    {
        emit viewModelHookRequested();
        return;
    }

    QString noteLoadError;
    if (!refreshIndexedNotesFromProjectsFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        emit viewModelHookRequested();
        return;
    }

    updateLoadState(true);
    emit viewModelHookRequested();
}

void ProjectsHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void ProjectsHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void ProjectsHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
    refreshNoteListForSelection();
}

bool ProjectsHierarchyViewModel::commitHierarchyUpdate(QVector<ProjectsHierarchyItem> stagedItems, int selectedIndex)
{
    WhatSonProjectsHierarchyStore stagedStore = m_store;
    stagedStore.setFolderEntries(projectEntriesFromItems(stagedItems));

    if (!m_projectsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_projectsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("commitHierarchyUpdate.writeFailed"),
                                      QStringLiteral("path=%1 reason=%2").arg(m_projectsFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_projectNames = m_store.projectNames();
    m_createdFolderSequence = WhatSon::Hierarchy::ProjectsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(selectedIndex);
    return true;
}

void ProjectsHierarchyViewModel::syncDomainStoreFromItems()
{
    m_store.setFolderEntries(projectEntriesFromItems(m_items));
    m_projectNames = m_store.projectNames();
}

LibraryNoteListItem ProjectsHierarchyViewModel::buildNoteListItem(const LibraryNoteRecord& note) const
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

void ProjectsHierarchyViewModel::refreshNoteListForSelection()
{
    // Keep project projection strictly aligned with live `.wsnhead` values.
    // This prevents stale index/cache labels from leaking into project buckets.
    synchronizeIndexedProjectLabelsFromHeaders(&m_allNotes);

    const QString selectedProject =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size()) ? m_items.at(m_selectedIndex).label.trimmed() : QString();

    QSet<QString> availableProjects;
    availableProjects.reserve(m_items.size());
    for (const ProjectsHierarchyItem& item : std::as_const(m_items))
    {
        if (item.accent && item.depth == 0)
        {
            continue;
        }

        const QString label = item.label.trimmed();
        if (label.isEmpty())
        {
            continue;
        }

        availableProjects.insert(label.toCaseFolded());
    }

    QVector<LibraryNoteListItem> items;
    items.reserve(m_allNotes.size());
    for (const LibraryNoteRecord& note : std::as_const(m_allNotes))
    {
        const QString projectLabel = note.project.trimmed();
        if (projectLabel.isEmpty())
        {
            continue;
        }

        const QString normalizedProjectLabel = projectLabel.toCaseFolded();
        if (!availableProjects.contains(normalizedProjectLabel))
        {
            continue;
        }

        if (!selectedProject.isEmpty() && projectLabel.compare(selectedProject, Qt::CaseInsensitive) != 0)
        {
            continue;
        }

        items.push_back(buildNoteListItem(note));
    }

    m_noteListModel.setItems(std::move(items));
}

bool ProjectsHierarchyViewModel::refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage)
{
    LibraryAll libraryAll;
    if (!libraryAll.indexFromWshub(wshubPath, errorMessage))
    {
        m_allNotes.clear();
        m_noteListModel.setItems({});
        emit hierarchyModelChanged();
        return false;
    }

    m_allNotes = libraryAll.notes();
    synchronizeIndexedProjectLabelsFromHeaders(&m_allNotes);
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

bool ProjectsHierarchyViewModel::refreshIndexedNotesFromProjectsFilePath(QString* errorMessage)
{
    const QString wshubPath = resolveWshubPathFromProjectsFile(m_projectsFilePath);
    if (wshubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve .wshub path from ProjectLists.wsproj.");
        }
        m_allNotes.clear();
        m_noteListModel.setItems({});
        emit hierarchyModelChanged();
        return false;
    }

    return refreshIndexedNotesFromWshub(wshubPath, errorMessage);
}
