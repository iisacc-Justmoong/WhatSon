#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct FlatHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

class FlatHierarchyModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        LabelRole = Qt::UserRole + 1,
        DepthRole,
        IndentLevelRole,
        AccentRole,
        ExpandedRole,
        ShowChevronRole
    };

    Q_ENUM(Role)

    explicit FlatHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QVector<FlatHierarchyItem> items);
    const QVector<FlatHierarchyItem>& items() const noexcept;

private:
    QVector<FlatHierarchyItem> m_items;
};
