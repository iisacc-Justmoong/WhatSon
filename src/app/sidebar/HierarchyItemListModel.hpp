#pragma once

#include "HierarchySectionModel.hpp"

#include <QAbstractListModel>
#include <QVector>

class HierarchyItemListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        LabelRole = Qt::UserRole + 1,
        IndentLevelRole,
        AccentRole,
        ExpandedRole,
        ShowChevronRole
    };

    Q_ENUM(Role)

    explicit HierarchyItemListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QVector<SidebarHierarchyItem> items);

private:
    QVector<SidebarHierarchyItem> m_items;
};
