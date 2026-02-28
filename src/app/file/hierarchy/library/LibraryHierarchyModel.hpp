#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct LibraryHierarchyItem
{
    int depth = 0;
    bool accent = false;
    bool expanded = false;
    QString label;
    bool showChevron = true;
};

class LibraryHierarchyModel final : public QAbstractListModel
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

    explicit LibraryHierarchyModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(QVector<LibraryHierarchyItem> items);
    const QVector<LibraryHierarchyItem>& items() const noexcept;

private:
    QVector<LibraryHierarchyItem> m_items;
};
