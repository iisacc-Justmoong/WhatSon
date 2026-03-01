#pragma once

#include "viewmodel/hierarchy/common/FlatHierarchyModel.hpp"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class ProjectsHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FlatHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    explicit ProjectsHierarchyViewModel(QObject* parent = nullptr);
    ~ProjectsHierarchyViewModel() override;

    FlatHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;

    void setProjectNames(QStringList projectNames);
    QStringList projectNames() const;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    signals  :


    void selectedIndexChanged();

private:
    void syncModel();

    QStringList m_projectNames;
    QVector<FlatHierarchyItem> m_items;
    FlatHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
};
