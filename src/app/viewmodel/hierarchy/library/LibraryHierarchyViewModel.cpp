#include "LibraryHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
    constexpr int kMaxNoteListSummaryLines = 5;

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

    QString noteDisplayLabel(const LibraryNoteRecord& note)
    {
        QString primary = note.title.trimmed();
        if (primary.isEmpty())
        {
            primary = note.bodyFirstLine.trimmed();
        }
        if (primary.isEmpty())
        {
            primary = note.noteId.trimmed();
        }
        if (primary.isEmpty() && !note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
        }

        QStringList attributes;
        if (!note.noteId.trimmed().isEmpty())
        {
            attributes.push_back(QStringLiteral("id=%1").arg(note.noteId.trimmed()));
        }
        if (!note.lastModifiedAt.trimmed().isEmpty())
        {
            attributes.push_back(QStringLiteral("modified=%1").arg(note.lastModifiedAt.trimmed()));
        }

        if (attributes.isEmpty())
        {
            return primary;
        }
        return QStringLiteral("%1 (%2)").arg(primary, attributes.join(QStringLiteral(", ")));
    }

    QString noteListTitle(const LibraryNoteRecord& note)
    {
        QString title = note.title.trimmed();
        if (!title.isEmpty())
        {
            return title;
        }

        title = note.bodyFirstLine.trimmed();
        if (!title.isEmpty())
        {
            return title;
        }

        title = note.noteId.trimmed();
        if (!title.isEmpty())
        {
            return title;
        }

        if (!note.noteDirectoryPath.isEmpty())
        {
            const QString fromPath = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
            if (!fromPath.isEmpty())
            {
                return fromPath;
            }
        }

        return {};
    }

    QString noteListSummary(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return {};
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
        const int depthOffset =
            (!items.isEmpty() && items.first().accent && items.first().depth == 0) ? 1 : 0;

        for (const LibraryHierarchyItem& item : items)
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
            entry.depth = std::max(0, item.depth - depthOffset);
            entries.push_back(std::move(entry));
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

    m_items = std::move(parsedItems);
    applyChevronByDepth(&m_items);
    m_foldersHierarchyLoaded = false;
    m_bucketRanges.clear();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    m_noteListModel.setItems({});
    updateNoteItemCount();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 nextFolderSeq=%2").arg(m_items.size()).arg(
                                  m_createdFolderSequence));
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
        m_items = buildFolderItems(folderEntries);
        m_foldersHierarchyLoaded = true;
        m_bucketRanges.clear();
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
    if (m_runtimeIndexLoaded && !m_foldersHierarchyLoaded)
    {
        return false;
    }

    const LibraryHierarchyItem& item = m_items.at(index);
    return !(item.accent && item.depth == 0);
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

    return !(m_items.at(m_selectedIndex).accent && m_items.at(m_selectedIndex).depth == 0);
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
        const int selectedDepth = m_items.at(m_selectedIndex).depth;
        folderDepth = selectedDepth + 1;

        insertIndex = m_selectedIndex + 1;
        while (insertIndex < m_items.size() && m_items.at(insertIndex).depth > selectedDepth)
        {
            ++insertIndex;
        }
    }

    LibraryHierarchyItem newItem;
    newItem.depth = folderDepth;
    ++m_createdFolderSequence;
    newItem.label.clear();
    newItem.accent = false;
    newItem.expanded = false;
    newItem.showChevron = true;

    m_items.insert(insertIndex, std::move(newItem));
    applyChevronByDepth(&m_items);
    m_bucketRanges.clear();
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

    const int baseDepth = m_items.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_items.size()
        && m_items.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    m_items.remove(startIndex, removeCount);
    applyChevronByDepth(&m_items);
    m_bucketRanges.clear();
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
        item.title = noteListTitle(note);
        item.desc = noteListSummary(note);
        item.folders = noteListFolders(note);
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

LibraryHierarchyViewModel::IndexedBucket LibraryHierarchyViewModel::selectedBucket() const
{
    if (m_selectedIndex < 0)
    {
        return IndexedBucket::All;
    }

    for (const IndexedBucketRange& range : m_bucketRanges)
    {
        if (m_selectedIndex >= range.startRow && m_selectedIndex <= range.endRow)
        {
            return range.bucket;
        }
    }

    return IndexedBucket::All;
}

void LibraryHierarchyViewModel::rebuildBucketRanges()
{
    m_bucketRanges.clear();

    int cursor = 0;
    auto appendRange = [&](IndexedBucket bucket, const QVector<LibraryNoteRecord>& notes)
    {
        IndexedBucketRange range;
        range.bucket = bucket;
        range.startRow = cursor;
        range.endRow = cursor;

        ++cursor;
        for (const LibraryNoteRecord& note : notes)
        {
            Q_UNUSED(note);
            ++cursor;
        }

        range.endRow = std::max(range.startRow, cursor - 1);
        m_bucketRanges.push_back(range);
    };

    appendRange(IndexedBucket::All, m_libraryAll.notes());
    appendRange(IndexedBucket::Draft, m_libraryDraft.notes());
    appendRange(IndexedBucket::Today, m_libraryToday.notes());
}

void LibraryHierarchyViewModel::refreshNoteListForSelection()
{
    if (!m_runtimeIndexLoaded)
    {
        m_noteListModel.setItems({});
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
    QVector<LibraryHierarchyItem> indexedItems;

    auto appendBucket = [&indexedItems](const QString& label, const QVector<LibraryNoteRecord>& notes)
    {
        LibraryHierarchyItem bucket;
        bucket.depth = 0;
        bucket.accent = true;
        bucket.expanded = true;
        bucket.label = QStringLiteral("%1 (%2)").arg(label).arg(notes.size());
        bucket.showChevron = !notes.isEmpty();
        indexedItems.push_back(std::move(bucket));

        for (const LibraryNoteRecord& note : notes)
        {
            LibraryHierarchyItem noteItem;
            noteItem.depth = 1;
            noteItem.accent = false;
            noteItem.expanded = false;
            noteItem.label = noteDisplayLabel(note);
            noteItem.showChevron = false;
            indexedItems.push_back(std::move(noteItem));
        }
    };

    appendBucket(QStringLiteral("All"), m_libraryAll.notes());
    appendBucket(QStringLiteral("Draft"), m_libraryDraft.notes());
    appendBucket(QStringLiteral("Today"), m_libraryToday.notes());

    m_items = std::move(indexedItems);
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
