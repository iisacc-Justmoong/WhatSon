#include "LibraryHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
    constexpr int kMaxNoteListSummaryLines = 5;
    constexpr auto kLibraryAllLabel = "All";
    constexpr auto kLibraryDraftLabel = "Draft";
    constexpr auto kLibraryTodayLabel = "Today";

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

    QString noteLabelText(const LibraryNoteRecord& note)
    {
        QString primary = note.bodyFirstLine.trimmed();
        if (primary.isEmpty())
        {
            primary = note.noteId.trimmed();
        }
        if (primary.isEmpty() && !note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
        }
        return primary;
    }

    QString notePrimaryText(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return noteLabelText(note);
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

    QDate parseNoteListDate(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QList<QString> formats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-dd")
        };

        for (const QString& format : formats)
        {
            const QDateTime dateTime = QDateTime::fromString(trimmed, format);
            if (dateTime.isValid())
            {
                return dateTime.date();
            }

            const QDate date = QDate::fromString(trimmed, format);
            if (date.isValid())
            {
                return date;
            }
        }

        const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
        if (isoDateTime.isValid())
        {
            return isoDateTime.date();
        }

        const QDateTime isoDateTimeWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoDateTimeWithMs.isValid())
        {
            return isoDateTimeWithMs.date();
        }

        return {};
    }

    QString noteListDisplayDate(const LibraryNoteRecord& note)
    {
        QDate date = parseNoteListDate(note.lastModifiedAt);
        if (!date.isValid())
        {
            date = parseNoteListDate(note.createdAt);
        }
        if (!date.isValid())
        {
            return {};
        }
        return date.toString(QStringLiteral("yyyy-MM-dd"));
    }

    QString bookmarkColorHexFromNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

    void applyChevronByDepth(QVector<LibraryHierarchyItem>* items)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            const int nextIndex = index + 1;
            const bool hasChild = nextIndex < items->size()
                && items->at(nextIndex).depth > items->at(index).depth;
            (*items)[index].showChevron = hasChild;
        }
    }

    bool isSystemBucketLabel(const QString& label)
    {
        const QString trimmed = label.trimmed();
        return trimmed == QLatin1String(kLibraryAllLabel)
            || trimmed == QLatin1String(kLibraryDraftLabel)
            || trimmed == QLatin1String(kLibraryTodayLabel);
    }

    bool isProtectedRootItem(const LibraryHierarchyItem& item)
    {
        return item.depth == 0 && item.accent;
    }

    bool isSystemBucketItem(const LibraryHierarchyItem& item)
    {
        return isProtectedRootItem(item) && isSystemBucketLabel(item.label);
    }

    LibraryHierarchyItem makeSystemBucketItem(const QString& label)
    {
        LibraryHierarchyItem item;
        item.depth = 0;
        item.accent = true;
        item.expanded = false;
        item.label = label;
        item.showChevron = false;
        return item;
    }

    QVector<LibraryHierarchyItem> prependSystemBuckets(QVector<LibraryHierarchyItem> items)
    {
        QVector<LibraryHierarchyItem> combined;
        combined.reserve(3 + items.size());
        combined.push_back(makeSystemBucketItem(QStringLiteral("All")));
        combined.push_back(makeSystemBucketItem(QStringLiteral("Draft")));
        combined.push_back(makeSystemBucketItem(QStringLiteral("Today")));

        for (LibraryHierarchyItem& item : items)
        {
            combined.push_back(std::move(item));
        }

        return combined;
    }

    QVector<LibraryHierarchyItem> buildFolderItems(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        QVector<LibraryHierarchyItem> items;
        items.reserve(entries.size());

        for (const WhatSonFolderDepthEntry& entry : entries)
        {
            LibraryHierarchyItem item;
            item.depth = std::max(0, entry.depth);
            item.label = entry.label.trimmed();
            item.accent = false;
            item.expanded = false;
            item.showChevron = true;

            if (item.label.isEmpty())
            {
                WhatSon::Debug::trace(
                    QStringLiteral("library.viewmodel"),
                    QStringLiteral("buildFolderItems.emptyLabelKept"));
            }

            items.push_back(std::move(item));
        }

        applyChevronByDepth(&items);
        return items;
    }

    QVector<WhatSonFolderDepthEntry> folderEntriesFromItems(const QVector<LibraryHierarchyItem>& items)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(items.size());
        QStringList pathStack;

        for (const LibraryHierarchyItem& item : items)
        {
            const QString label = item.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }
            if (isProtectedRootItem(item))
            {
                continue;
            }

            int depth = std::max(0, item.depth);
            if (depth > pathStack.size())
            {
                depth = pathStack.size();
            }
            while (pathStack.size() > depth)
            {
                pathStack.removeLast();
            }

            const QString parentPath = (depth > 0 && !pathStack.isEmpty()) ? pathStack.constLast() : QString();
            const QString id = parentPath.isEmpty() ? label : parentPath + QLatin1Char('/') + label;

            WhatSonFolderDepthEntry entry;
            entry.id = id;
            entry.label = label;
            entry.depth = depth;
            entries.push_back(std::move(entry));

            if (pathStack.size() <= depth)
            {
                pathStack.push_back(id);
            }
            else
            {
                pathStack[depth] = id;
                pathStack = pathStack.mid(0, depth + 1);
            }
        }

        return entries;
    }
} // namespace

LibraryHierarchyViewModel::LibraryHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
      , m_noteListModel(this)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.viewmodel"), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &LibraryHierarchyModel::itemCountChanged,
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
    syncModel();
    refreshNoteListForSelection();
}

LibraryHierarchyViewModel::~LibraryHierarchyViewModel() = default;

LibraryHierarchyModel* LibraryHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* LibraryHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

int LibraryHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int LibraryHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

int LibraryHierarchyViewModel::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool LibraryHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString LibraryHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void LibraryHierarchyViewModel::setSelectedIndex(int index)
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
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void LibraryHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));

    if (depthItems.isEmpty() && m_runtimeIndexLoaded)
    {
        if (m_foldersHierarchyLoaded)
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("setDepthItems.keepFoldersHierarchy"),
                                      QStringLiteral("folderCount=%1").arg(m_items.size()));
            refreshNoteListForSelection();
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("setDepthItems.useIndexedBuckets"),
                                  QStringLiteral("all=%1 draft=%2 today=%3")
                                  .arg(m_libraryAll.notes().size())
                                  .arg(m_libraryDraft.notes().size())
                                  .arg(m_libraryToday.notes().size()));
        applyIndexedBuckets();
        setSelectedIndex(-1);
        return;
    }

    QVector<LibraryHierarchyItem> parsedItems;
    parsedItems.reserve(depthItems.size());

    int ordinal = 1;
    for (const QVariant& entry : depthItems)
    {
        parsedItems.push_back(parseItem(entry, ordinal));
        ++ordinal;
    }

    m_items = m_runtimeIndexLoaded ? prependSystemBuckets(std::move(parsedItems)) : std::move(parsedItems);
    applyChevronByDepth(&m_items);
    m_foldersHierarchyLoaded = m_runtimeIndexLoaded;
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    if (m_runtimeIndexLoaded)
    {
        refreshNoteListForSelection();
    }
    else
    {
        m_noteListModel.setItems({});
        updateNoteItemCount();
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 nextFolderSeq=%2 foldersLoaded=%3")
                              .arg(m_items.size())
                              .arg(m_createdFolderSequence)
                              .arg(m_foldersHierarchyLoaded ? QStringLiteral("1") : QStringLiteral("0")));
}

bool LibraryHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_foldersFilePath.clear();

    QString indexError;
    if (!m_libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        m_libraryDraft.clear();
        m_libraryToday.clear();
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        m_noteListModel.setItems({});
        updateNoteItemCount();
        updateLoadState(false, indexError);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("loadFromWshub.failed"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        return false;
    }

    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
    m_runtimeIndexLoaded = true;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::LibrarySupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
        return false;
    }

    WhatSonProjectsHierarchyParser foldersParser;
    QVector<WhatSonFolderDepthEntry> folderEntries;
    bool foldersFileFound = false;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Folders.wsfolders"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        foldersFileFound = true;
        if (m_foldersFilePath.isEmpty())
        {
            m_foldersFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            updateLoadState(false, readError);
            return false;
        }

        WhatSonProjectsHierarchyStore foldersStore;
        QString parseError;
        if (!foldersParser.parse(rawText, &foldersStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonFolderDepthEntry> parsedEntries = foldersStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            folderEntries.push_back(entry);
        }
    }

    if (m_foldersFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_foldersFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Folders.wsfolders"));
    }

    if (!folderEntries.isEmpty())
    {
        m_items = prependSystemBuckets(buildFolderItems(folderEntries));
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
        refreshNoteListForSelection();

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("loadFromWshub.folderHierarchy"),
                                  QStringLiteral("path=%1 folderCount=%2 all=%3 draft=%4 today=%5")
                                  .arg(wshubPath)
                                  .arg(m_items.size())
                                  .arg(m_libraryAll.notes().size())
                                  .arg(m_libraryDraft.notes().size())
                                  .arg(m_libraryToday.notes().size()));
        updateLoadState(true);
        return true;
    }

    applyIndexedBuckets();
    setSelectedIndex(-1);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("foldersFileFound=%1 all=%2 draft=%3 today=%4")
                              .arg(foldersFileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_libraryAll.notes().size())
                              .arg(m_libraryDraft.notes().size())
                              .arg(m_libraryToday.notes().size()));
    updateLoadState(true);
    return true;
}

void LibraryHierarchyViewModel::applyRuntimeSnapshot(
    const QString& wshubPath,
    QVector<LibraryNoteRecord> allNotes,
    QVector<LibraryNoteRecord> draftNotes,
    QVector<LibraryNoteRecord> todayNotes,
    QVector<WhatSonFolderDepthEntry> folderEntries,
    QString foldersFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_foldersFilePath = foldersFilePath.trimmed();

    if (!loadSucceeded)
    {
        m_libraryAll.clear();
        m_libraryDraft.clear();
        m_libraryToday.clear();
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        m_items.clear();
        m_bucketRanges.clear();
        syncModel();
        m_noteListModel.setItems({});
        updateNoteItemCount();
        updateLoadState(false, errorMessage);
        return;
    }

    m_libraryAll.setIndexedNotes(wshubPath, std::move(allNotes));
    m_libraryDraft.setNotes(std::move(draftNotes));
    m_libraryToday.setNotes(std::move(todayNotes));
    m_runtimeIndexLoaded = true;

    if (!folderEntries.isEmpty())
    {
        m_items = prependSystemBuckets(buildFolderItems(folderEntries));
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
        refreshNoteListForSelection();
    }
    else
    {
        applyIndexedBuckets();
        setSelectedIndex(-1);
    }

    updateLoadState(true);
}

QVariantList LibraryHierarchyViewModel::depthItems() const
{
    QVariantList serializedItems;
    serializedItems.reserve(m_items.size());
    for (const LibraryHierarchyItem& item : m_items)
    {
        serializedItems.push_back(QVariantMap{
            {"label", item.label},
            {"depth", item.depth},
            {"accent", item.accent},
            {"expanded", item.expanded},
            {"showChevron", item.showChevron}
        });
    }
    return serializedItems;
}

QString LibraryHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool LibraryHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const LibraryHierarchyItem& item = m_items.at(index);
    if (isProtectedRootItem(item))
    {
        return false;
    }

    if (m_runtimeIndexLoaded && !m_foldersHierarchyLoaded)
    {
        return false;
    }

    return true;
}

bool LibraryHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    const QString trimmedName = displayName.trimmed();
    if (trimmedName.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=empty label index=%1").arg(index));
        return false;
    }

    if (m_items.at(index).label == trimmedName)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.skipped"),
                                  QStringLiteral("reason=same label index=%1").arg(index));
        return true;
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems[index].label = trimmedName;

    WhatSonProjectsHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));

    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_foldersFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("renameItem"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(trimmedName));
    return true;
}

bool LibraryHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !isProtectedRootItem(m_items.at(m_selectedIndex));
}

void LibraryHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    int insertIndex = m_items.size();
    int folderDepth = 0;

    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
    {
        if (isProtectedRootItem(m_items.at(m_selectedIndex)))
        {
            insertIndex = firstEditableInsertIndex();
        }
        else
        {
            const int selectedDepth = m_items.at(m_selectedIndex).depth;
            folderDepth = selectedDepth + 1;

            insertIndex = m_selectedIndex + 1;
            while (insertIndex < m_items.size() && m_items.at(insertIndex).depth > selectedDepth)
            {
                ++insertIndex;
            }
        }
    }

    LibraryHierarchyItem newItem;
    newItem.depth = folderDepth;
    const QString folderLabel = QStringLiteral("Folder%1").arg(m_createdFolderSequence);
    newItem.label = folderLabel;
    ++m_createdFolderSequence;
    newItem.accent = false;
    newItem.expanded = false;
    newItem.showChevron = true;

    m_items.insert(insertIndex, std::move(newItem));
    if (insertIndex >= 0 && insertIndex < m_items.size())
    {
        m_items[insertIndex].label = folderLabel;
    }
    applyChevronByDepth(&m_items);
    rebuildBucketRanges();
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("createFolder"),
                              QStringLiteral("insertIndex=%1 depth=%2 itemCount=%3")
                              .arg(insertIndex)
                              .arg(folderDepth)
                              .arg(m_items.size()));
}

void LibraryHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=selection out of range selectedIndex=%1").arg(startIndex));
        return;
    }

    if (isProtectedRootItem(m_items.at(m_selectedIndex)))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=protected system bucket selectedIndex=%1").arg(startIndex));
        return;
    }

    const int baseDepth = m_items.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_items.size()
        && m_items.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    m_items.remove(startIndex, removeCount);
    applyChevronByDepth(&m_items);
    rebuildBucketRanges();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("deleteSelectedFolder"),
                              QStringLiteral("startIndex=%1 removeCount=%2 remaining=%3")
                              .arg(startIndex)
                              .arg(removeCount)
                              .arg(m_items.size()));

    if (m_items.isEmpty())
    {
        setSelectedIndex(-1);
        return;
    }

    setSelectedIndex(std::min(startIndex, static_cast<int>(m_items.size() - 1)));
}

void LibraryHierarchyViewModel::setHubStore(WhatSonHubStore store)
{
    const QString nextHubPath = store.hubPath().trimmed();
    const QString nextLibraryPath = store.libraryPath().trimmed();
    if (m_hubStore.hubPath() == nextHubPath
        && m_hubStore.libraryPath() == nextLibraryPath)
    {
        return;
    }

    m_hubStore = std::move(store);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setHubStore"),
                              QStringLiteral("hub=%1 library=%2")
                              .arg(m_hubStore.hubPath(), m_hubStore.libraryPath()));
}

WhatSonHubStore LibraryHierarchyViewModel::hubStore() const
{
    return m_hubStore;
}

int LibraryHierarchyViewModel::extractDepth(const QVariantMap& entryMap)
{
    QVariant depthValue;
    if (entryMap.contains(QStringLiteral("depth")))
    {
        depthValue = entryMap.value(QStringLiteral("depth"));
    }
    else if (entryMap.contains(QStringLiteral("dpeth")))
    {
        depthValue = entryMap.value(QStringLiteral("dpeth"));
    }
    else if (entryMap.contains(QStringLiteral("indentLevel")))
    {
        depthValue = entryMap.value(QStringLiteral("indentLevel"));
    }

    bool converted = false;
    const int depth = depthValue.toInt(&converted);
    if (!converted)
    {
        return 0;
    }

    return std::max(0, depth);
}

LibraryHierarchyItem LibraryHierarchyViewModel::parseItem(const QVariant& entry, int fallbackOrdinal)
{
    LibraryHierarchyItem parsed;
    Q_UNUSED(fallbackOrdinal);

    if (entry.metaType().id() == QMetaType::QVariantMap)
    {
        const QVariantMap entryMap = entry.toMap();
        parsed.depth = extractDepth(entryMap);
        parsed.label = entryMap.value(QStringLiteral("label")).toString().trimmed();
        parsed.accent = entryMap.value(QStringLiteral("accent"), false).toBool();
        parsed.expanded = entryMap.value(QStringLiteral("expanded"), false).toBool();
        parsed.showChevron = entryMap.value(QStringLiteral("showChevron"), true).toBool();
    }
    else
    {
        bool converted = false;
        const int depth = entry.toInt(&converted);
        if (converted)
        {
            parsed.depth = std::max(0, depth);
        }
        parsed.label = entry.toString().trimmed();
    }

    if (parsed.label.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("library.viewmodel"),
            QStringLiteral("parseItem.emptyLabelKept"));
    }

    return parsed;
}

int LibraryHierarchyViewModel::nextFolderSequence(const QVector<LibraryHierarchyItem>& items)
{
    static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

    int maxSequence = 0;
    for (const LibraryHierarchyItem& item : items)
    {
        const QRegularExpressionMatch match = folderPattern.match(item.label);
        if (!match.hasMatch())
        {
            continue;
        }

        bool converted = false;
        const int value = match.captured(1).toInt(&converted);
        if (converted)
        {
            maxSequence = std::max(maxSequence, value);
        }
    }

    return maxSequence + 1;
}

QVector<LibraryNoteListItem> LibraryHierarchyViewModel::buildNoteListItems(
    const QVector<LibraryNoteRecord>& notes)
{
    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        LibraryNoteListItem item;
        item.id = note.noteId.trimmed();
        item.primaryText = notePrimaryText(note);
        item.displayDate = noteListDisplayDate(note);
        item.folders = noteListFolders(note);
        item.tags = noteListTags(note);
        item.bookmarked = note.bookmarked;
        item.bookmarkColor = bookmarkColorHexFromNote(note);
        items.push_back(std::move(item));
    }

    return items;
}

const QVector<LibraryNoteRecord>& LibraryHierarchyViewModel::notesForBucket(IndexedBucket bucket) const
{
    switch (bucket)
    {
    case IndexedBucket::Draft:
        return m_libraryDraft.notes();
    case IndexedBucket::Today:
        return m_libraryToday.notes();
    case IndexedBucket::All:
    default:
        return m_libraryAll.notes();
    }
}

const LibraryHierarchyViewModel::IndexedBucketRange* LibraryHierarchyViewModel::bucketRangeForIndex(int index) const
    noexcept
{
    if (index < 0)
    {
        return nullptr;
    }

    for (const IndexedBucketRange& range : m_bucketRanges)
    {
        if (index >= range.startRow && index <= range.endRow)
        {
            return &range;
        }
    }

    return nullptr;
}

LibraryHierarchyViewModel::IndexedBucket LibraryHierarchyViewModel::selectedBucket() const
{
    if (m_selectedIndex < 0)
    {
        return IndexedBucket::All;
    }

    const IndexedBucketRange* range = bucketRangeForIndex(m_selectedIndex);
    if (range != nullptr)
    {
        return range->bucket;
    }

    return IndexedBucket::All;
}

LibraryHierarchyViewModel::FolderSelectionScope LibraryHierarchyViewModel::selectedFolderScope() const
{
    FolderSelectionScope scope;
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return scope;
    }

    const LibraryHierarchyItem& selectedItem = m_items.at(m_selectedIndex);
    scope.selectedLabelKey = normalizeFolderKey(selectedItem.label);
    scope.selectedPathKey = normalizeFolderKey(folderPathForIndex(m_selectedIndex));

    const int selectedDepth = selectedItem.depth;
    for (int index = m_selectedIndex; index < m_items.size(); ++index)
    {
        if (index > m_selectedIndex && m_items.at(index).depth <= selectedDepth)
        {
            break;
        }

        const QString labelKey = normalizeFolderKey(m_items.at(index).label);
        if (!labelKey.isEmpty())
        {
            scope.subtreeLabelKeys.insert(labelKey);
        }

        const QString pathKey = normalizeFolderKey(folderPathForIndex(index));
        if (!pathKey.isEmpty())
        {
            scope.subtreePathKeys.insert(pathKey);
        }
    }

    return scope;
}

QString LibraryHierarchyViewModel::normalizeFolderKey(const QString& value)
{
    QString normalized = value.trimmed();
    normalized.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (normalized.contains(QStringLiteral("//")))
    {
        normalized.replace(QStringLiteral("//"), QStringLiteral("/"));
    }
    while (normalized.startsWith(QLatin1Char('/')))
    {
        normalized.remove(0, 1);
    }
    while (normalized.endsWith(QLatin1Char('/')))
    {
        normalized.chop(1);
    }
    return normalized.toCaseFolded();
}

QString LibraryHierarchyViewModel::folderPathForIndex(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    QStringList pathSegments;
    int cursor = index;
    int targetDepth = m_items.at(index).depth;
    if (targetDepth < 0)
    {
        targetDepth = 0;
    }

    for (int depth = targetDepth; depth >= 0; --depth)
    {
        bool foundAncestor = false;
        for (int scan = cursor; scan >= 0; --scan)
        {
            if (m_items.at(scan).depth != depth)
            {
                continue;
            }

            const QString label = m_items.at(scan).label.trimmed();
            if (!label.isEmpty())
            {
                pathSegments.prepend(label);
            }
            cursor = scan - 1;
            foundAncestor = true;
            break;
        }

        if (!foundAncestor)
        {
            break;
        }
    }

    return pathSegments.join(QLatin1Char('/'));
}

bool LibraryHierarchyViewModel::noteMatchesFolderScope(
    const LibraryNoteRecord& note,
    const FolderSelectionScope& scope)
{
    if (scope.selectedLabelKey.isEmpty() && scope.selectedPathKey.isEmpty())
    {
        return true;
    }

    for (const QString& folder : note.folders)
    {
        const QString folderKey = normalizeFolderKey(folder);
        if (folderKey.isEmpty())
        {
            continue;
        }

        if (!scope.selectedLabelKey.isEmpty() && folderKey == scope.selectedLabelKey)
        {
            return true;
        }
        if (!scope.selectedPathKey.isEmpty() && folderKey == scope.selectedPathKey)
        {
            return true;
        }
        if (!scope.selectedPathKey.isEmpty()
            && folderKey.startsWith(scope.selectedPathKey + QLatin1Char('/')))
        {
            return true;
        }
        if (scope.subtreeLabelKeys.contains(folderKey) || scope.subtreePathKeys.contains(folderKey))
        {
            return true;
        }

        const int slashIndex = folderKey.lastIndexOf(QLatin1Char('/'));
        if (slashIndex >= 0)
        {
            const QString tailLabel = folderKey.mid(slashIndex + 1);
            if (scope.subtreeLabelKeys.contains(tailLabel))
            {
                return true;
            }
        }
    }

    return false;
}

int LibraryHierarchyViewModel::firstEditableInsertIndex() const noexcept
{
    int index = 0;
    while (index < m_items.size() && isProtectedRootItem(m_items.at(index)))
    {
        ++index;
    }
    return index;
}

void LibraryHierarchyViewModel::rebuildBucketRanges()
{
    m_bucketRanges.clear();

    for (int index = 0; index < m_items.size(); ++index)
    {
        const LibraryHierarchyItem& item = m_items.at(index);
        if (!isSystemBucketItem(item))
        {
            continue;
        }

        IndexedBucketRange range;
        const QString label = item.label.trimmed();
        if (label == QLatin1String(kLibraryDraftLabel))
        {
            range.bucket = IndexedBucket::Draft;
        }
        else if (label == QLatin1String(kLibraryTodayLabel))
        {
            range.bucket = IndexedBucket::Today;
        }
        else
        {
            range.bucket = IndexedBucket::All;
        }
        range.startRow = index;
        range.endRow = index;
        m_bucketRanges.push_back(range);
    }
}

void LibraryHierarchyViewModel::refreshNoteListForSelection()
{
    if (!m_runtimeIndexLoaded)
    {
        m_noteListModel.setItems({});
        updateNoteItemCount();
        return;
    }

    if (const IndexedBucketRange* range = bucketRangeForIndex(m_selectedIndex))
    {
        const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(range->bucket));
        m_noteListModel.setItems(listItems);
        updateNoteItemCount();
        return;
    }

    if (m_foldersHierarchyLoaded)
    {
        if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
        {
            m_noteListModel.setItems(buildNoteListItems(m_libraryAll.notes()));
            updateNoteItemCount();
            return;
        }

        const FolderSelectionScope scope = selectedFolderScope();
        QVector<LibraryNoteRecord> filtered;
        filtered.reserve(m_libraryAll.notes().size());

        for (const LibraryNoteRecord& note : m_libraryAll.notes())
        {
            if (noteMatchesFolderScope(note, scope))
            {
                filtered.push_back(note);
            }
        }

        m_noteListModel.setItems(buildNoteListItems(filtered));
        updateNoteItemCount();
        return;
    }

    const IndexedBucket bucket = selectedBucket();
    const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(bucket));
    m_noteListModel.setItems(listItems);
    updateNoteItemCount();
}

void LibraryHierarchyViewModel::applyIndexedBuckets()
{
    m_items = prependSystemBuckets({});
    m_foldersHierarchyLoaded = false;
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    refreshNoteListForSelection();
}

void LibraryHierarchyViewModel::syncModel()
{
    applyChevronByDepth(&m_items);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("syncModel"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
    updateItemCount();
}

void LibraryHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void LibraryHierarchyViewModel::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void LibraryHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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
