#include "HierarchyItemListModel.hpp"

#include "../../file/WhatSonDebugTrace.hpp"

#include <utility>

HierarchyItemListModel::HierarchyItemListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    WhatSon::Debug::trace(QStringLiteral("sidebar.itemModel"), QStringLiteral("ctor"));
}

int HierarchyItemListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant HierarchyItemListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const SidebarHierarchyItem& item = m_items.at(index.row());
    switch (role)
    {
    case LabelRole:
        return item.label;
    case IndentLevelRole:
        return item.indentLevel;
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

QHash<int, QByteArray> HierarchyItemListModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {IndentLevelRole, "indentLevel"},
        {AccentRole, "accent"},
        {ExpandedRole, "expanded"},
        {ShowChevronRole, "showChevron"}
    };
}

void HierarchyItemListModel::setItems(QVector<SidebarHierarchyItem> items)
{
    const int previousCount = static_cast<int>(m_items.size());
    const int incomingCount = static_cast<int>(items.size());
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.itemModel"),
        QStringLiteral("setItems"),
        QStringLiteral("previous=%1 incoming=%2").arg(previousCount).arg(incomingCount));
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
    WhatSon::Debug::trace(
        QStringLiteral("sidebar.itemModel"),
        QStringLiteral("setItemsApplied"),
        QStringLiteral("current=%1").arg(static_cast<int>(m_items.size())));
}
