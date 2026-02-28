#include "LibraryHierarchyModel.hpp"

#include <utility>

LibraryHierarchyModel::LibraryHierarchyModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int LibraryHierarchyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_items.size();
}

QVariant LibraryHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const LibraryHierarchyItem& item = m_items.at(index.row());
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

QHash<int, QByteArray> LibraryHierarchyModel::roleNames() const
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

void LibraryHierarchyModel::setItems(QVector<LibraryHierarchyItem> items)
{
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

const QVector<LibraryHierarchyItem>& LibraryHierarchyModel::items() const noexcept
{
    return m_items;
}
