#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <QVariantList>

class LibraryHierarchyViewModel;
class SidebarHierarchyStore;
class TagsHierarchyViewModel;

class SidebarSelectionStore final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)
    Q_PROPERTY(QAbstractItemModel* itemModel READ itemModel NOTIFY itemModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(QStringList toolbarIconNames READ toolbarIconNames NOTIFY toolbarStateChanged)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarStateChanged)

public:
    explicit SidebarSelectionStore(
        SidebarHierarchyStore* sidebarStore,
        LibraryHierarchyViewModel* libraryViewModel,
        TagsHierarchyViewModel* tagsViewModel,
        QObject* parent = nullptr);
    ~SidebarSelectionStore() override;

    int activeIndex() const noexcept;
    void setActiveIndex(int index);

    QAbstractItemModel* itemModel() const noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    QStringList toolbarIconNames() const;
    QVariantList toolbarItems() const;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    signals  :


    void activeIndexChanged();
    void itemModelChanged();
    void selectedIndexChanged();
    void capabilitiesChanged();
    void toolbarStateChanged();

private:
    static constexpr int kLibrarySectionIndex = 0;
    static constexpr int kTagsSectionIndex = 3;

    void emitSelectionAndCapabilityUpdates();

    SidebarHierarchyStore* m_sidebarStore = nullptr;
    LibraryHierarchyViewModel* m_libraryViewModel = nullptr;
    TagsHierarchyViewModel* m_tagsViewModel = nullptr;
    QHash<int, int> m_genericSelectedIndexBySection;
};
