#include "viewmodel/panel/HierarchyDragDropBridge.hpp"

#include <QObject>
#include <QtTest/QtTest>

class FakeHierarchyDragDropViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel WRITE setHierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    QVariantList hierarchyModel() const
    {
        return m_hierarchyModel;
    }

    void setHierarchyModel(const QVariantList& hierarchyModel)
    {
        if (m_hierarchyModel == hierarchyModel)
        {
            return;
        }

        m_hierarchyModel = hierarchyModel;
        emit hierarchyModelChanged();
    }

    int selectedIndex() const noexcept
    {
        return m_selectedIndex;
    }

    void setSelectedIndex(int index)
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

    Q_INVOKABLE bool applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
    {
        lastHierarchyNodes = hierarchyNodes;
        lastActiveItemKey = activeItemKey;
        applyCallCount += 1;
        return applyResult;
    }

    Q_INVOKABLE bool canAcceptNoteDrop(int index, const QString& noteId)
    {
        lastCanAcceptIndex = index;
        lastCanAcceptNoteId = noteId;
        canAcceptCallCount += 1;
        return canAcceptResult;
    }

    Q_INVOKABLE bool assignNoteToFolder(int index, const QString& noteId)
    {
        lastAssignedIndex = index;
        lastAssignedNoteId = noteId;
        assignCallCount += 1;
        return assignResult;
    }

    QVariantList lastHierarchyNodes;
    QString lastActiveItemKey;
    int applyCallCount = 0;
    bool applyResult = true;

    int lastCanAcceptIndex = -1;
    QString lastCanAcceptNoteId;
    int canAcceptCallCount = 0;
    bool canAcceptResult = true;

    int lastAssignedIndex = -1;
    QString lastAssignedNoteId;
    int assignCallCount = 0;
    bool assignResult = true;

    signals  :


    void hierarchyModelChanged();
    void selectedIndexChanged();

private:
    QVariantList m_hierarchyModel;
    int m_selectedIndex = -1;
};

class HierarchyDragDropBridgeTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void reorderContract_mustTrackViewModelSelectionKey();
    void applyHierarchyReorder_mustInvokeViewModelWithResolvedActiveKey();
    void noteDropContract_mustProxyValidationAndAssignment();
};

void HierarchyDragDropBridgeTest::reorderContract_mustTrackViewModelSelectionKey()
{
    FakeHierarchyDragDropViewModel viewModel;
    viewModel.setHierarchyModel(QVariantList{
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("bucket:all")},
            {QStringLiteral("depth"), 0}
        },
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("library:brand")},
            {QStringLiteral("depth"), 0}
        }
    });
    viewModel.setSelectedIndex(1);

    HierarchyDragDropBridge bridge;
    QVERIFY(!bridge.reorderContractAvailable());
    QCOMPARE(bridge.selectedItemKey(), QString());

    bridge.setHierarchyViewModel(&viewModel);
    QVERIFY(bridge.reorderContractAvailable());
    QVERIFY(bridge.noteDropContractAvailable());
    QCOMPARE(bridge.selectedItemKey(), QStringLiteral("library:brand"));

    viewModel.setSelectedIndex(0);
    QCOMPARE(bridge.selectedItemKey(), QStringLiteral("bucket:all"));
}

void HierarchyDragDropBridgeTest::applyHierarchyReorder_mustInvokeViewModelWithResolvedActiveKey()
{
    FakeHierarchyDragDropViewModel viewModel;
    viewModel.setHierarchyModel(QVariantList{
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("bucket:all")},
            {QStringLiteral("depth"), 0}
        },
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("library:brand")},
            {QStringLiteral("depth"), 0}
        }
    });
    viewModel.setSelectedIndex(1);

    HierarchyDragDropBridge bridge;
    bridge.setHierarchyViewModel(&viewModel);

    const QVariantList reorderedNodes{
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("bucket:all")},
            {QStringLiteral("depth"), 0}
        },
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("library:reordered")},
            {QStringLiteral("depth"), 0}
        }
    };

    QVERIFY(bridge.applyHierarchyReorder(reorderedNodes));
    QCOMPARE(viewModel.applyCallCount, 1);
    QCOMPARE(viewModel.lastHierarchyNodes, reorderedNodes);
    QCOMPARE(viewModel.lastActiveItemKey, QStringLiteral("library:brand"));

    QVERIFY(bridge.applyHierarchyReorder(reorderedNodes, QStringLiteral("library:reordered")));
    QCOMPARE(viewModel.applyCallCount, 2);
    QCOMPARE(viewModel.lastActiveItemKey, QStringLiteral("library:reordered"));
}

void HierarchyDragDropBridgeTest::noteDropContract_mustProxyValidationAndAssignment()
{
    FakeHierarchyDragDropViewModel viewModel;
    HierarchyDragDropBridge bridge;
    bridge.setHierarchyViewModel(&viewModel);

    QVERIFY(bridge.canAcceptNoteDrop(3, QStringLiteral("note-a")));
    QCOMPARE(viewModel.canAcceptCallCount, 1);
    QCOMPARE(viewModel.lastCanAcceptIndex, 3);
    QCOMPARE(viewModel.lastCanAcceptNoteId, QStringLiteral("note-a"));

    QVERIFY(bridge.assignNoteToFolder(4, QStringLiteral("note-b")));
    QCOMPARE(viewModel.assignCallCount, 1);
    QCOMPARE(viewModel.lastAssignedIndex, 4);
    QCOMPARE(viewModel.lastAssignedNoteId, QStringLiteral("note-b"));
}

QTEST_MAIN(HierarchyDragDropBridgeTest)

#include "test_hierarchy_drag_drop_bridge.moc"
