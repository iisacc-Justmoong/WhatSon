#pragma once

#include "LibraryHierarchyModel.hpp"
#include "LibraryNoteListModel.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/library/LibraryDraft.hpp"
#include "file/hierarchy/library/LibraryToday.hpp"

#include <QObject>
#include <QVariantList>
#include <QVector>

class LibraryHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(LibraryHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(LibraryNoteListModel* noteListModel READ noteListModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    explicit LibraryHierarchyViewModel(QObject* parent = nullptr);
    ~LibraryHierarchyViewModel() override;

    LibraryHierarchyModel* itemModel() noexcept;
    LibraryNoteListModel* noteListModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    signals  :



    void selectedIndexChanged();

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

    static int extractDepth(const QVariantMap& entryMap);
    static LibraryHierarchyItem parseItem(const QVariant& entry, int fallbackOrdinal);
    static int nextFolderSequence(const QVector<LibraryHierarchyItem>& items);
    static QVector<LibraryNoteListItem> buildNoteListItems(
        const QVector<LibraryNoteRecord>& notes,
        const QString& highlightedNoteId);
    const QVector<LibraryNoteRecord>& notesForBucket(IndexedBucket bucket) const;
    IndexedBucket selectedBucket() const;
    QString selectedNoteId() const;
    void rebuildBucketRanges();
    void refreshNoteListForSelection();
    void applyIndexedBuckets();
    void syncModel();

    QVector<LibraryHierarchyItem> m_items;
    LibraryHierarchyModel m_itemModel;
    LibraryNoteListModel m_noteListModel;
    LibraryAll m_libraryAll;
    LibraryDraft m_libraryDraft;
    LibraryToday m_libraryToday;
    QVector<IndexedBucketRange> m_bucketRanges;
    QVector<QString> m_rowNoteIds;
    bool m_runtimeIndexLoaded = false;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
};
