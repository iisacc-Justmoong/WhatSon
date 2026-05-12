#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/panel/HierarchyDragDropBridge.hpp"

namespace
{
    QVariantMap hierarchyNode(const QString& key, int itemId)
    {
        return QVariantMap{
            {QStringLiteral("key"), key},
            {QStringLiteral("itemId"), itemId},
            {QStringLiteral("label"), key},
        };
    }
}

void WhatSonCppRegressionTests::hierarchyDragDropBridge_assignsDraggedNoteListItemsToFolderCapability()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeNoteDropHierarchyController controller(QStringLiteral("drop"));
    controller.setNodes(QVariantList{
        hierarchyNode(QStringLiteral("system:all"), 0),
        hierarchyNode(QStringLiteral("folder:interview"), 1),
    });
    controller.setSelectedIndex(1);
    QSet<int> acceptedDropIndices;
    acceptedDropIndices.insert(1);
    controller.setAcceptedDropIndices(acceptedDropIndices);
    controller.rejectNoteId(QStringLiteral("blocked-note"));

    HierarchyDragDropBridge bridge;
    bridge.setHierarchyController(&controller);

    QVERIFY(bridge.noteDropContractAvailable());
    QCOMPARE(bridge.selectedItemKey(), QStringLiteral("folder:interview"));
    QVERIFY(!bridge.canAcceptNoteDrop(0, QStringLiteral("note-a")));
    QVERIFY(bridge.canAcceptNoteDrop(1, QStringLiteral(" note-a ")));

    QVariantList draggedNoteIds;
    draggedNoteIds << QStringLiteral(" note-a ")
                   << QStringLiteral("note-b")
                   << QStringLiteral("note-a")
                   << QString()
                   << QStringLiteral("blocked-note");

    QVERIFY(bridge.canAcceptNoteDropList(1, draggedNoteIds));
    QVERIFY(bridge.assignNotesToFolder(1, draggedNoteIds));
    QCOMPARE(controller.assignedNoteIds(), QStringList({QStringLiteral("note-a"), QStringLiteral("note-b")}));
    QVERIFY(!bridge.canAcceptNoteDrop(1, QStringLiteral("note-a")));
    QVERIFY(!bridge.assignNotesToFolder(0, draggedNoteIds));

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}

void WhatSonCppRegressionTests::hierarchyDragDropBridge_appliesReorderFromQmlArrayModel()
{
    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();

    FakeReorderHierarchyController controller(QStringLiteral("reorder"));
    controller.setNodes(QVariantList{
        hierarchyNode(QStringLiteral("folder:a"), 0),
        hierarchyNode(QStringLiteral("folder:b"), 1),
        hierarchyNode(QStringLiteral("folder:c"), 2),
    });
    controller.setSelectedIndex(0);

    HierarchyDragDropBridge bridge;
    bridge.setHierarchyController(&controller);

    QVERIFY(bridge.reorderContractAvailable());
    QCOMPARE(bridge.selectedItemKey(), QStringLiteral("folder:a"));

    QJSEngine engine;
    const QJSValue reorderedModel = engine.evaluate(QStringLiteral(R"JS([
        {"key": "folder:b", "itemId": 1, "label": "B", "depth": 0},
        {"key": "folder:a", "itemId": 0, "label": "A", "depth": 1},
        {"key": "folder:c", "itemId": 2, "label": "C", "depth": 0}
    ])JS"));
    QVERIFY(!reorderedModel.isError());

    QVERIFY(bridge.applyHierarchyReorder(QVariant::fromValue(reorderedModel), QStringLiteral("folder:b")));

    const QVariantList appliedNodes = controller.appliedNodes();
    QCOMPARE(appliedNodes.size(), 3);
    QCOMPARE(appliedNodes.at(0).toMap().value(QStringLiteral("key")).toString(), QStringLiteral("folder:b"));
    QCOMPARE(appliedNodes.at(0).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(appliedNodes.at(1).toMap().value(QStringLiteral("key")).toString(), QStringLiteral("folder:a"));
    QCOMPARE(appliedNodes.at(1).toMap().value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(controller.appliedActiveItemKey(), QStringLiteral("folder:b"));
    QCOMPARE(bridge.selectedItemKey(), QStringLiteral("folder:b"));

    QVERIFY(bridge.applyHierarchyMove(0, 1, 1, QStringLiteral("folder:a")));
    QCOMPARE(controller.appliedMoveSourceIndex(), 0);
    QCOMPARE(controller.appliedMoveTargetIndex(), 1);
    QCOMPARE(controller.appliedMoveTargetDepth(), 1);
    QCOMPARE(controller.appliedActiveItemKey(), QStringLiteral("folder:a"));
    QVERIFY(!bridge.applyHierarchyMove(-1, 1, 0, QStringLiteral("folder:a")));

    const QJSValue invalidModel = engine.evaluate(QStringLiteral("42"));
    QVERIFY(!invalidModel.isError());
    QVERIFY(!bridge.applyHierarchyReorder(QVariant::fromValue(invalidModel), QStringLiteral("folder:c")));

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
}
