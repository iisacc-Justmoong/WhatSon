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
    WhatSon::Debug::trace(QStringLiteral("tags.viewmodel"), QStringLiteral("ctor"));
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
    WhatSon::Debug::trace(
        QStringLiteral("tags.viewmodel"),
        QStringLiteral("setSelectedIndex"),
        QStringLiteral("value=%1").arg(m_selectedIndex));
    emit selectedIndexChanged();
}

void TagsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    QVector<WhatSonTagDepthEntry> parsedEntries;
    parsedEntries.reserve(depthItems.size());

    int ordinal = 1;
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

        if (parsedEntry.label.isEmpty())
        {
            parsedEntry.label = fallbackLabel(ordinal);
        }
        if (parsedEntry.id.isEmpty())
        {
            parsedEntry.id = parsedEntry.label;
        }

        parsedEntries.push_back(std::move(parsedEntry));
        ++ordinal;
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

bool TagsHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    if (!renameEnabled() || index < 0 || index >= m_entries.size())
    {
        return false;
    }

    const QString trimmed = displayName.trimmed();
    if (trimmed.isEmpty())
    {
        return false;
    }

    m_entries[index].label = trimmed;
    if (m_entries[index].id.trimmed().isEmpty())
    {
        m_entries[index].id = trimmed;
    }

    m_items = buildItems(m_entries);
    syncStore();
    syncModel();
    return true;
}

void TagsHierarchyViewModel::createFolder()
{
    if (!createFolderEnabled())
    {
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

    const QString folderLabel = QStringLiteral("Folder%1").arg(m_createdFolderSequence++);
    WhatSonTagDepthEntry entry;
    entry.id = folderLabel;
    entry.label = folderLabel;
    entry.depth = depth;
    m_entries.insert(insertIndex, std::move(entry));

    m_items = buildItems(m_entries);
    syncStore();
    syncModel();
    setSelectedIndex(insertIndex);
}

void TagsHierarchyViewModel::deleteSelectedFolder()
{
    if (!deleteFolderEnabled())
    {
        return;
    }

    const int startIndex = m_selectedIndex;
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
}

void TagsHierarchyViewModel::setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries)
{
    WhatSon::Debug::trace(
        QStringLiteral("tags.viewmodel"),
        QStringLiteral("setTagDepthEntries.begin"),
        QStringLiteral("count=%1").arg(entries.size()));

    QVector<WhatSonTagDepthEntry> sanitized;
    sanitized.reserve(entries.size());

    int ordinal = 1;
    for (WhatSonTagDepthEntry& entry : entries)
    {
        entry.id = entry.id.trimmed();
        entry.label = entry.label.trimmed();
        entry.depth = std::max(0, entry.depth);

        if (entry.label.isEmpty())
        {
            entry.label = entry.id.isEmpty() ? fallbackLabel(ordinal) : entry.id;
        }
        if (entry.id.isEmpty())
        {
            entry.id = entry.label;
        }

        sanitized.push_back(std::move(entry));
        ++ordinal;
    }

    m_entries = std::move(sanitized);
    m_items = buildItems(m_entries);
    syncStore();
    m_createdFolderSequence = nextFolderSequence(m_entries);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::trace(
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
    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
        return false;
    }

    QVector<WhatSonTagDepthEntry> aggregated;
    WhatSonTagsHierarchyParser parser;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
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
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonTagDepthEntry> parsed = m_store.tagEntries();
        for (const WhatSonTagDepthEntry& entry : parsed)
        {
            aggregated.push_back(entry);
        }
    }

    setTagDepthEntries(std::move(aggregated));
    updateLoadState(true);
    return true;
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

QString TagsHierarchyViewModel::fallbackLabel(int ordinal)
{
    return QStringLiteral("Tag%1").arg(ordinal);
}

QVector<TagsHierarchyItem> TagsHierarchyViewModel::buildItems(const QVector<WhatSonTagDepthEntry>& entries)
{
    QVector<TagsHierarchyItem> items;
    items.reserve(entries.size());

    int ordinal = 1;
    for (const WhatSonTagDepthEntry& entry : entries)
    {
        TagsHierarchyItem item;
        item.id = entry.id.trimmed();
        item.label = entry.label.trimmed();
        item.depth = std::max(0, entry.depth);

        if (item.label.isEmpty())
        {
            item.label = item.id.isEmpty() ? fallbackLabel(ordinal) : item.id;
        }

        items.push_back(std::move(item));
        ++ordinal;
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
    WhatSon::Debug::trace(
        QStringLiteral("tags.viewmodel"),
        QStringLiteral("syncModel"),
        QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
    updateItemCount();
}
