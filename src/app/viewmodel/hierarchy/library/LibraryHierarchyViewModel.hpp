#pragma once

#include "LibraryHierarchyModel.hpp"
#include "LibraryNoteListModel.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/library/LibraryDraft.hpp"
#include "file/hierarchy/library/LibraryToday.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "file/hub/WhatSonHubStore.hpp"

#include <QObject>
#include <QSet>
#include <QVariantList>
#include <QVector>

class LibraryHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(LibraryHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(LibraryNoteListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(int noteItemCount READ noteItemCount NOTIFY noteItemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool createTxtEnabled READ createTxtEnabled NOTIFY createTxtStateChanged)
    Q_PROPERTY(QString lastCreateTxtError READ lastCreateTxtError NOTIFY createTxtStateChanged)

public:
    explicit LibraryHierarchyViewModel(QObject* parent = nullptr);
    ~LibraryHierarchyViewModel() override;

    LibraryHierarchyModel* itemModel() noexcept;
    LibraryNoteListModel* noteListModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);
    int itemCount() const noexcept;
    int noteItemCount() const noexcept;
    bool loadSucceeded() const noexcept;
    QString lastLoadError() const;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;
    bool createTxtEnabled() const noexcept;
    QString lastCreateTxtError() const;
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

    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();
    Q_INVOKABLE bool createTxtFile();

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
    void itemCountChanged();
    void noteItemCountChanged();
    void loadStateChanged();
    void createTxtStateChanged();
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
        QString selectedLabelKey;
        QString selectedPathKey;
        QSet<QString> subtreeLabelKeys;
        QSet<QString> subtreePathKeys;
    };

    static int extractDepth(const QVariantMap& entryMap);
    static LibraryHierarchyItem parseItem(const QVariant& entry, int fallbackOrdinal);
    static int nextFolderSequence(const QVector<LibraryHierarchyItem>& items);
    static QVector<LibraryNoteListItem> buildNoteListItems(const QVector<LibraryNoteRecord>& notes);
    const QVector<LibraryNoteRecord>& notesForBucket(IndexedBucket bucket) const;
    IndexedBucket selectedBucket() const;
    FolderSelectionScope selectedFolderScope() const;
    static QString normalizeFolderKey(const QString& value);
    QString folderPathForIndex(int index) const;
    static bool noteMatchesFolderScope(const LibraryNoteRecord& note, const FolderSelectionScope& scope);
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
    LibraryAll m_libraryAll;
    LibraryDraft m_libraryDraft;
    LibraryToday m_libraryToday;
    QVector<IndexedBucketRange> m_bucketRanges;
    bool m_runtimeIndexLoaded = false;
    bool m_foldersHierarchyLoaded = false;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    int m_noteItemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_foldersFilePath;
    WhatSonHubStore m_hubStore;
    QString m_lastCreateTxtError;
};
