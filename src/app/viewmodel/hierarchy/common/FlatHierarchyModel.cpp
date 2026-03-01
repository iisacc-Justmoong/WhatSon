#include "FlatHierarchyModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <utility>

FlatHierarchyModel::FlatHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("hierarchy.flat.model"), QStringLiteral("ctor"));
}

int FlatHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant FlatHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const FlatHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
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

QHash<int, QByteArray> FlatHierarchyModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {DepthRole, "depth"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"}
    };
}

void FlatHierarchyModel::setItems(QVector<FlatHierarchyItem> items)
{
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.flat.model"),
        QStringLiteral("setItems"),
        QStringLiteral("count=%1").arg(items.size()));
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

const QVector<FlatHierarchyItem>& FlatHierarchyModel::items() const noexcept
{
    return m_items;
}
