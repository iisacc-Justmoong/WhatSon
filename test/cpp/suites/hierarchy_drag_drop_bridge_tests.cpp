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
