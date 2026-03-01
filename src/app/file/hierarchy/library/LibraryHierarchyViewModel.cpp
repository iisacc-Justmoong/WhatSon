#include "LibraryHierarchyViewModel.hpp"

#include "WhatSonDebugTrace.hpp"

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
} // namespace

LibraryHierarchyViewModel::LibraryHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::trace(QStringLiteral("library.viewmodel"), QStringLiteral("ctor"));
    syncModel();
}

LibraryHierarchyViewModel::~LibraryHierarchyViewModel() = default;

LibraryHierarchyModel* LibraryHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
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
    emit selectedIndexChanged();
}

void LibraryHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("setDepthItems.begin"),
        QStringLiteral("count=%1").arg(depthItems.size()));
    QVector<LibraryHierarchyItem> parsedItems;
    parsedItems.reserve(depthItems.size());

    int ordinal = 1;
    for (const QVariant& entry : depthItems)
    {
        parsedItems.push_back(parseItem(entry, ordinal));
        ++ordinal;
    }

    m_items = std::move(parsedItems);
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("setDepthItems.success"),
        QStringLiteral("itemCount=%1 nextFolderSeq=%2").arg(m_items.size()).arg(m_createdFolderSequence));
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

void LibraryHierarchyViewModel::syncModel()
{
    WhatSon::Debug::trace(
        QStringLiteral("library.viewmodel"),
        QStringLiteral("syncModel"),
        QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
}
