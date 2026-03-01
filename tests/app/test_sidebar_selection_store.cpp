#include "sidebar/SidebarSelectionStore.hpp"

#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <QtTest>

class SidebarSelectionStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaultState_usesLibraryModelAndToolbarSelection();
    void activeIndex_tags_switchesModelAndDisablesFolderActions();
    void activeIndex_projects_usesProjectsViewModel();
};

void SidebarSelectionStoreTest::defaultState_usesLibraryModelAndToolbarSelection()
{
    LibraryHierarchyViewModel libraryViewModel;
    ProjectsHierarchyViewModel projectsViewModel;
    BookmarksHierarchyViewModel bookmarksViewModel;
    TagsHierarchyViewModel tagsViewModel;
    ResourcesHierarchyViewModel resourcesViewModel;
    ProgressHierarchyViewModel progressViewModel;
    EventHierarchyViewModel eventViewModel;
    PresetHierarchyViewModel presetViewModel;
    SidebarSelectionStore selectionStore(
        &libraryViewModel,
        &projectsViewModel,
        &bookmarksViewModel,
        &tagsViewModel,
        &resourcesViewModel,
        &progressViewModel,
        &eventViewModel,
        &presetViewModel);

    QCOMPARE(selectionStore.activeIndex(), 0);
    QVERIFY(selectionStore.itemModel() == libraryViewModel.itemModel());
    QVERIFY(selectionStore.renameEnabled());
    QVERIFY(selectionStore.createFolderEnabled());
    QVERIFY(!selectionStore.deleteFolderEnabled());

    const QVariantList toolbarItems = selectionStore.toolbarItems();
    QCOMPARE(toolbarItems.size(), selectionStore.toolbarIconNames().size());
    QCOMPARE(toolbarItems.size(), 8);
    QCOMPARE(toolbarItems.at(0).toMap().value(QStringLiteral("selected")).toBool(), true);
}

void SidebarSelectionStoreTest::activeIndex_tags_switchesModelAndDisablesFolderActions()
{
    LibraryHierarchyViewModel libraryViewModel;
    ProjectsHierarchyViewModel projectsViewModel;
    BookmarksHierarchyViewModel bookmarksViewModel;
    TagsHierarchyViewModel tagsViewModel;
    ResourcesHierarchyViewModel resourcesViewModel;
    ProgressHierarchyViewModel progressViewModel;
    EventHierarchyViewModel eventViewModel;
    PresetHierarchyViewModel presetViewModel;
    SidebarSelectionStore selectionStore(
        &libraryViewModel,
        &projectsViewModel,
        &bookmarksViewModel,
        &tagsViewModel,
        &resourcesViewModel,
        &progressViewModel,
        &eventViewModel,
        &presetViewModel);

    tagsViewModel.setTagDepthEntries({
        {QStringLiteral("tag-root"), QStringLiteral("TagRoot"), 0}
    });

    selectionStore.setActiveIndex(3);

    QCOMPARE(selectionStore.activeIndex(), 3);
    QVERIFY(selectionStore.itemModel() == tagsViewModel.itemModel());
    QVERIFY(!selectionStore.renameEnabled());
    QVERIFY(!selectionStore.createFolderEnabled());
    QVERIFY(!selectionStore.deleteFolderEnabled());

    selectionStore.setSelectedIndex(0);
    QCOMPARE(selectionStore.selectedIndex(), 0);

    const QVariantList toolbarItems = selectionStore.toolbarItems();
    QCOMPARE(toolbarItems.at(3).toMap().value(QStringLiteral("selected")).toBool(), true);
}

void SidebarSelectionStoreTest::activeIndex_projects_usesProjectsViewModel()
{
    LibraryHierarchyViewModel libraryViewModel;
    ProjectsHierarchyViewModel projectsViewModel;
    BookmarksHierarchyViewModel bookmarksViewModel;
    TagsHierarchyViewModel tagsViewModel;
    ResourcesHierarchyViewModel resourcesViewModel;
    ProgressHierarchyViewModel progressViewModel;
    EventHierarchyViewModel eventViewModel;
    PresetHierarchyViewModel presetViewModel;
    SidebarSelectionStore selectionStore(
        &libraryViewModel,
        &projectsViewModel,
        &bookmarksViewModel,
        &tagsViewModel,
        &resourcesViewModel,
        &progressViewModel,
        &eventViewModel,
        &presetViewModel);

    projectsViewModel.setProjectNames({QStringLiteral("Alpha"), QStringLiteral("Beta")});

    selectionStore.setActiveIndex(1);
    QCOMPARE(selectionStore.activeIndex(), 1);
    QVERIFY(selectionStore.itemModel() == projectsViewModel.itemModel());
    QVERIFY(selectionStore.itemModel() != libraryViewModel.itemModel());
    QVERIFY(selectionStore.itemModel() != tagsViewModel.itemModel());

    selectionStore.setSelectedIndex(1);
    QCOMPARE(selectionStore.selectedIndex(), 1);
    QCOMPARE(selectionStore.itemLabel(0), QStringLiteral("Projects (2)"));

    selectionStore.createFolder();
    selectionStore.deleteSelectedFolder();

    QCOMPARE(selectionStore.itemModel()->rowCount(), 3);
}

QTEST_APPLESS_MAIN(SidebarSelectionStoreTest)

#include "test_sidebar_selection_store.moc"
