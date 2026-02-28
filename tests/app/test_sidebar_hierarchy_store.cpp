#include "sidebar/HierarchyItemListModel.hpp"
#include "sidebar/SidebarHierarchyStore.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

class SidebarHierarchyStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaultState_isLibrarySection();
    void setActiveIndex_clampsRangeAndEmitsSignal();
    void metadata_matchesSectionModels();
};

void SidebarHierarchyStoreTest::defaultState_isLibrarySection()
{
    SidebarHierarchyStore store;
    QCOMPARE(store.activeIndex(), 0);
    QCOMPARE(store.sectionCount(), 8);

    HierarchyItemListModel* model = store.itemModel();
    QVERIFY(model != nullptr);
    QCOMPARE(model->rowCount(), 10);

    const QModelIndex firstIndex = model->index(0, 0);
    QCOMPARE(model->data(firstIndex, HierarchyItemListModel::LabelRole).toString(), QStringLiteral("Library"));
    QCOMPARE(model->data(firstIndex, HierarchyItemListModel::IndentLevelRole).toInt(), 0);
}

void SidebarHierarchyStoreTest::setActiveIndex_clampsRangeAndEmitsSignal()
{
    SidebarHierarchyStore store;
    QSignalSpy activeIndexSpy(&store, &SidebarHierarchyStore::activeIndexChanged);

    store.setActiveIndex(999);
    QCOMPARE(store.activeIndex(), 7);
    QCOMPARE(activeIndexSpy.count(), 1);
    QCOMPARE(
        store.itemModel()->data(store.itemModel()->index(0, 0), HierarchyItemListModel::LabelRole).toString(),
        QStringLiteral("Presets"));

    store.setActiveIndex(-999);
    QCOMPARE(store.activeIndex(), 0);
    QCOMPARE(activeIndexSpy.count(), 2);
    QCOMPARE(
        store.itemModel()->data(store.itemModel()->index(0, 0), HierarchyItemListModel::LabelRole).toString(),
        QStringLiteral("Library"));

    store.setActiveIndex(0);
    QCOMPARE(activeIndexSpy.count(), 2);
}

void SidebarHierarchyStoreTest::metadata_matchesSectionModels()
{
    SidebarHierarchyStore store;

    const QStringList expectedSections = {
        QStringLiteral("Library"),
        QStringLiteral("Projects"),
        QStringLiteral("Bookmarks"),
        QStringLiteral("Tags"),
        QStringLiteral("Resources"),
        QStringLiteral("Progress"),
        QStringLiteral("Event"),
        QStringLiteral("Preset")
    };
    QCOMPARE(store.sectionNames(), expectedSections);

    const QStringList expectedIcons = {
        QStringLiteral("nodeslibraryFolder"),
        QStringLiteral("table"),
        QStringLiteral("bookmarksbookmarksList"),
        QStringLiteral("vcscurrentBranch"),
        QStringLiteral("imageToImage"),
        QStringLiteral("chartBar"),
        QStringLiteral("dataView"),
        QStringLiteral("dataFile")
    };
    QCOMPARE(store.toolbarIconNames(), expectedIcons);
}

QTEST_APPLESS_MAIN(SidebarHierarchyStoreTest)

#include "test_sidebar_hierarchy_store.moc"
