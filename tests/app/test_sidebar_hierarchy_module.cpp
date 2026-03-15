#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QObject>
#include <QVariant>
#include <QtTest/QtTest>

class SidebarHierarchyModuleTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void activeHierarchyIndex_mustRoundTripThroughSelectionStore();
    void activeHierarchyBindings_mustResolveThroughProviderInterface();
};

void SidebarHierarchyModuleTest::activeHierarchyIndex_mustRoundTripThroughSelectionStore()
{
    SidebarSelectionStore selectionStore;
    SidebarHierarchyViewModel sidebarViewModel;
    sidebarViewModel.setSelectionStore(&selectionStore);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), 0);
    QCOMPARE(selectionStore.selectedHierarchyIndex(), 0);

    sidebarViewModel.setActiveHierarchyIndex(2);
    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), 2);
    QCOMPARE(selectionStore.selectedHierarchyIndex(), 2);

    sidebarViewModel.setActiveHierarchyIndex(999);
    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), 0);
    QCOMPARE(selectionStore.selectedHierarchyIndex(), 0);
}

void SidebarHierarchyModuleTest::activeHierarchyBindings_mustResolveThroughProviderInterface()
{
    QObject libraryViewModel;
    QObject projectsViewModel;
    QObject bookmarksViewModel;
    QObject libraryNoteListModel;
    QObject bookmarksNoteListModel;

    libraryViewModel.setProperty("noteListModel", QVariant::fromValue(static_cast<QObject*>(&libraryNoteListModel)));
    bookmarksViewModel.setProperty(
        "noteListModel",
        QVariant::fromValue(static_cast<QObject*>(&bookmarksNoteListModel)));

    HierarchyViewModelProvider provider;
    HierarchyViewModelProvider::Targets targets;
    targets.libraryViewModel = &libraryViewModel;
    targets.projectsViewModel = &projectsViewModel;
    targets.bookmarksViewModel = &bookmarksViewModel;
    provider.setTargets(targets);

    SidebarSelectionStore selectionStore;
    SidebarHierarchyViewModel sidebarViewModel;
    sidebarViewModel.setSelectionStore(&selectionStore);
    sidebarViewModel.setViewModelProvider(&provider);

    sidebarViewModel.setActiveHierarchyIndex(0);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), &libraryViewModel);
    QCOMPARE(sidebarViewModel.activeNoteListModel(), &libraryNoteListModel);
    QCOMPARE(sidebarViewModel.property("resolvedActiveHierarchyIndex").toInt(), 0);
    QCOMPARE(sidebarViewModel.property("resolvedHierarchyViewModel").value<QObject*>(), &libraryViewModel);
    QCOMPARE(sidebarViewModel.property("resolvedNoteListModel").value<QObject*>(), &libraryNoteListModel);

    sidebarViewModel.setActiveHierarchyIndex(1);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), &projectsViewModel);
    QCOMPARE(sidebarViewModel.activeNoteListModel(), static_cast<QObject*>(nullptr));
    QCOMPARE(sidebarViewModel.property("resolvedActiveHierarchyIndex").toInt(), 1);
    QCOMPARE(sidebarViewModel.property("resolvedHierarchyViewModel").value<QObject*>(), &projectsViewModel);
    QCOMPARE(sidebarViewModel.property("resolvedNoteListModel").value<QObject*>(), static_cast<QObject*>(nullptr));

    sidebarViewModel.setActiveHierarchyIndex(2);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), &bookmarksViewModel);
    QCOMPARE(sidebarViewModel.activeNoteListModel(), &bookmarksNoteListModel);
    QCOMPARE(sidebarViewModel.property("resolvedActiveHierarchyIndex").toInt(), 2);
    QCOMPARE(sidebarViewModel.property("resolvedHierarchyViewModel").value<QObject*>(), &bookmarksViewModel);
    QCOMPARE(sidebarViewModel.property("resolvedNoteListModel").value<QObject*>(), &bookmarksNoteListModel);
}

QTEST_MAIN(SidebarHierarchyModuleTest)

#include "test_sidebar_hierarchy_module.moc"
