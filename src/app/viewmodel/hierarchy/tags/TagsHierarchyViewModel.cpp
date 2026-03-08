#include "TagsHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>

#include <algorithm>
#include <utility>

TagsHierarchyViewModel::TagsHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("tags.viewmodel"), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &TagsHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    syncModel();
}

TagsHierarchyViewModel::~TagsHierarchyViewModel() = default;

TagsHierarchyModel* TagsHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

int TagsHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int TagsHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool TagsHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString TagsHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void TagsHierarchyViewModel::setSelectedIndex(int index)
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
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void TagsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    QVector<WhatSonTagDepthEntry> parsedEntries;
    parsedEntries.reserve(depthItems.size());

    for (const QVariant& entry : depthItems)
    {
        WhatSonTagDepthEntry parsedEntry;

        if (entry.metaType().id() == QMetaType::QVariantMap)
        {
            const QVariantMap entryMap = entry.toMap();
            parsedEntry.id = entryMap.value(QStringLiteral("id")).toString().trimmed();
            parsedEntry.label = entryMap.value(QStringLiteral("label")).toString().trimmed();
            parsedEntry.depth = extractDepth(entryMap);
        }
        else
        {
            parsedEntry.depth = 0;
            parsedEntry.label = entry.toString().trimmed();
        }

        if (parsedEntry.id.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setDepthItems.emptyIdKept"),
                                      QStringLiteral("label=%1").arg(parsedEntry.label));
        }
        if (parsedEntry.label.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setDepthItems.emptyLabelKept"));
        }

        parsedEntries.push_back(std::move(parsedEntry));
    }

    setTagDepthEntries(std::move(parsedEntries));
}

QVariantList TagsHierarchyViewModel::depthItems() const
{
    QVariantList serialized;
    serialized.reserve(m_entries.size());

    for (const WhatSonTagDepthEntry& entry : m_entries)
    {
        serialized.push_back(QVariantMap{
            {QStringLiteral("id"), entry.id},
            {QStringLiteral("label"), entry.label},
            {QStringLiteral("depth"), entry.depth}
        });
    }

    return serialized;
}

QString TagsHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index).label;
}

bool TagsHierarchyViewModel::canRenameItem(int index) const
{
    return index >= 0 && index < m_entries.size();
}

bool TagsHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    const QString trimmed = displayName.trimmed();
    if (trimmed.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=empty label index=%1").arg(index));
        return false;
    }

    QVector<WhatSonTagDepthEntry> stagedEntries = m_entries;
    stagedEntries[index].label = trimmed;
    if (stagedEntries[index].id.trimmed().isEmpty())
    {
        stagedEntries[index].id = trimmed;
    }

    WhatSonTagsHierarchyStore stagedStore = m_store;
    stagedStore.setTagEntries(stagedEntries);

    if (!m_tagsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_tagsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_tagsFilePath, writeError));
            return false;
        }
    }

    m_entries = std::move(stagedEntries);
    m_store = std::move(stagedStore);
    m_items = buildItems(m_entries);
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(trimmed).arg(
                                  m_items.size()));
    return true;
}

void TagsHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).
                                                                              arg(m_entries.size()));
    if (!createFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=createFolderEnabled false"));
        return;
    }

    int insertIndex = m_entries.size();
    int depth = 0;

    if (m_selectedIndex >= 0 && m_selectedIndex < m_entries.size())
    {
        const int selectedDepth = std::max(0, m_entries.at(m_selectedIndex).depth);
        depth = selectedDepth + 1;

        insertIndex = m_selectedIndex + 1;
        while (insertIndex < m_entries.size() && m_entries.at(insertIndex).depth > selectedDepth)
        {
            ++insertIndex;
        }
    }

    WhatSonTagDepthEntry entry;
    ++m_createdFolderSequence;
    entry.id.clear();
    entry.label = QStringLiteral("Untitled");
    entry.depth = depth;
    m_entries.insert(insertIndex, std::move(entry));

    m_items = buildItems(m_entries);
    syncStore();
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("createFolder.success"),
                              QStringLiteral("insertIndex=%1 label=<empty> depth=%2 itemCount=%3").arg(insertIndex).
                              arg(depth).arg(m_entries.size()));
}

void TagsHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_entries.size()));
    if (!deleteFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=deleteFolderEnabled false selectedIndex=%1").arg(startIndex));
        return;
    }

    const int baseDepth = m_entries.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_entries.size() && m_entries.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    m_entries.remove(startIndex, removeCount);
    m_items = buildItems(m_entries);
    syncStore();
    syncModel();
    setSelectedIndex(std::min(startIndex, static_cast<int>(m_entries.size() - 1)));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 removeCount=%2 itemCount=%3").arg(startIndex).
                              arg(removeCount).arg(m_entries.size()));
}

void TagsHierarchyViewModel::setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setTagDepthEntries.begin"),
                              QStringLiteral("count=%1").arg(entries.size()));

    QVector<WhatSonTagDepthEntry> sanitized;
    sanitized.reserve(entries.size());

    for (WhatSonTagDepthEntry& entry : entries)
    {
        entry.id = entry.id.trimmed();
        entry.label = entry.label.trimmed();
        entry.depth = std::max(0, entry.depth);

        if (entry.label.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setTagDepthEntries.emptyLabelKept"));
        }
        if (entry.id.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setTagDepthEntries.emptyIdKept"),
                                      QStringLiteral("label=%1").arg(entry.label));
        }

        sanitized.push_back(std::move(entry));
    }

    m_entries = std::move(sanitized);
    m_items = buildItems(m_entries);
    syncStore();
    m_createdFolderSequence = nextFolderSequence(m_entries);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setTagDepthEntries.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVector<WhatSonTagDepthEntry> TagsHierarchyViewModel::tagDepthEntries() const
{
    return m_entries;
}

bool TagsHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_tagsFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    QVector<WhatSonTagDepthEntry> aggregated;
    bool fileFound = false;
    WhatSonTagsHierarchyParser parser;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }
        fileFound = true;

        if (m_tagsFilePath.isEmpty())
        {
            m_tagsFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
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
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("loadFromWshub.failed.parse"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, parseError));
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonTagDepthEntry> parsed = m_store.tagEntries();
        for (const WhatSonTagDepthEntry& entry : parsed)
        {
            aggregated.push_back(entry);
        }
    }

    if (m_tagsFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_tagsFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Tags.wstags"));
    }

    setTagDepthEntries(std::move(aggregated));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3").arg(wshubPath).arg(
                                  fileFound ? QStringLiteral("1") : QStringLiteral("0")).arg(m_entries.size()));
    updateLoadState(true);
    return true;
}

void TagsHierarchyViewModel::applyRuntimeSnapshot(
    QVector<WhatSonTagDepthEntry> entries,
    QString tagsFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_tagsFilePath = tagsFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    setTagDepthEntries(std::move(entries));
    updateLoadState(true);
}

bool TagsHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return m_selectedIndex >= 0;
}

int TagsHierarchyViewModel::extractDepth(const QVariantMap& entryMap)
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
    return converted ? std::max(0, depth) : 0;
}

QVector<TagsHierarchyItem> TagsHierarchyViewModel::buildItems(const QVector<WhatSonTagDepthEntry>& entries)
{
    QVector<TagsHierarchyItem> items;
    items.reserve(entries.size());

    for (const WhatSonTagDepthEntry& entry : entries)
    {
        TagsHierarchyItem item;
        item.id = entry.id.trimmed();
        item.label = entry.label.trimmed();
        item.depth = std::max(0, entry.depth);

        if (item.label.isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("tags.viewmodel"),
                QStringLiteral("buildItems.emptyLabelKept"),
                QStringLiteral("id=%1").arg(item.id));
        }

        items.push_back(std::move(item));
    }

    for (int index = 0; index < items.size(); ++index)
    {
        const int nextIndex = index + 1;
        const bool hasChild = nextIndex < items.size() && items.at(nextIndex).depth > items.at(index).depth;
        items[index].showChevron = hasChild;
    }

    return items;
}

int TagsHierarchyViewModel::nextFolderSequence(const QVector<WhatSonTagDepthEntry>& entries)
{
    static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

    int maxSequence = 0;
    for (const WhatSonTagDepthEntry& entry : entries)
    {
        const QRegularExpressionMatch match = folderPattern.match(entry.label.trimmed());
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

void TagsHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void TagsHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

void TagsHierarchyViewModel::syncStore()
{
    m_store.setTagEntries(m_entries);
}

void TagsHierarchyViewModel::syncModel()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("syncModel"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
    updateItemCount();
}
