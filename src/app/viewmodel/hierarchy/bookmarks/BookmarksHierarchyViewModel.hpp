#pragma once

#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyModel.hpp"

#include <QObject>
#include <QVariantList>
#include <QVector>

class BookmarksHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(BookmarksHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(BookmarksNoteListModel* noteListModel READ noteListModel CONSTANT)
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

    BookmarksHierarchyModel* itemModel() noexcept;
    BookmarksNoteListModel* noteListModel() noexcept;

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
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();
    Q_INVOKABLE bool saveCurrentBodyText(const QString& text);

    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;
    bool viewOptionsEnabled() const noexcept;

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
    void itemCountChanged();
    void noteItemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    void updateItemCount();
    void updateNoteItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void rebuildColorFolders();
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
};
