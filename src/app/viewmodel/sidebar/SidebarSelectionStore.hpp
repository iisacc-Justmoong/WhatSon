#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QStringList>
#include <QVariantList>

class BookmarksHierarchyViewModel;
class EventHierarchyViewModel;
class LibraryHierarchyViewModel;
class PresetHierarchyViewModel;
class ProgressHierarchyViewModel;
class ProjectsHierarchyViewModel;
class ResourcesHierarchyViewModel;
class TagsHierarchyViewModel;

class SidebarSelectionStore final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)
    Q_PROPERTY(QAbstractItemModel* itemModel READ itemModel NOTIFY itemModelChanged)
    Q_PROPERTY(QAbstractItemModel* listItemModel READ listItemModel NOTIFY listItemModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY capabilitiesChanged)
    Q_PROPERTY(QStringList toolbarIconNames READ toolbarIconNames NOTIFY toolbarStateChanged)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarStateChanged)

public:
    explicit SidebarSelectionStore(
        LibraryHierarchyViewModel* libraryViewModel,
        ProjectsHierarchyViewModel* projectsViewModel,
        BookmarksHierarchyViewModel* bookmarksViewModel,
        TagsHierarchyViewModel* tagsViewModel,
        ResourcesHierarchyViewModel* resourcesViewModel,
        ProgressHierarchyViewModel* progressViewModel,
        EventHierarchyViewModel* eventViewModel,
        PresetHierarchyViewModel* presetViewModel,
        QObject* parent = nullptr);
    ~SidebarSelectionStore() override;

    int activeIndex() const noexcept;
    void setActiveIndex(int index);

    QAbstractItemModel* itemModel() const noexcept;
    QAbstractItemModel* listItemModel() const noexcept;

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
    void listItemModelChanged();
    void selectedIndexChanged();
    void capabilitiesChanged();
    void toolbarStateChanged();

private:
    static constexpr int kLibrarySectionIndex = 0;
    static constexpr int kProjectsSectionIndex = 1;
    static constexpr int kBookmarksSectionIndex = 2;
    static constexpr int kTagsSectionIndex = 3;
    static constexpr int kResourcesSectionIndex = 4;
    static constexpr int kProgressSectionIndex = 5;
    static constexpr int kEventSectionIndex = 6;
    static constexpr int kPresetSectionIndex = 7;
    static constexpr int kSectionCount = 8;

    void emitSelectionAndCapabilityUpdates();

    LibraryHierarchyViewModel* m_libraryViewModel = nullptr;
    ProjectsHierarchyViewModel* m_projectsViewModel = nullptr;
    BookmarksHierarchyViewModel* m_bookmarksViewModel = nullptr;
    TagsHierarchyViewModel* m_tagsViewModel = nullptr;
    ResourcesHierarchyViewModel* m_resourcesViewModel = nullptr;
    ProgressHierarchyViewModel* m_progressViewModel = nullptr;
    EventHierarchyViewModel* m_eventViewModel = nullptr;
    PresetHierarchyViewModel* m_presetViewModel = nullptr;
    int m_activeIndex = kLibrarySectionIndex;
};
