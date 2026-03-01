#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <QtTest>

class HierarchyViewModelsTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void libraryViewModel_supportsCrudContract();
    void projectsViewModel_supportsCrudContract();
    void bookmarksViewModel_supportsCrudContract();
    void resourcesViewModel_supportsCrudContract();
    void progressViewModel_supportsCrudContract();
    void eventViewModel_supportsCrudContract();
    void presetViewModel_supportsCrudContract();
    void tagsViewModel_supportsCrudContract();
};

void HierarchyViewModelsTest::libraryViewModel_supportsCrudContract()
{
    LibraryHierarchyViewModel viewModel;
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QVERIFY(!viewModel.deleteFolderEnabled());

    viewModel.setDepthItems(QVariantList{
        QVariantMap{{"label", QStringLiteral("Root")}, {"depth", 0}}
    });
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("RenamedRoot")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("RenamedRoot"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);

    viewModel.setDepthItems(QVariantList{
        QVariantMap{{"label", QStringLiteral("Library (1)")}, {"depth", 0}, {"accent", true}},
        QVariantMap{{"label", QStringLiteral("Leaf")}, {"depth", 1}, {"accent", false}}
    });
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("BlockedHeaderRename")));
}

void HierarchyViewModelsTest::projectsViewModel_supportsCrudContract()
{
    ProjectsHierarchyViewModel viewModel;
    viewModel.setProjectNames({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Projects-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Alpha-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Alpha-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
}

void HierarchyViewModelsTest::bookmarksViewModel_supportsCrudContract()
{
    BookmarksHierarchyViewModel viewModel;
    viewModel.setBookmarkIds({QStringLiteral("bookmark://a")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Bookmarks-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("bookmark://renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("bookmark://renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::resourcesViewModel_supportsCrudContract()
{
    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({QStringLiteral("assets/logo.png")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Resources-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("assets/logo-renamed.png")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("assets/logo-renamed.png"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::progressViewModel_supportsCrudContract()
{
    ProgressHierarchyViewModel viewModel;
    viewModel.setProgressState(0, {QStringLiteral("Ready"), QStringLiteral("Done")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Progress-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Ready-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Ready-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
}

void HierarchyViewModelsTest::eventViewModel_supportsCrudContract()
{
    EventHierarchyViewModel viewModel;
    viewModel.setEventNames({QStringLiteral("Kickoff")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Event-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Kickoff-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Kickoff-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::presetViewModel_supportsCrudContract()
{
    PresetHierarchyViewModel viewModel;
    viewModel.setPresetNames({QStringLiteral("Executive Summary")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Preset-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Executive-Summary-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Executive-Summary-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::tagsViewModel_supportsCrudContract()
{
    TagsHierarchyViewModel viewModel;
    viewModel.setTagDepthEntries({
        {QStringLiteral("brand"), QStringLiteral("Brand"), 0},
        {QStringLiteral("brand/social"), QStringLiteral("Social"), 1}
    });
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Social-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Social-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

QTEST_APPLESS_MAIN(HierarchyViewModelsTest)

#include "test_hierarchy_viewmodels.moc"
