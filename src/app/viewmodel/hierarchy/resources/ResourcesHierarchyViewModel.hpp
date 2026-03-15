#pragma once

#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyModel.hpp"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class ResourcesHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ResourcesHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ResourcesHierarchyViewModel(QObject* parent = nullptr);
    ~ResourcesHierarchyViewModel() override;

    ResourcesHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);
    int itemCount() const noexcept;
    bool loadSucceeded() const noexcept;
    QString lastLoadError() const;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    QVariantList hierarchyModel() const;
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setResourcePaths(QStringList resourcePaths);
    QStringList resourcePaths() const;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QStringList resourcePaths,
        QString resourcesFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncModel();
    void syncDomainStoreFromItems();

    QStringList m_resourcePaths;
    QVector<ResourcesHierarchyItem> m_items;
    WhatSonResourcesHierarchyStore m_store;
    ResourcesHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_resourcesFilePath;
};
