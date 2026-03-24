#pragma once

#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyModel.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QPointer>
#include <QVariantList>
#include <QVector>

class SystemCalendarStore;

class BookmarksHierarchyViewModel final : public IHierarchyViewModel,
                                          public IHierarchyRenameCapability,
                                          public IHierarchyCrudCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability)

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
    explicit BookmarksHierarchyViewModel(QObject* parent = nullptr);
    ~BookmarksHierarchyViewModel() override;

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
    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();
    bool removeNoteById(const QString& noteId);
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool saveCurrentBodyText(const QString& text);

    void setSystemCalendarStore(SystemCalendarStore* store);
    SystemCalendarStore* systemCalendarStore() const noexcept;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;
    bool viewOptionsEnabled() const noexcept override;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QVector<LibraryNoteRecord> bookmarkedNotes,
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
    void noteItemCountChanged();
    void loadStateChanged();
    void hubFilesystemMutated();
    void viewModelHookRequested();

private:
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
    QPointer<SystemCalendarStore> m_systemCalendarStore;
    QMetaObject::Connection m_systemCalendarStoreChangedConnection;
};
