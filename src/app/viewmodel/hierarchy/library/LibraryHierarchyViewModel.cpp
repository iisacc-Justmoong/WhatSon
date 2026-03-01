#include "LibraryHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
    QString nextFolderName(int sequence)
    {
        return QStringLiteral("Folder%1").arg(sequence);
    }

    QString noteDisplayLabel(const LibraryNoteRecord& note)
    {
        QString primary = note.title.trimmed();
        if (primary.isEmpty())
        {
            primary = note.noteId.trimmed();
        }
        if (primary.isEmpty() && !note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName();
        }
        if (primary.isEmpty())
        {
            primary = QStringLiteral("Untitled Note");
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

        return QStringLiteral("Untitled Note");
    }

    QString noteListSummary(const LibraryNoteRecord& note)
    {
        QStringList summaryParts;
        if (!note.lastModifiedAt.trimmed().isEmpty())
        {
            summaryParts.push_back(QStringLiteral("Modified %1").arg(note.lastModifiedAt.trimmed()));
        }
        else if (!note.createdAt.trimmed().isEmpty())
        {
            summaryParts.push_back(QStringLiteral("Created %1").arg(note.createdAt.trimmed()));
        }

        if (!note.project.trimmed().isEmpty())
        {
            summaryParts.push_back(QStringLiteral("Project %1").arg(note.project.trimmed()));
        }

        if (!note.tags.isEmpty())
        {
            summaryParts.push_back(note.tags.join(QStringLiteral(" • ")));
        }

        if (summaryParts.isEmpty())
        {
            return QStringLiteral("No metadata");
        }
        return summaryParts.join(QStringLiteral(" | "));
    }

    QString noteListFolders(const LibraryNoteRecord& note)
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
            (*items)[index].showChevron = items->at(index).showChevron && hasChild;
        }
    }
} // namespace

LibraryHierarchyViewModel::LibraryHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
      , m_noteListModel(this)
{
    WhatSon::Debug::trace(QStringLiteral("library.viewmodel"), QStringLiteral("ctor"));
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

void LibraryHierarchyViewModel::setSelectedIndex(int index)
{
    const int maxIndex = m_items.size() - 1;
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
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void LibraryHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("setDepthItems.begin"),
        QStringLiteral("count=%1").arg(depthItems.size()));

    if (depthItems.isEmpty() && m_runtimeIndexLoaded)
    {
        WhatSon::Debug::trace(
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
    m_bucketRanges.clear();
    m_rowNoteIds.clear();
    m_rowNoteIds.resize(m_items.size());
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    m_noteListModel.setItems({});
    setSelectedIndex(-1);
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("setDepthItems.success"),
        QStringLiteral("itemCount=%1 nextFolderSeq=%2").arg(m_items.size()).arg(m_createdFolderSequence));
}

bool LibraryHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QString indexError;
    if (!m_libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        m_libraryDraft.clear();
        m_libraryToday.clear();
        m_runtimeIndexLoaded = false;
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        m_noteListModel.setItems({});
        WhatSon::Debug::trace(
            QStringLiteral("library.viewmodel"),
            QStringLiteral("loadFromWshub.failed"),
            QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        return false;
    }

    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
    m_runtimeIndexLoaded = true;

    applyIndexedBuckets();
    setSelectedIndex(-1);

    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("loadFromWshub.success"),
        QStringLiteral("all=%1 draft=%2 today=%3")
        .arg(m_libraryAll.notes().size())
        .arg(m_libraryDraft.notes().size())
        .arg(m_libraryToday.notes().size()));
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

bool LibraryHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const QString trimmedName = displayName.trimmed();
    if (trimmedName.isEmpty())
    {
        return false;
    }

    LibraryHierarchyItem& target = m_items[index];
    if (target.label == trimmedName)
    {
        return true;
    }

    target.label = trimmedName;
    syncModel();
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("renameItem"),
        QStringLiteral("index=%1 label=%2").arg(index).arg(trimmedName));
    return true;
}

void LibraryHierarchyViewModel::createFolder()
{
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
    newItem.label = nextFolderName(m_createdFolderSequence++);
    newItem.accent = false;
    newItem.expanded = false;
    newItem.showChevron = true;

    m_items.insert(insertIndex, std::move(newItem));
    m_bucketRanges.clear();
    m_rowNoteIds.clear();
    m_rowNoteIds.resize(m_items.size());
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("createFolder"),
        QStringLiteral("insertIndex=%1 depth=%2 itemCount=%3")
        .arg(insertIndex)
        .arg(folderDepth)
        .arg(m_items.size()));
}

void LibraryHierarchyViewModel::deleteSelectedFolder()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return;
    }

    const int startIndex = m_selectedIndex;
    const int baseDepth = m_items.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_items.size()
        && m_items.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    m_items.remove(startIndex, removeCount);
    m_bucketRanges.clear();
    m_rowNoteIds.clear();
    m_rowNoteIds.resize(m_items.size());
    syncModel();
    WhatSon::Debug::trace(
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
        parsed.label = nextFolderName(fallbackOrdinal);
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
    const QVector<LibraryNoteRecord>& notes,
    const QString& highlightedNoteId)
{
    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());

    const QString highlightedKey = highlightedNoteId.trimmed().toCaseFolded();

    for (const LibraryNoteRecord& note : notes)
    {
        LibraryNoteListItem item;
        item.noteId = note.noteId.trimmed();
        item.titleText = noteListTitle(note);
        item.summaryText = noteListSummary(note);
        item.foldersText = noteListFolders(note);
        item.bookmarked = note.bookmarked;
        item.highlighted = !highlightedKey.isEmpty() && item.noteId.toCaseFolded() == highlightedKey;
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

QString LibraryHierarchyViewModel::selectedNoteId() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_rowNoteIds.size())
    {
        return {};
    }

    return m_rowNoteIds.at(m_selectedIndex).trimmed();
}

void LibraryHierarchyViewModel::rebuildBucketRanges()
{
    m_bucketRanges.clear();
    m_rowNoteIds.clear();
    m_rowNoteIds.resize(m_items.size());

    int cursor = 0;
    auto appendRange = [&](IndexedBucket bucket, const QVector<LibraryNoteRecord>& notes)
    {
        IndexedBucketRange range;
        range.bucket = bucket;
        range.startRow = cursor;
        range.endRow = cursor;

        ++cursor;
        int row = range.startRow + 1;
        for (const LibraryNoteRecord& note : notes)
        {
            if (row >= 0 && row < m_rowNoteIds.size())
            {
                m_rowNoteIds[row] = note.noteId.trimmed();
            }
            ++row;
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
        return;
    }

    const IndexedBucket bucket = selectedBucket();
    const QString highlighted = selectedNoteId();
    const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(bucket), highlighted);
    m_noteListModel.setItems(listItems);
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
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    refreshNoteListForSelection();
}

void LibraryHierarchyViewModel::syncModel()
{
    applyChevronByDepth(&m_items);
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("syncModel"),
        QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
}
