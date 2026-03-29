#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/panel/HierarchyInteractionBridge.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QObject>
#include <QSignalSpy>
#include <QVariant>
#include <QtTest/QtTest>

class StubStandardHierarchyViewModel final : public IHierarchyViewModel,
                                             public IHierarchyRenameCapability,
                                             public IHierarchyCrudCapability,
                                             public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

public:
    explicit StubStandardHierarchyViewModel(QObject* parent = nullptr)
        : IHierarchyViewModel(parent)
    {
        initializeHierarchyInterfaceSignalBridge();
        m_hierarchyModel = QVariantList{
            QVariantMap{
                {QStringLiteral("itemId"), 0},
                {QStringLiteral("key"), QStringLiteral("bucket:all")},
                {QStringLiteral("label"), QStringLiteral("All Library")},
                {QStringLiteral("depth"), 0},
                {QStringLiteral("expanded"), false},
                {QStringLiteral("showChevron"), true}
            },
            QVariantMap{
                {QStringLiteral("itemId"), 1},
                {QStringLiteral("key"), QStringLiteral("library:brand")},
                {QStringLiteral("label"), QStringLiteral("Brand")},
                {QStringLiteral("depth"), 0},
                {QStringLiteral("expanded"), false},
                {QStringLiteral("showChevron"), false}
            }
        };
    }

    QObject* itemModel() noexcept override
    {
        return nullptr;
    }

    QObject* noteListModel() noexcept override
    {
        return m_noteListModel;
    }

    QVariantList hierarchyModel() const override
    {
        return m_hierarchyModel;
    }

    int selectedIndex() const noexcept override
    {
        return m_selectedIndex;
    }

    int itemCount() const noexcept override
    {
        return m_hierarchyModel.size();
    }

    bool loadSucceeded() const noexcept override
    {
        return true;
    }

    QString lastLoadError() const override
    {
        return {};
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

    void setSelectedIndex(int index) override
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

    QString itemLabel(int index) const override
    {
        if (index < 0 || index >= m_hierarchyModel.size())
        {
            return {};
        }
        return m_hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString();
    }

    bool canRenameItem(int index) const override
    {
        Q_UNUSED(index);
        return false;
    }

    bool renameItem(int index, const QString& displayName) override
    {
        Q_UNUSED(index);
        Q_UNUSED(displayName);
        return false;
    }

    void createFolder() override
    {
    }

    void deleteSelectedFolder() override
    {
    }

    bool renameEnabled() const noexcept override
    {
        return false;
    }

    bool createFolderEnabled() const noexcept override
    {
        return false;
    }

    bool deleteFolderEnabled() const noexcept override
    {
        return false;
    }

    bool setItemExpanded(int index, bool expanded) override
    {
        if (index < 0 || index >= m_hierarchyModel.size())
        {
            return false;
        }

        QVariantMap entry = m_hierarchyModel.at(index).toMap();
        if (!entry.value(QStringLiteral("showChevron")).toBool())
        {
            return false;
        }

        entry.insert(QStringLiteral("expanded"), expanded);
        m_hierarchyModel[index] = entry;
        m_expansionCalls.push_back(qMakePair(index, expanded));
        emit hierarchyModelChanged();
        return true;
    }

    Q_INVOKABLE bool setAllItemsExpanded(bool expanded)
    {
        bool handled = false;
        for (int index = 0; index < m_hierarchyModel.size(); ++index)
        {
            const QVariantMap entry = m_hierarchyModel.at(index).toMap();
            if (!entry.value(QStringLiteral("showChevron")).toBool())
            {
                continue;
            }

            handled = true;
            if (entry.value(QStringLiteral("expanded")).toBool() == expanded)
            {
                continue;
            }
            setItemExpanded(index, expanded);
        }

        return handled;
    }

    QVector<QPair<int, bool>> expansionCalls() const
    {
        return m_expansionCalls;
    }

    void clearExpansionCalls()
    {
        m_expansionCalls.clear();
    }

    void setNoteListModel(QObject* noteListModel)
    {
        m_noteListModel = noteListModel;
    }

    signals  :


    void hierarchyModelChanged();
    void selectedIndexChanged();
    void itemCountChanged();
    void loadStateChanged();

private:
    QVariantList m_hierarchyModel;
    int m_selectedIndex = 0;
    QObject* m_noteListModel = nullptr;
    QVector<QPair<int, bool>> m_expansionCalls;
};

class SidebarHierarchyModuleTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void activeHierarchyIndex_mustRoundTripThroughSelectionStore();
    void activeHierarchyBindings_mustResolveThroughProviderInterface();
    void hierarchyViewModel_mustExposeStandardHierarchyModelProperty();
    void hierarchyInteractionBridge_bulkExpansion_mustUpdateOnlyExpandableRows();
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
    StubStandardHierarchyViewModel projectsViewModel;
    StubStandardHierarchyViewModel bookmarksViewModel;
    QObject libraryNoteListModel;
    QObject projectsNoteListModel;
    QObject bookmarksNoteListModel;

    libraryViewModel.setNoteListModel(&libraryNoteListModel);
    projectsViewModel.setNoteListModel(&projectsNoteListModel);
    bookmarksViewModel.setNoteListModel(&bookmarksNoteListModel);

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
    QCOMPARE(sidebarViewModel.activeNoteListModel(), &projectsNoteListModel);
    QCOMPARE(sidebarViewModel.property("resolvedActiveHierarchyIndex").toInt(), 1);
    QCOMPARE(sidebarViewModel.property("resolvedHierarchyViewModel").value<QObject*>(), &projectsViewModel);
    QCOMPARE(sidebarViewModel.property("resolvedNoteListModel").value<QObject*>(), &projectsNoteListModel);

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

    IHierarchyViewModel* activeViewModel = qobject_cast<IHierarchyViewModel*>(sidebarViewModel.activeHierarchyViewModel());
    QVERIFY(activeViewModel != nullptr);

    const QVariantList hierarchyModel = activeViewModel->hierarchyNodes();
    QCOMPARE(hierarchyModel.size(), 2);

    const QVariantMap firstNode = hierarchyModel.at(0).toMap();
    QCOMPARE(firstNode.value(QStringLiteral("itemId")).toInt(), 0);
    QCOMPARE(firstNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:all"));
    QCOMPARE(firstNode.value(QStringLiteral("label")).toString(), QStringLiteral("All Library"));

    QSignalSpy modelSpy(activeViewModel, SIGNAL(hierarchyNodesChanged()));
    libraryViewModel.setHierarchyModel(QVariantList{
        QVariantMap{
            {QStringLiteral("itemId"), 0},
            {QStringLiteral("key"), QStringLiteral("library:reordered")},
            {QStringLiteral("label"), QStringLiteral("Reordered")},
            {QStringLiteral("depth"), 0}
        }
    });
    QCOMPARE(modelSpy.count(), 1);

    const QVariantList updatedModel = activeViewModel->hierarchyNodes();
    QCOMPARE(updatedModel.size(), 1);
    QCOMPARE(updatedModel.at(0).toMap().value(QStringLiteral("key")).toString(), QStringLiteral("library:reordered"));
}

void SidebarHierarchyModuleTest::hierarchyInteractionBridge_bulkExpansion_mustUpdateOnlyExpandableRows()
{
    StubStandardHierarchyViewModel viewModel;
    viewModel.setHierarchyModel(QVariantList{
        QVariantMap{
            {QStringLiteral("itemId"), 0},
            {QStringLiteral("key"), QStringLiteral("parent")},
            {QStringLiteral("label"), QStringLiteral("Parent")},
            {QStringLiteral("depth"), 0},
            {QStringLiteral("expanded"), false},
            {QStringLiteral("showChevron"), true}
        },
        QVariantMap{
            {QStringLiteral("itemId"), 1},
            {QStringLiteral("key"), QStringLiteral("leaf")},
            {QStringLiteral("label"), QStringLiteral("Leaf")},
            {QStringLiteral("depth"), 1},
            {QStringLiteral("expanded"), false},
            {QStringLiteral("showChevron"), false}
        },
        QVariantMap{
            {QStringLiteral("itemId"), 2},
            {QStringLiteral("key"), QStringLiteral("second-parent")},
            {QStringLiteral("label"), QStringLiteral("Second Parent")},
            {QStringLiteral("depth"), 0},
            {QStringLiteral("expanded"), true},
            {QStringLiteral("showChevron"), true}
        }
    });

    HierarchyInteractionBridge bridge;
    bridge.setHierarchyViewModel(&viewModel);

    QVERIFY(bridge.setAllItemsExpanded(true));
    QCOMPARE(viewModel.expansionCalls().size(), 1);
    QCOMPARE(viewModel.expansionCalls().at(0).first, 0);
    QCOMPARE(viewModel.expansionCalls().at(0).second, true);
    QCOMPARE(viewModel.hierarchyNodes().at(0).toMap().value(QStringLiteral("expanded")).toBool(), true);
    QCOMPARE(viewModel.hierarchyNodes().at(1).toMap().value(QStringLiteral("expanded")).toBool(), false);
    QCOMPARE(viewModel.hierarchyNodes().at(2).toMap().value(QStringLiteral("expanded")).toBool(), true);

    viewModel.clearExpansionCalls();

    QVERIFY(bridge.setAllItemsExpanded(false));
    QCOMPARE(viewModel.expansionCalls().size(), 2);
    QCOMPARE(viewModel.expansionCalls().at(0).first, 0);
    QCOMPARE(viewModel.expansionCalls().at(0).second, false);
    QCOMPARE(viewModel.expansionCalls().at(1).first, 2);
    QCOMPARE(viewModel.expansionCalls().at(1).second, false);
    QCOMPARE(viewModel.hierarchyNodes().at(0).toMap().value(QStringLiteral("expanded")).toBool(), false);
    QCOMPARE(viewModel.hierarchyNodes().at(2).toMap().value(QStringLiteral("expanded")).toBool(), false);
}

QTEST_MAIN(SidebarHierarchyModuleTest)

#include "test_sidebar_hierarchy_module.moc"
