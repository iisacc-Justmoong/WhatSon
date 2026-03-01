#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct TagsHierarchyItem
{
    QString id;
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

class TagsHierarchyModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        LabelRole,
        DepthRole,
        IndentLevelRole,
        AccentRole,
        ExpandedRole,
        ShowChevronRole
    };

    Q_ENUM(Role)

    explicit TagsHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QVector<TagsHierarchyItem> items);
    const QVector<TagsHierarchyItem>& items() const noexcept;

private:
    QVector<TagsHierarchyItem> m_items;
};
