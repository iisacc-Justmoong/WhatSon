#pragma once

#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"
#include "app/models/file/hierarchy/IHierarchyCapabilities.hpp"
#include "app/models/file/hierarchy/bookmarks/BookmarksNoteListModel.hpp"
#include "app/models/file/hierarchy/bookmarks/BookmarksHierarchyModel.hpp"
#include "app/models/file/hierarchy/IHierarchyController.hpp"

#include <QPointer>
#include <QVariantList>
#include <QVector>

class ISystemCalendarStore;

class BookmarksHierarchyController final : public IHierarchyController,
                                          public IHierarchyRenameCapability,
                                          public IHierarchyCrudCapability,
                                          public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

    Q_PROPERTY(BookmarksHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(BookmarksNoteListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(int noteItemCount READ noteItemCount NOTIFY noteItemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool viewOptionsEnabled READ viewOptionsEnabled CONSTANT)

public:
    explicit BookmarksHierarchyController(QObject* parent = nullptr);
    ~BookmarksHierarchyController() override;

    BookmarksHierarchyModel* itemModel() noexcept override;
    BookmarksNoteListModel* noteListModel() noexcept override;

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
    bool removeNoteById(const QString& noteId);
    Q_INVOKABLE bool applyPersistedBodyStateForNote(
        const QString& noteId,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt);
    Q_INVOKABLE bool requestTrackedStatisticsRefreshForNote(const QString& noteId, bool incrementOpenCount);
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool saveCurrentBodyText(const QString& text);
    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const;
    Q_INVOKABLE QString noteBodySourceTextForNoteId(const QString& noteId) const;
    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId);

    void setSystemCalendarStore(ISystemCalendarStore* store);
    ISystemCalendarStore* systemCalendarStore() const noexcept;
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;
    bool viewOptionsEnabled() const noexcept override;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QVector<LibraryNoteRecord> bookmarkedNotes,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestControllerHook();

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void noteItemCountChanged();
    void loadStateChanged();
    void hubFilesystemMutated();
    void controllerHookRequested();

private:
    bool reloadFromWshubPath(QString* errorMessage = nullptr);
    void updateItemCount();
    void updateNoteItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void rebuildColorFolders();
    BookmarksNoteListItem buildBookmarksListItem(const LibraryNoteRecord& note) const;
    void refreshNoteListForSelection();
    QString selectedColorLabel() const;
    void syncModel();

    QVector<BookmarksHierarchyItem> m_items;
    QVector<LibraryNoteRecord> m_bookmarkedNotes;
    BookmarksHierarchyModel m_itemModel;
    BookmarksNoteListModel m_noteListModel;
    int m_selectedIndex = -1;
    int m_itemCount = 0;
    int m_noteItemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_wshubPath;
    QPointer<ISystemCalendarStore> m_systemCalendarStore;
    QMetaObject::Connection m_systemCalendarStoreChangedConnection;
};
