#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/panel/HierarchyDragDropBridge.hpp"

#include <QtTest/QtTest>

class FakeHierarchyDragDropViewModel final : public IHierarchyViewModel,
                                             public IHierarchyRenameCapability,
                                             public IHierarchyCrudCapability,
                                             public IHierarchyReorderCapability,
                                             public IHierarchyNoteDropCapability
{
    Q_OBJECT
    Q_INTERFACES(
        IHierarchyRenameCapability
        IHierarchyCrudCapability
        IHierarchyReorderCapability
        IHierarchyNoteDropCapability)

public:
    FakeHierarchyDragDropViewModel()
        : IHierarchyViewModel(nullptr)
    {
        initializeHierarchyInterfaceSignalBridge();
    }

    QObject* itemModel() noexcept override
    {
        return nullptr;
    }

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

    void setSelectedIndex(int index) override
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
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

    bool applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey) override
    {
        lastHierarchyNodes = hierarchyNodes;
        lastActiveItemKey = activeItemKey;
        applyCallCount += 1;
        return applyResult;
    }

    bool canAcceptNoteDrop(int index, const QString& noteId) const override
    {
        lastCanAcceptIndex = index;
        lastCanAcceptNoteId = noteId;
        canAcceptCallCount += 1;
        return canAcceptResult;
    }

    bool assignNoteToFolder(int index, const QString& noteId) override
    {
        lastAssignedIndex = index;
        lastAssignedNoteId = noteId;
        assignCallCount += 1;
        return assignResult;
    }

    bool supportsHierarchyNodeReorder() const noexcept override
    {
        return true;
    }

    bool supportsHierarchyNoteDrop() const noexcept override
    {
        return true;
    }

    QVariantList lastHierarchyNodes;
    QString lastActiveItemKey;
    int applyCallCount = 0;
    bool applyResult = true;

    mutable int lastCanAcceptIndex = -1;
    mutable QString lastCanAcceptNoteId;
    mutable int canAcceptCallCount = 0;
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
