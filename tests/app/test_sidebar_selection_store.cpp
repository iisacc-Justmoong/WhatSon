#include "sidebar/SidebarHierarchyStore.hpp"
#include "sidebar/SidebarSelectionStore.hpp"

#include "hierarchy/library/LibraryHierarchyModel.hpp"
#include "hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "hierarchy/tags/TagsHierarchyModel.hpp"
#include "hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <QtTest>

class SidebarSelectionStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaultState_usesLibraryModelAndToolbarSelection();
    void activeIndex_tags_switchesModelAndDisablesFolderActions();
    void activeIndex_projects_usesStaticSectionModel();
};

void SidebarSelectionStoreTest::defaultState_usesLibraryModelAndToolbarSelection()
{
    SidebarHierarchyStore hierarchyStore;
    LibraryHierarchyViewModel libraryViewModel;
    TagsHierarchyViewModel tagsViewModel;
    SidebarSelectionStore selectionStore(&hierarchyStore, &libraryViewModel, &tagsViewModel);

    QCOMPARE(selectionStore.activeIndex(), 0);
    QVERIFY(selectionStore.itemModel() == libraryViewModel.itemModel());
    QVERIFY(selectionStore.renameEnabled());
    QVERIFY(selectionStore.createFolderEnabled());
    QVERIFY(!selectionStore.deleteFolderEnabled());

    const QVariantList toolbarItems = selectionStore.toolbarItems();
    QCOMPARE(toolbarItems.size(), hierarchyStore.toolbarIconNames().size());
    QCOMPARE(toolbarItems.at(0).toMap().value(QStringLiteral("selected")).toBool(), true);
}

void SidebarSelectionStoreTest::activeIndex_tags_switchesModelAndDisablesFolderActions()
{
    SidebarHierarchyStore hierarchyStore;
    LibraryHierarchyViewModel libraryViewModel;
    TagsHierarchyViewModel tagsViewModel;
    SidebarSelectionStore selectionStore(&hierarchyStore, &libraryViewModel, &tagsViewModel);

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

void SidebarSelectionStoreTest::activeIndex_projects_usesStaticSectionModel()
{
    SidebarHierarchyStore hierarchyStore;
    LibraryHierarchyViewModel libraryViewModel;
    TagsHierarchyViewModel tagsViewModel;
    SidebarSelectionStore selectionStore(&hierarchyStore, &libraryViewModel, &tagsViewModel);

    selectionStore.setActiveIndex(1);
    QCOMPARE(selectionStore.activeIndex(), 1);
    QVERIFY(selectionStore.itemModel() == hierarchyStore.itemModel());
    QVERIFY(selectionStore.itemModel() == hierarchyStore.itemModelForSection(1));
    QVERIFY(selectionStore.itemModel() != hierarchyStore.itemModelForSection(2));
    QVERIFY(selectionStore.itemModel() != libraryViewModel.itemModel());
    QVERIFY(selectionStore.itemModel() != tagsViewModel.itemModel());

    selectionStore.setSelectedIndex(2);
    QCOMPARE(selectionStore.selectedIndex(), 2);
    QCOMPARE(selectionStore.itemLabel(0), QStringLiteral("Projects"));

    selectionStore.createFolder();
    selectionStore.deleteSelectedFolder();

    QCOMPARE(selectionStore.itemModel()->rowCount(), 10);
}

QTEST_APPLESS_MAIN(SidebarSelectionStoreTest)

#include "test_sidebar_selection_store.moc"
