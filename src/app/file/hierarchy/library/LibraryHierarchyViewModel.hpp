#pragma once

#include "LibraryHierarchyModel.hpp"
#include "LibraryAll.hpp"
#include "LibraryDraft.hpp"
#include "LibraryToday.hpp"

#include <QObject>
#include <QVariantList>
#include <QVector>

class LibraryHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(LibraryHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    explicit LibraryHierarchyViewModel(QObject* parent = nullptr);
    ~LibraryHierarchyViewModel() override;

    LibraryHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);

    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    signals  :


    void selectedIndexChanged();

private:
    static int extractDepth(const QVariantMap& entryMap);
    static LibraryHierarchyItem parseItem(const QVariant& entry, int fallbackOrdinal);
    static int nextFolderSequence(const QVector<LibraryHierarchyItem>& items);
    void syncModel();

    QVector<LibraryHierarchyItem> m_items;
    LibraryHierarchyModel m_itemModel;
    LibraryAll m_libraryAll;
    LibraryDraft m_libraryDraft;
    LibraryToday m_libraryToday;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
};
