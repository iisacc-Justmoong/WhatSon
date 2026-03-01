#include "TagsHierarchyModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <utility>

TagsHierarchyModel::TagsHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("tags.model"), QStringLiteral("ctor"));
}

int TagsHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant TagsHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const TagsHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
    case IdRole:
        return item.id;
    case LabelRole:
        return item.label;
    case DepthRole:
    case IndentLevelRole:
        return item.depth;
    case AccentRole:
        return item.accent;
    case ExpandedRole:
        return item.expanded;
    case ShowChevronRole:
        return item.showChevron;
    default:
        return {};
    }
}

QHash<int, QByteArray> TagsHierarchyModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {LabelRole, "label"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"}
    };
}

void TagsHierarchyModel::setItems(QVector<TagsHierarchyItem> items)
{
    WhatSon::Debug::trace(
        QStringLiteral("tags.model"),
        QStringLiteral("setItems"),
        QStringLiteral("count=%1").arg(items.size()));
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

const QVector<TagsHierarchyItem>& TagsHierarchyModel::items() const noexcept
{
    return m_items;
}
