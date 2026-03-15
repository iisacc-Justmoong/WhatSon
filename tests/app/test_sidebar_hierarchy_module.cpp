#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QObject>
#include <QSignalSpy>
#include <QVariant>
#include <QtTest/QtTest>

class StubStandardHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    explicit StubStandardHierarchyViewModel(QObject* parent = nullptr)
        : QObject(parent)
    {
        m_hierarchyModel = QVariantList{
            QVariantMap{
                {QStringLiteral("itemId"), 0},
                {QStringLiteral("key"), QStringLiteral("bucket:all")},
                {QStringLiteral("label"), QStringLiteral("All Library")},
                {QStringLiteral("depth"), 0}
            },
            QVariantMap{
                {QStringLiteral("itemId"), 1},
                {QStringLiteral("key"), QStringLiteral("library:brand")},
                {QStringLiteral("label"), QStringLiteral("Brand")},
                {QStringLiteral("depth"), 0}
            }
        };
    }

    QVariantList hierarchyModel() const
    {
        return m_hierarchyModel;
    }

    int selectedIndex() const noexcept
    {
        return m_selectedIndex;
    }

    int itemCount() const noexcept
    {
        return m_hierarchyModel.size();
    }

    void setHierarchyModel(QVariantList hierarchyModel)
    {
        if (m_hierarchyModel == hierarchyModel)
        {
            return;
        }

        m_hierarchyModel = hierarchyModel;
        emit hierarchyModelChanged();
        emit itemCountChanged();
    }

    Q_INVOKABLE void setSelectedIndex(int index)
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

    signals  :


    void hierarchyModelChanged();
    void selectedIndexChanged();
    void itemCountChanged();

private:
    QVariantList m_hierarchyModel;
    int m_selectedIndex = 0;
};

class SidebarHierarchyModuleTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void activeHierarchyIndex_mustRoundTripThroughSelectionStore();
    void activeHierarchyBindings_mustResolveThroughProviderInterface();
    void hierarchyViewModel_mustExposeStandardHierarchyModelProperty();
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
    StubStandardHierarchyViewModel libraryViewModel;
    QObject projectsViewModel;
    StubStandardHierarchyViewModel bookmarksViewModel;
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

void SidebarHierarchyModuleTest::hierarchyViewModel_mustExposeStandardHierarchyModelProperty()
{
    StubStandardHierarchyViewModel libraryViewModel;
    HierarchyViewModelProvider provider;
    HierarchyViewModelProvider::Targets targets;
    targets.libraryViewModel = &libraryViewModel;
    provider.setTargets(targets);

    SidebarSelectionStore selectionStore;
    SidebarHierarchyViewModel sidebarViewModel;
    sidebarViewModel.setSelectionStore(&selectionStore);
    sidebarViewModel.setViewModelProvider(&provider);

    sidebarViewModel.setActiveHierarchyIndex(0);

    QObject* activeViewModel = sidebarViewModel.activeHierarchyViewModel();
    QVERIFY(activeViewModel != nullptr);

    const QVariantList hierarchyModel = activeViewModel->property("hierarchyModel").toList();
    QCOMPARE(hierarchyModel.size(), 2);

    const QVariantMap firstNode = hierarchyModel.at(0).toMap();
    QCOMPARE(firstNode.value(QStringLiteral("itemId")).toInt(), 0);
    QCOMPARE(firstNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:all"));
    QCOMPARE(firstNode.value(QStringLiteral("label")).toString(), QStringLiteral("All Library"));

    QSignalSpy modelSpy(&libraryViewModel, &StubStandardHierarchyViewModel::hierarchyModelChanged);
    libraryViewModel.setHierarchyModel(QVariantList{
        QVariantMap{
            {QStringLiteral("itemId"), 0},
            {QStringLiteral("key"), QStringLiteral("library:reordered")},
            {QStringLiteral("label"), QStringLiteral("Reordered")},
            {QStringLiteral("depth"), 0}
        }
    });
    QCOMPARE(modelSpy.count(), 1);

    const QVariantList updatedModel = activeViewModel->property("hierarchyModel").toList();
    QCOMPARE(updatedModel.size(), 1);
    QCOMPARE(updatedModel.at(0).toMap().value(QStringLiteral("key")).toString(), QStringLiteral("library:reordered"));
}

QTEST_MAIN(SidebarHierarchyModuleTest)

#include "test_sidebar_hierarchy_module.moc"
