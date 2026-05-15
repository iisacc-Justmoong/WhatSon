#include "test/cpp/whatson_cpp_regression_tests.hpp"

namespace
{
    struct ParentExpansionProbeItem
    {
        bool expanded = false;
        bool showChevron = false;
    };

    class ParentExpansionProbeController final : public FakeHierarchyController,
                                                 public IHierarchyExpansionCapability
    {
    public:
        ParentExpansionProbeController()
            : FakeHierarchyController(QStringLiteral("probe"))
        {
        }

        bool setItemExpanded(int index, bool expanded) override
        {
            return setHierarchyItemExpanded(
                &items,
                index,
                expanded,
                [this](int changedIndex, bool changedExpanded)
                {
                    lastSingleIndex = changedIndex;
                    lastSingleExpanded = changedExpanded;
                    ++singleCommitCount;
                    emit hierarchyModelChanged();
                });
        }

        bool setAllItemsExpanded(bool expanded)
        {
            return setAllHierarchyItemsExpanded(
                &items,
                expanded,
                [this]()
                {
                    ++bulkCommitCount;
                    emit hierarchyModelChanged();
                });
        }

        QVector<ParentExpansionProbeItem> items;
        int singleCommitCount = 0;
        int bulkCommitCount = 0;
        int lastSingleIndex = -1;
        bool lastSingleExpanded = false;
    };

    class ArchitecturePolicyUnlockScope final
    {
    public:
        ArchitecturePolicyUnlockScope()
        {
            WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
        }

        ~ArchitecturePolicyUnlockScope()
        {
            WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
        }
    };

    QVariantMap expandableHierarchyNode(const QString& key, int itemId, bool expanded, bool showChevron)
    {
        return QVariantMap{
            {QStringLiteral("key"), key},
            {QStringLiteral("itemId"), itemId},
            {QStringLiteral("label"), key},
            {QStringLiteral("expanded"), expanded},
            {QStringLiteral("showChevron"), showChevron},
        };
    }
}

void WhatSonCppRegressionTests::hierarchyItemModel_usesSharedLvrsModelContract()
{
    WhatSonHierarchyModel model;
    const QVariantList nodes{
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("type:image")},
            {QStringLiteral("itemId"), 0},
            {QStringLiteral("label"), QStringLiteral("Images")},
            {QStringLiteral("depth"), 0},
            {QStringLiteral("expanded"), false},
            {QStringLiteral("showChevron"), true},
            {QStringLiteral("iconName"), QStringLiteral("imageToImage")},
            {QStringLiteral("count"), 3},
        },
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("format:image:.png")},
            {QStringLiteral("itemId"), 1},
            {QStringLiteral("label"), QStringLiteral(".png")},
            {QStringLiteral("depth"), 1},
            {QStringLiteral("expanded"), false},
            {QStringLiteral("showChevron"), false},
            {QStringLiteral("iconName"), QStringLiteral("virtualFolder")},
            {QStringLiteral("count"), 3},
        },
    };

    model.setItems(nodes);
    QCOMPARE(model.rowCount(), 2);

    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelAboutToBeReset);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);
    QSignalSpy itemsChangedSpy(&model, &WhatSonHierarchyModel::itemsChanged);

    QVERIFY(model.setItemExpanded(0, true));
    QCOMPARE(resetSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(itemsChangedSpy.count(), 1);
    QCOMPARE(model.data(model.index(0, 0), WhatSonHierarchyModel::ExpandedRole).toBool(), true);
    QCOMPARE(model.data(model.index(0, 0), WhatSonHierarchyModel::LabelRole).toString(), QStringLiteral("Images"));
    QCOMPARE(model.data(model.index(0, 0), WhatSonHierarchyModel::KeyRole).toString(), QStringLiteral("type:image"));
    QCOMPARE(model.data(model.index(0, 0), WhatSonHierarchyModel::ItemKeyRole).toString(), QStringLiteral("type:image"));
    QCOMPARE(model.data(model.index(0, 0), WhatSonHierarchyModel::CountRole).toInt(), 3);

    QVERIFY(model.flags(model.index(0, 0)).testFlag(Qt::ItemIsEditable));
    QVERIFY(model.setData(model.index(1, 0), 2, WhatSonHierarchyModel::DepthRole));
    QCOMPARE(model.data(model.index(1, 0), WhatSonHierarchyModel::DepthRole).toInt(), 2);
    QVERIFY(model.setData(model.index(1, 0), QStringLiteral("type:image"), WhatSonHierarchyModel::ParentItemKeyRole));
    QCOMPARE(model.data(model.index(1, 0), WhatSonHierarchyModel::ParentItemKeyRole).toString(), QStringLiteral("type:image"));

    QSignalSpy rowsMovedSpy(&model, &QAbstractItemModel::rowsMoved);
    QVERIFY(model.moveRows(QModelIndex(), 0, 1, QModelIndex(), 2));
    QCOMPARE(rowsMovedSpy.count(), 1);
    QCOMPARE(model.data(model.index(1, 0), WhatSonHierarchyModel::KeyRole).toString(), QStringLiteral("type:image"));
    QCOMPARE(model.items().size(), 2);
}

void WhatSonCppRegressionTests::hierarchyControllers_exposeSharedLvrsHierarchyModel()
{
    const QStringList controllerHeaders{
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/tags/TagsHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/resources/ResourcesHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/progress/ProgressHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/event/EventHierarchyController.hpp"),
        QStringLiteral("src/app/models/hierarchy/preset/PresetHierarchyController.hpp"),
    };

    for (const QString& path : controllerHeaders)
    {
        const QString source = readUtf8SourceFile(path);
        QVERIFY2(!source.isEmpty(), qPrintable(path));
        QVERIFY2(
            source.contains(QStringLiteral("#include \"app/models/hierarchy/WhatSonHierarchyModel.hpp\"")),
            qPrintable(QStringLiteral("%1 must include the shared LVRS hierarchy model").arg(path)));
        QVERIFY2(
            source.contains(QStringLiteral("Q_PROPERTY(WhatSonHierarchyModel* itemModel READ itemModel CONSTANT)")),
            qPrintable(QStringLiteral("%1 must expose the shared item model type").arg(path)));
        QVERIFY2(
            source.contains(QStringLiteral("WhatSonHierarchyModel* itemModel() noexcept override;")),
            qPrintable(QStringLiteral("%1 must return the shared item model type").arg(path)));
        QVERIFY2(
            source.contains(QStringLiteral("WhatSonHierarchyModel m_itemModel;")),
            qPrintable(QStringLiteral("%1 must store the shared item model type").arg(path)));
        QVERIFY2(
            !source.contains(QStringLiteral("Q_PROPERTY(LibraryHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(ProjectsHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(BookmarksHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(TagsHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(ResourcesHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(ProgressHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(EventHierarchyModel* itemModel"))
            && !source.contains(QStringLiteral("Q_PROPERTY(PresetHierarchyModel* itemModel")),
            qPrintable(QStringLiteral("%1 must not expose a domain-specific hierarchy model").arg(path)));
    }

    const QStringList controllerSources{
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/tags/TagsHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/resources/ResourcesHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/progress/ProgressHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/event/EventHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/preset/PresetHierarchyController.cpp"),
    };

    for (const QString& path : controllerSources)
    {
        const QString source = readUtf8SourceFile(path);
        QVERIFY2(!source.isEmpty(), qPrintable(path));
        QVERIFY2(
            source.contains(QStringLiteral("m_itemModel.setItems(depthItems());")),
            qPrintable(QStringLiteral("%1 must feed the shared model from the LV.Hierarchy node contract").arg(path)));
        QVERIFY2(
            source.contains(QStringLiteral("m_itemModel.setItemExpanded(changedIndex, changedExpanded);")),
            qPrintable(QStringLiteral("%1 must update single chevron changes without resetting the model").arg(path)));
        QVERIFY2(
            !source.contains(QStringLiteral("m_itemModel.setItems(m_items);")),
            qPrintable(QStringLiteral("%1 must not publish domain vectors directly to the LVRS-facing model").arg(path)));
    }
}

void WhatSonCppRegressionTests::sidebarHierarchyController_preservesFallbackAcrossStoreAttachDetach()
{
    HierarchyControllerProvider provider;
    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));
    FakeSidebarSelectionStore selectionStore;
    SidebarHierarchyController sidebarController;

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
    });

    sidebarController.setControllerProvider(&provider);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.activeNoteListModel(), libraryController.hierarchyNoteListModel());

    QSignalSpy activeBindingsSpy(&sidebarController, &IActiveHierarchyContextSource::activeBindingsChanged);
    sidebarController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(
        sidebarController.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&tagsController));
    QVERIFY(activeBindingsSpy.count() >= 1);

    sidebarController.setSelectionStore(&selectionStore);

    QCOMPARE(
        selectionStore.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        sidebarController.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarController.activeNoteListModel(), tagsController.hierarchyNoteListModel());

    selectionStore.setSelectedHierarchyIndex(999);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.hierarchyControllerForIndex(-1), static_cast<QObject*>(&libraryController));
    QCOMPARE(sidebarController.noteListModelForIndex(-1), libraryController.hierarchyNoteListModel());

    sidebarController.setSelectionStore(nullptr);

    QCOMPARE(sidebarController.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarController.activeHierarchyController(), static_cast<QObject*>(&libraryController));
}

void WhatSonCppRegressionTests::sidebarHierarchyInteractionController_keepsFooterDispatchOutOfCppPolicy()
{
    const QString controllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/sidebar/SidebarHierarchyInteractionController.hpp"));
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/sidebar/SidebarHierarchyInteractionController.cpp"));
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!controllerHeader.isEmpty());
    QVERIFY(!controllerSource.isEmpty());
    QVERIFY(!sidebarSource.isEmpty());

    QVERIFY(!controllerHeader.contains(QStringLiteral("footerActionName")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("requestFooterAction")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("footerCreateRequested")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("footerDeleteRequested")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("footerOptionsRequested")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("selectedHierarchySyncRequested")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("m_queuedFooterAction")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("m_pendingFooterAction")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("bridgeBoolProperty")));

    QVERIFY(!controllerSource.contains(QStringLiteral("SidebarHierarchyInteractionController::footerActionName")));
    QVERIFY(!controllerSource.contains(QStringLiteral("SidebarHierarchyInteractionController::requestFooterAction")));
    QVERIFY(!controllerSource.contains(QStringLiteral("footerCreateRequested")));
    QVERIFY(!controllerSource.contains(QStringLiteral("footerDeleteRequested")));
    QVERIFY(!controllerSource.contains(QStringLiteral("footerOptionsRequested")));
    QVERIFY(!controllerSource.contains(QStringLiteral("selectedHierarchySyncRequested")));
    QVERIFY(!controllerSource.contains(QStringLiteral("m_pendingFooterAction")));
    QVERIFY(!controllerSource.contains(QStringLiteral("bridgeBoolProperty")));

    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyFooterActionName(index, eventName)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function requestHierarchyFooterAction(action)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function scheduleSelectedHierarchySync(focusView)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("Qt.callLater(function () {\n            sidebarHierarchyView.syncSelectedHierarchyItem(Boolean(focusView));")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function onFooterCreateRequested()")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function onFooterDeleteRequested()")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function onFooterOptionsRequested()")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function onSelectedHierarchySyncRequested(focusView)")));
}

void WhatSonCppRegressionTests::hierarchyController_parentExpansionPolicyMutatesOnlyChevronRows()
{
    ParentExpansionProbeController controller;
    controller.items = QVector<ParentExpansionProbeItem>{
        {false, false},
        {false, true},
        {true, true},
    };

    QVERIFY(!controller.setItemExpanded(-1, true));
    QVERIFY(!controller.setItemExpanded(0, true));
    QVERIFY(!controller.items.at(0).expanded);
    QCOMPARE(controller.singleCommitCount, 0);

    QVERIFY(controller.setItemExpanded(1, false));
    QCOMPARE(controller.singleCommitCount, 0);

    QVERIFY(controller.setItemExpanded(1, true));
    QVERIFY(controller.items.at(1).expanded);
    QCOMPARE(controller.singleCommitCount, 1);
    QCOMPARE(controller.lastSingleIndex, 1);
    QVERIFY(controller.lastSingleExpanded);

    QVERIFY(controller.setAllItemsExpanded(false));
    QVERIFY(!controller.items.at(0).expanded);
    QVERIFY(!controller.items.at(1).expanded);
    QVERIFY(!controller.items.at(2).expanded);
    QCOMPARE(controller.bulkCommitCount, 1);

    QVERIFY(controller.setAllItemsExpanded(false));
    QCOMPARE(controller.bulkCommitCount, 1);
}

void WhatSonCppRegressionTests::hierarchyControllers_delegateChevronExpansionToParentPolicy()
{
    const QString parentHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/IHierarchyController.hpp"));
    const QString sidebarHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/sidebar/SidebarHierarchyInteractionController.hpp"));
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(parentHeader.contains(QStringLiteral("setHierarchyItemExpanded")));
    QVERIFY(parentHeader.contains(QStringLiteral("setAllHierarchyItemsExpanded")));
    QVERIFY(sidebarHeader.contains(QStringLiteral("armExpansionForItem")));
    QVERIFY(sidebarHeader.contains(QStringLiteral("requestChevronExpansionForItem")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("function hierarchyItemExpansionKey(item, fallbackIndex)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("armExpansionForItem(target.item, target.index)")));
    QVERIFY(sidebarSource.contains(
        QStringLiteral("requestChevronExpansionForItem(targetItem, resolvedIndex, expectedKey)")));

    const QStringList controllerSourcePaths{
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/tags/TagsHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/resources/ResourcesHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/progress/ProgressHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/event/EventHierarchyController.cpp"),
        QStringLiteral("src/app/models/hierarchy/preset/PresetHierarchyController.cpp"),
    };

    for (const QString& path : controllerSourcePaths)
    {
        const QString source = readUtf8SourceFile(path);
        QVERIFY2(
            source.contains(QStringLiteral("setHierarchyItemExpanded(\n        &m_items,")),
            qPrintable(QStringLiteral("%1 must delegate single-row chevron expansion to IHierarchyController").arg(path)));
    }

    const QString projectsSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp"));
    QVERIFY(projectsSource.contains(QStringLiteral("setAllHierarchyItemsExpanded(\n        &m_items,")));
}

void WhatSonCppRegressionTests::sidebarHierarchyInteractionController_commitsExpansionStateThroughCppPolicy()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));
    QVERIFY(!sidebarSource.isEmpty());

    const qsizetype directHandlerIndex = sidebarSource.indexOf(QStringLiteral("onListItemExpanded:"));
    QVERIFY(directHandlerIndex >= 0);
    const qsizetype directHandlerEndIndex =
        sidebarSource.indexOf(QStringLiteral("onListItemMoved:"), directHandlerIndex);
    QVERIFY(directHandlerEndIndex > directHandlerIndex);
    const qsizetype directCommittedIndex =
        sidebarSource.indexOf(QStringLiteral("if (result && result.committed)"), directHandlerIndex);
    QVERIFY(directCommittedIndex > directHandlerIndex && directCommittedIndex < directHandlerEndIndex);
    const QString directHandlerBlock =
        sidebarSource.mid(directCommittedIndex, directHandlerEndIndex - directCommittedIndex);
    const qsizetype directSyncIndex =
        directHandlerBlock.indexOf(QStringLiteral("sidebarHierarchyView.syncDisplayedHierarchyModel(true);"));
    QVERIFY(directSyncIndex < 0);
    const qsizetype directHookIndex =
        directHandlerBlock.indexOf(
            QStringLiteral("sidebarHierarchyView.requestViewHook(\"hierarchy.chevron.toggle\");"));
    QVERIFY(directHookIndex < 0);

    const qsizetype fallbackFunctionIndex =
        sidebarSource.indexOf(QStringLiteral("function requestHierarchyChevronExpansionForTarget"));
    QVERIFY(fallbackFunctionIndex >= 0);
    const qsizetype fallbackFunctionEndIndex =
        sidebarSource.indexOf(QStringLiteral("function requestHierarchyChevronExpansionAtPosition"), fallbackFunctionIndex);
    QVERIFY(fallbackFunctionEndIndex > fallbackFunctionIndex);
    const qsizetype fallbackCommittedIndex =
        sidebarSource.indexOf(QStringLiteral("if (result && result.committed)"), fallbackFunctionIndex);
    QVERIFY(fallbackCommittedIndex > fallbackFunctionIndex && fallbackCommittedIndex < fallbackFunctionEndIndex);
    const QString fallbackFunctionBlock =
        sidebarSource.mid(fallbackCommittedIndex, fallbackFunctionEndIndex - fallbackCommittedIndex);
    const qsizetype fallbackSyncIndex =
        fallbackFunctionBlock.indexOf(QStringLiteral("sidebarHierarchyView.syncDisplayedHierarchyModel(true);"));
    QVERIFY(fallbackSyncIndex < 0);
    const qsizetype fallbackHookIndex =
        fallbackFunctionBlock.indexOf(
            QStringLiteral("sidebarHierarchyView.requestViewHook(\"hierarchy.chevron.toggle\");"));
    QVERIFY(fallbackHookIndex < 0);

    SidebarHierarchyInteractionController controller;
    FakeSidebarHierarchyInteractionBridge bridge;
    controller.setHierarchyInteractionBridge(&bridge);
    controller.setActiveHierarchyIndex(3);

    QVariantMap node{
        {QStringLiteral("itemKey"), QStringLiteral("alpha")},
        {QStringLiteral("expanded"), false},
        {QStringLiteral("showChevron"), true},
    };
    const QVariantList nodes{node};
    const QString key = controller.itemExpansionKey(node, 0);

    QCOMPARE(key, QStringLiteral("hierarchy:3:alpha"));
    controller.captureExpansionState(nodes);
    QVERIFY(controller.expansionStateContainsKey(key));
    QVERIFY(!controller.expansionStateForKey(key, true));

    const int pendingActivation = controller.beginActivationAttempt();
    node.insert(QStringLiteral("expanded"), true);
    const QVariantMap expandedResult = controller.handleExpansionSignal(node, 0, true);

    QVERIFY(expandedResult.value(QStringLiteral("committed")).toBool());
    QCOMPARE(bridge.setItemExpandedCallCount, 1);
    QCOMPARE(bridge.lastExpandedIndex, 0);
    QVERIFY(bridge.lastExpandedValue);
    QVERIFY(controller.expansionStateForKey(key, false));
    QVERIFY(!controller.activationAttemptCurrent(pendingActivation));
    QVERIFY(controller.shouldSuppressActivation());
    QTRY_VERIFY_WITH_TIMEOUT(!controller.shouldSuppressActivation(), 1000);

    QVariantMap staleNode = node;
    staleNode.insert(QStringLiteral("expanded"), false);
    const QVariantList preservedModel = controller.modelWithPreservedExpansion(QVariantList{staleNode});
    QCOMPARE(preservedModel.size(), 1);
    QVERIFY(preservedModel.first().toMap().value(QStringLiteral("expanded")).toBool());

    QVERIFY(controller.armExpansionKey(key));
    const QVariantMap collapsedResult = controller.requestChevronExpansion(0, key, true, key);
    QVERIFY(collapsedResult.value(QStringLiteral("committed")).toBool());
    QCOMPARE(bridge.setItemExpandedCallCount, 2);
    QVERIFY(!bridge.lastExpandedValue);
    QVERIFY(!controller.expansionStateForKey(key, true));

    bridge.setItemExpandedResult = false;
    QVERIFY(controller.armExpansionKey(key));
    const QVariantMap failedResult = controller.requestChevronExpansion(0, key, false, key);
    QVERIFY(failedResult.value(QStringLiteral("rollbackRequired")).toBool());
    QVERIFY(!controller.expansionStateForKey(key, true));
}

void WhatSonCppRegressionTests::hierarchyInteractionBridge_bindsRuntimeControllerAfterArchitectureLock()
{
    ArchitecturePolicyUnlockScope unlockScope;

    FakeExpandableHierarchyController hierarchyController(QStringLiteral("library"));
    hierarchyController.setNodes(QVariantList{
        expandableHierarchyNode(QStringLiteral("folder:root"), 0, false, true),
        expandableHierarchyNode(QStringLiteral("folder:child"), 1, false, false),
    });

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    HierarchyInteractionBridge bridge;
    bridge.setHierarchyController(&hierarchyController);

    QCOMPARE(bridge.hierarchyController(), static_cast<QObject*>(&hierarchyController));
    QVERIFY(bridge.setItemExpanded(0, true));
    QCOMPARE(hierarchyController.setItemExpandedCallCount, 1);
    QCOMPARE(hierarchyController.lastExpandedIndex, 0);
    QVERIFY(hierarchyController.lastExpandedValue);
    QVERIFY(hierarchyController.expandedAt(0));
}

void WhatSonCppRegressionTests::hierarchyInteractionBridge_rebindsActiveRuntimeControllerAfterArchitectureLock()
{
    ArchitecturePolicyUnlockScope unlockScope;

    FakeExpandableHierarchyController libraryController(QStringLiteral("library"));
    libraryController.setNodes(QVariantList{
        expandableHierarchyNode(QStringLiteral("folder:library"), 0, false, true),
        expandableHierarchyNode(QStringLiteral("folder:library-child"), 1, false, false),
    });
    FakeExpandableHierarchyController tagsController(QStringLiteral("tags"));
    tagsController.setNodes(QVariantList{
        expandableHierarchyNode(QStringLiteral("tag:root"), 0, false, true),
        expandableHierarchyNode(QStringLiteral("tag:child"), 1, false, false),
    });

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    HierarchyInteractionBridge bridge;
    SidebarHierarchyInteractionController interactionController;

    bridge.setHierarchyController(&libraryController);
    interactionController.setHierarchyInteractionBridge(&bridge);
    QCOMPARE(interactionController.hierarchyInteractionBridge(), static_cast<QObject*>(&bridge));

    QVariantMap libraryNode = libraryController.hierarchyModel().at(0).toMap();
    QVariantMap libraryResult = interactionController.handleExpansionSignal(libraryNode, 0, true);
    QVERIFY(libraryResult.value(QStringLiteral("committed")).toBool());
    QVERIFY(libraryController.expandedAt(0));

    bridge.setHierarchyController(&tagsController);
    QCOMPARE(bridge.hierarchyController(), static_cast<QObject*>(&tagsController));

    QVariantMap tagsNode = tagsController.hierarchyModel().at(0).toMap();
    QVariantMap tagsResult = interactionController.handleExpansionSignal(tagsNode, 0, true);
    QVERIFY(tagsResult.value(QStringLiteral("committed")).toBool());
    QVERIFY(tagsController.expandedAt(0));
    QVERIFY(!libraryController.expandedAt(1));
}

void WhatSonCppRegressionTests::sidebarHierarchyController_reactsToProviderMappingChanges()
{
    HierarchyControllerProvider provider;
    FakeHierarchyController firstLibraryController(QStringLiteral("library-a"));
    FakeHierarchyController replacementLibraryController(QStringLiteral("library-b"));
    SidebarHierarchyController sidebarController;

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &firstLibraryController},
    });
    sidebarController.setControllerProvider(&provider);

    QSignalSpy hierarchyControllerChangedSpy(
        &sidebarController,
        &IActiveHierarchyContextSource::activeHierarchyControllerChanged);
    QSignalSpy noteListModelChangedSpy(
        &sidebarController,
        &IActiveHierarchyContextSource::activeNoteListModelChanged);
    QSignalSpy activeBindingsSpy(&sidebarController, &IActiveHierarchyContextSource::activeBindingsChanged);

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &replacementLibraryController},
    });

    QCOMPARE(
        sidebarController.activeHierarchyController(),
        static_cast<QObject*>(&replacementLibraryController));
    QCOMPARE(
        sidebarController.activeNoteListModel(),
        replacementLibraryController.hierarchyNoteListModel());
    QVERIFY(hierarchyControllerChangedSpy.count() >= 1);
    QVERIFY(noteListModelChangedSpy.count() >= 1);
    QVERIFY(activeBindingsSpy.count() >= 1);
}
