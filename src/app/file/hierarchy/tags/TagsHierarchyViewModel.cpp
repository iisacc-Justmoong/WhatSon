#include "TagsHierarchyViewModel.hpp"

#include <algorithm>
#include <utility>

TagsHierarchyViewModel::TagsHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
{
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

void TagsHierarchyViewModel::setSelectedIndex(int index)
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
    emit selectedIndexChanged();
}

void TagsHierarchyViewModel::setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries)
{
    m_entries = std::move(entries);
    m_items = buildItems(m_entries);
    syncModel();
    setSelectedIndex(-1);
}

QVector<WhatSonTagDepthEntry> TagsHierarchyViewModel::tagDepthEntries() const
{
    return m_entries;
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

void TagsHierarchyViewModel::syncModel()
{
    m_itemModel.setItems(m_items);
}
