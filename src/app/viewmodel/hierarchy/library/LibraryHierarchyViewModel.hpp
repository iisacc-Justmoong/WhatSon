#pragma once

#include "LibraryHierarchyModel.hpp"
#include "LibraryNoteListModel.hpp"
#include "file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "file/hierarchy/library/WhatSonLibraryIndexedState.hpp"
#include "file/hub/WhatSonHubStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QVariantList>
#include <QVector>

class ISystemCalendarStore;

class LibraryHierarchyViewModel final : public IHierarchyViewModel,
                                        public IHierarchyRenameCapability,
                                        public IHierarchyCrudCapability,
                                        public IHierarchyExpansionCapability,
                                        public IHierarchyReorderCapability,
                                        public IHierarchyNoteDropCapability,
                                        public ILibraryNoteMutationCapability
{
    Q_OBJECT
    Q_INTERFACES(
        IHierarchyRenameCapability
        IHierarchyCrudCapability
        IHierarchyExpansionCapability
        IHierarchyReorderCapability
        IHierarchyNoteDropCapability
        ILibraryNoteMutationCapability)

    Q_PROPERTY(LibraryHierarchyModel* itemModel READ itemModel CONSTANT)
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
    explicit LibraryHierarchyViewModel(QObject* parent = nullptr);
    ~LibraryHierarchyViewModel() override;

    LibraryHierarchyModel* itemModel() noexcept override;
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
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;
    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        const QString& wshubPath,
        QVector<LibraryNoteRecord> allNotes,
        QVector<LibraryNoteRecord> draftNotes,
        QVector<LibraryNoteRecord> todayNotes,
        QVector<WhatSonFolderDepthEntry> folderEntries,
        QString foldersFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

    Q_INVOKABLE void createFolder() override;
    Q_INVOKABLE void deleteSelectedFolder() override;
    Q_INVOKABLE bool canMoveFolder(int index) const;
    Q_INVOKABLE bool canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const;
    Q_INVOKABLE bool moveFolderBefore(int sourceIndex, int targetIndex);
    Q_INVOKABLE bool canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const;
    Q_INVOKABLE bool moveFolder(int sourceIndex, int targetIndex, bool asChild);
    Q_INVOKABLE bool canMoveFolderToRoot(int sourceIndex) const;
    Q_INVOKABLE bool moveFolderToRoot(int sourceIndex);
    Q_INVOKABLE bool canAcceptNoteDrop(int index, const QString& noteId) const override;
    Q_INVOKABLE bool assignNoteToFolder(int index, const QString& noteId) override;
    Q_INVOKABLE bool applyHierarchyNodes(
        const QVariantList& hierarchyNodes,
        const QString& activeItemKey = QString()) override;
    Q_INVOKABLE bool createEmptyNote() override;
    Q_INVOKABLE bool clearNoteFoldersById(const QString& noteId) override;
    Q_INVOKABLE bool deleteNoteById(const QString& noteId) override;
    Q_INVOKABLE bool applyPersistedBodyStateForNote(
        const QString& noteId,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt);
    Q_INVOKABLE bool requestTrackedStatisticsRefreshForNote(const QString& noteId, bool incrementOpenCount);
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool saveCurrentBodyText(const QString& text);
    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const;
    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId);
    Q_INVOKABLE bool activateNoteById(const QString& noteId);
    QVector<LibraryNoteRecord> indexedNotesSnapshot() const;
    bool indexedNoteRecordById(const QString& noteId, LibraryNoteRecord* outNote) const;
    bool supportsHierarchyNodeReorder() const noexcept override;
    bool supportsHierarchyNoteDrop() const noexcept override;

    void setSystemCalendarStore(ISystemCalendarStore* store);
    ISystemCalendarStore* systemCalendarStore() const noexcept;
    void setHubStore(WhatSonHubStore store);
    WhatSonHubStore hubStore() const;

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
    void noteItemCountChanged();
    void loadStateChanged();
    void noteDeleted(const QString& noteId);
    void emptyNoteCreated(const QString& noteId);
    void indexedNoteUpserted(const QString& noteId);
    void indexedNotesSnapshotChanged();
    void hubFilesystemMutated();
    void viewModelHookRequested();

private:
    enum class IndexedBucket
    {
        All,
        Draft,
        Today
    };

    struct IndexedBucketRange
    {
        IndexedBucket bucket = IndexedBucket::All;
        int startRow = 0;
        int endRow = -1;
    };

    struct FolderSelectionScope final
    {
        QString selectedFolderUuid;
    };

    static int extractDepth(const QVariantMap& entryMap);
    static LibraryHierarchyItem parseItem(const QVariant& entry, int fallbackOrdinal);
    static int nextFolderSequence(const QVector<LibraryHierarchyItem>& items);
    LibraryNoteListItem buildNoteListItem(const LibraryNoteRecord& note, const QStringList& folderLabels) const;
    QVector<LibraryNoteListItem> buildNoteListItems(const QVector<LibraryNoteRecord>& notes) const;
    QVector<LibraryNoteListItem> buildFolderScopedNoteListItems(const FolderSelectionScope& scope) const;
    const QVector<LibraryNoteRecord>& notesForBucket(IndexedBucket bucket) const;
    const IndexedBucketRange* bucketRangeForIndex(int index) const noexcept;
    IndexedBucket selectedBucket() const;
    FolderSelectionScope selectedFolderScope() const;
    static QString normalizeFolderKey(const QString& value);
    QString folderPathForIndex(int index) const;
    QString folderUuidForIndex(int index) const;
    bool upsertIndexedNote(const LibraryNoteRecord& note);
    bool removeIndexedNoteById(const QString& noteId);
    void invalidateNoteListItemCache() const;
    void invalidateNoteListItemCacheForNoteId(const QString& noteId) const;
    void setIndexedStateNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes);
    void applyIndexedStateSnapshot(
        QString wshubPath,
        QVector<LibraryNoteRecord> allNotes,
        QVector<LibraryNoteRecord> draftNotes,
        QVector<LibraryNoteRecord> todayNotes);
    bool loadIndexedStateFromWshub(const QString& wshubPath, QString* errorMessage);
    bool commitFolderHierarchyUpdate(
        QVector<LibraryHierarchyItem> stagedItems,
        int selectedIndex,
        const QHash<QString, QString>& movedFolderPathMap = {});
    void applySelectedIndex(int index, bool forceReapply = false);
    int firstEditableInsertIndex() const noexcept;
    void rebuildBucketRanges();
    void refreshNoteListForSelection();
    void applyIndexedBuckets();
    void updateItemCount();
    void updateNoteItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncModel();

    QVector<LibraryHierarchyItem> m_items;
    LibraryHierarchyModel m_itemModel;
    LibraryNoteListModel m_noteListModel;
    WhatSonLibraryIndexedState m_indexedState;
    QVector<IndexedBucketRange> m_bucketRanges;
    mutable QHash<QString, LibraryNoteListItem> m_noteListItemCache;
    mutable bool m_noteListItemCacheUsesFoldersHierarchy = false;
    bool m_runtimeIndexLoaded = false;
    bool m_foldersHierarchyLoaded = false;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    int m_noteItemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_foldersFilePath;
    QPointer<ISystemCalendarStore> m_systemCalendarStore;
    QMetaObject::Connection m_systemCalendarStoreChangedConnection;
    WhatSonHubStore m_hubStore;
};
