#pragma once

#include "app/models/file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "app/models/file/hierarchy/IHierarchyCapabilities.hpp"
#include "app/models/file/hierarchy/IHierarchyController.hpp"
#include "app/models/file/hierarchy/resources/ResourcesListModel.hpp"
#include "app/models/file/hierarchy/resources/ResourcesHierarchyModel.hpp"

#include <QStringList>
#include <QVariantList>
#include <QVector>

class ResourcesHierarchyController final : public IHierarchyController,
                                          public IHierarchyRenameCapability,
                                          public IHierarchyCrudCapability,
                                          public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

    Q_PROPERTY(ResourcesHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(ResourcesListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ResourcesHierarchyController(QObject* parent = nullptr);
    ~ResourcesHierarchyController() override;

    ResourcesHierarchyModel* itemModel() noexcept override;
    ResourcesListModel* noteListModel() noexcept override;

    int selectedIndex() const noexcept override;
    Q_INVOKABLE void setSelectedIndex(int index) override;
    int itemCount() const noexcept override;
    bool loadSucceeded() const noexcept override;
    QString lastLoadError() const override;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    QVariantList hierarchyModel() const override;
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const override;
    Q_INVOKABLE bool canRenameItem(int index) const override;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName) override;
    Q_INVOKABLE bool setItemExpanded(int index, bool expanded) override;
    Q_INVOKABLE void createFolder() override;
    Q_INVOKABLE void deleteSelectedFolder() override;
    Q_INVOKABLE bool deleteNoteById(const QString& noteId);
    Q_INVOKABLE bool deleteNotesByIds(const QVariantList& noteIds);
    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const;

    void setResourcePaths(QStringList resourcePaths);
    QStringList resourcePaths() const;
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QStringList resourcePaths,
        QString resourcesFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestControllerHook();

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
    void resourceDeleted(const QString& noteId);
    void hubFilesystemMutated();
    void controllerHookRequested();

private:
    bool reloadFromResourcesFilePath(QString* errorMessage = nullptr);
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncModel();
    void syncDomainStoreFromItems();
    void refreshNoteListForSelection();
    void setResourceResolutionBasePaths(QStringList basePaths);

    QStringList m_resourcePaths;
    QStringList m_resourceResolutionBasePaths;
    QVector<ResourcesHierarchyItem> m_items;
    WhatSonResourcesHierarchyStore m_store;
    ResourcesHierarchyModel m_itemModel;
    ResourcesListModel m_noteListModel;
    int m_selectedIndex = -1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_resourcesFilePath;
};
