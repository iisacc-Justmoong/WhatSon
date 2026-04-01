#pragma once

#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyModel.hpp"

#include <QStringList>
#include <QVariantList>
#include <QVector>

class ProgressHierarchyViewModel final : public IHierarchyViewModel,
                                         public IHierarchyRenameCapability,
                                         public IHierarchyCrudCapability,
                                         public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

    Q_PROPERTY(ProgressHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(LibraryNoteListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(int noteItemCount READ noteItemCount NOTIFY noteItemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ProgressHierarchyViewModel(QObject* parent = nullptr);
    ~ProgressHierarchyViewModel() override;

    ProgressHierarchyModel* itemModel() noexcept override;
    LibraryNoteListModel* noteListModel() noexcept override;

    int selectedIndex() const noexcept override;
    Q_INVOKABLE void setSelectedIndex(int index) override;
    int itemCount() const noexcept override;
    int noteItemCount() const noexcept;
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

    void setProgressState(int progressValue, QStringList progressStates);
    int progressValue() const noexcept;
    QStringList progressStates() const;
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool saveCurrentBodyText(const QString& text);
    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const;
    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId);

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        int progressValue,
        QStringList progressStates,
        QString progressFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestViewModelHook();

signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void noteItemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();
    void hubFilesystemMutated();

private:
    bool reloadFromProgressFilePath(QString* errorMessage = nullptr);
    void updateItemCount();
    void updateNoteItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    LibraryNoteListItem buildNoteListItem(const LibraryNoteRecord& note) const;
    void refreshNoteListForSelection();
    bool refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    bool refreshIndexedNotesFromProgressFilePath(QString* errorMessage = nullptr);
    int selectedProgressFilterValue() const noexcept;
    void rebuildItems();
    void syncProgressStore();
    void syncProgressStatesFromItems();
    void syncModel();

    int m_progressValue = 0;
    QStringList m_progressStates;
    QVector<ProgressHierarchyItem> m_items;
    WhatSonProgressHierarchyStore m_store;
    ProgressHierarchyModel m_itemModel;
    LibraryNoteListModel m_noteListModel;
    QVector<LibraryNoteRecord> m_allNotes;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    int m_noteItemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_progressFilePath;
};
