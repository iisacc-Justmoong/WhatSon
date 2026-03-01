#pragma once

#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/FlatHierarchyModel.hpp"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class ResourcesHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FlatHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ResourcesHierarchyViewModel(QObject* parent = nullptr);
    ~ResourcesHierarchyViewModel() override;

    FlatHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setResourcePaths(QStringList resourcePaths);
    QStringList resourcePaths() const;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    signals  :



    void selectedIndexChanged();

private:
    void syncModel();
    void syncDomainStoreFromItems();

    QStringList m_resourcePaths;
    QVector<FlatHierarchyItem> m_items;
    WhatSonResourcesHierarchyStore m_store;
    FlatHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
};
