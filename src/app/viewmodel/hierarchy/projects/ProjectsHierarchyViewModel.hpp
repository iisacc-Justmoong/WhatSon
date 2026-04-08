#pragma once

#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyModel.hpp"

#include <QStringList>
#include <QVariantList>
#include <QVector>

class ProjectsHierarchyViewModel final : public IHierarchyViewModel,
                                         public IHierarchyRenameCapability,
                                         public IHierarchyCrudCapability,
                                         public IHierarchyExpansionCapability,
                                         public IHierarchyReorderCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability IHierarchyReorderCapability)

    Q_PROPERTY(ProjectsHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(LibraryNoteListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ProjectsHierarchyViewModel(QObject* parent = nullptr);
    ~ProjectsHierarchyViewModel() override;

    ProjectsHierarchyModel* itemModel() noexcept override;
    LibraryNoteListModel* noteListModel() noexcept override;

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
    Q_INVOKABLE bool setAllItemsExpanded(bool expanded);
    Q_INVOKABLE void createFolder() override;
    Q_INVOKABLE void deleteSelectedFolder() override;
    Q_INVOKABLE bool canMoveFolder(int index) const;
    Q_INVOKABLE bool canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const;
    Q_INVOKABLE bool moveFolderBefore(int sourceIndex, int targetIndex);
    Q_INVOKABLE bool canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const;
    Q_INVOKABLE bool moveFolder(int sourceIndex, int targetIndex, bool asChild);
    Q_INVOKABLE bool canMoveFolderToRoot(int sourceIndex) const;
    Q_INVOKABLE bool moveFolderToRoot(int sourceIndex);
    Q_INVOKABLE bool applyHierarchyNodes(
        const QVariantList& hierarchyNodes,
        const QString& activeItemKey = QString()) override;
    Q_INVOKABLE bool requestTrackedStatisticsRefreshForNote(const QString& noteId, bool incrementOpenCount);
    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const;
    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId);
    bool supportsHierarchyNodeReorder() const noexcept override;

    void setProjectNames(QStringList projectNames);
    QStringList projectNames() const;
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QVector<WhatSonFolderDepthEntry> projectEntries,
        QString projectsFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestViewModelHook();

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    LibraryNoteListItem buildNoteListItem(const LibraryNoteRecord& note) const;
    void refreshNoteListForSelection();
    bool refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    bool refreshIndexedNotesFromProjectsFilePath(QString* errorMessage = nullptr);
    void syncModel();
    bool commitHierarchyUpdate(QVector<ProjectsHierarchyItem> stagedItems, int selectedIndex);
    void syncDomainStoreFromItems();

    QStringList m_projectNames;
    QVector<ProjectsHierarchyItem> m_items;
    WhatSonProjectsHierarchyStore m_store;
    ProjectsHierarchyModel m_itemModel;
    LibraryNoteListModel m_noteListModel;
    QVector<LibraryNoteRecord> m_allNotes;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_projectsFilePath;
};
