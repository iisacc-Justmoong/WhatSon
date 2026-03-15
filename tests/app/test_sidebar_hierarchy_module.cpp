#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyLvrsAdapter.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QObject>
#include <QVariant>
#include <QtTest/QtTest>

class StubLvrsHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    explicit StubLvrsHierarchyViewModel(QObject* parent = nullptr)
        : QObject(parent)
    {
        m_depthItems = QVariantList{
            QVariantMap{
                {QStringLiteral("label"), QStringLiteral("All Library")},
                {QStringLiteral("key"), QStringLiteral("bucket:all")},
                {QStringLiteral("depth"), 0}
            },
            QVariantMap{
                {QStringLiteral("label"), QStringLiteral("Brand")},
                {QStringLiteral("key"), QStringLiteral("Brand")},
                {QStringLiteral("depth"), 0}
            }
        };
    }

    int selectedIndex() const noexcept
    {
        return m_selectedIndex;
    }

    int itemCount() const noexcept
    {
        return m_depthItems.size();
    }

    QVariantList lastCommittedNodes() const
    {
        return m_lastCommittedNodes;
    }

    QString lastCommittedActiveKey() const
    {
        return m_lastCommittedActiveKey;
    }

    Q_INVOKABLE QVariantList depthItems() const
    {
        return m_depthItems;
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

    Q_INVOKABLE bool canMoveFolder(int index) const
    {
        return index > 0;
    }

    Q_INVOKABLE bool canRenameItem(int index) const
    {
        return index > 0;
    }

    Q_INVOKABLE bool renameItem(int index, const QString& displayName)
    {
        if (!canRenameItem(index))
        {
            return false;
        }

        QVariantMap entry = m_depthItems.at(index).toMap();
        entry.insert(QStringLiteral("label"), displayName.trimmed());
        m_depthItems[index] = entry;
        emit itemCountChanged();
        return true;
    }

    Q_INVOKABLE bool applyHierarchyNodes(const QVariantList& nodes, const QString& activeItemKey)
    {
        m_lastCommittedNodes = nodes;
        m_lastCommittedActiveKey = activeItemKey.trimmed();
        return true;
    }

    Q_INVOKABLE bool canAcceptNoteDrop(int index, const QString& noteId) const
    {
        return index > 0 && !noteId.trimmed().isEmpty();
    }

    Q_INVOKABLE bool assignNoteToFolder(int index, const QString& noteId)
    {
        Q_UNUSED(index)
        m_lastDroppedNoteId = noteId.trimmed();
        return !m_lastDroppedNoteId.isEmpty();
    }

    QString lastDroppedNoteId() const
    {
        return m_lastDroppedNoteId;
    }

    signals  :


    void selectedIndexChanged();
    void itemCountChanged();
    void loadStateChanged();
    void noteItemCountChanged();

private:
    QVariantList m_depthItems;
    QVariantList m_lastCommittedNodes;
    QString m_lastCommittedActiveKey;
    QString m_lastDroppedNoteId;
    int m_selectedIndex = 0;
};

class SidebarHierarchyModuleTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void activeHierarchyIndex_mustRoundTripThroughSelectionStore();
    void activeHierarchyBindings_mustResolveThroughProviderInterface();
    void lvrsAdapter_mustBridgeDepthItemsAndDisableEditableSearchMoves();
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

void SidebarHierarchyModuleTest::lvrsAdapter_mustBridgeDepthItemsAndDisableEditableSearchMoves()
{
    StubLvrsHierarchyViewModel hierarchyViewModel;
    SidebarHierarchyLvrsAdapter adapter;
    adapter.setHierarchyViewModel(&hierarchyViewModel);

    QCOMPARE(adapter.nodes().size(), 2);
    QCOMPARE(adapter.flatNodes().size(), 2);
    QCOMPARE(adapter.selectedItemKey(), QStringLiteral("bucket:all"));
    QVERIFY(adapter.editable());
    QVERIFY(adapter.noteDropEnabled());

    const QVariantMap firstNode = adapter.flatNodes().at(0).toMap();
    const QVariantMap secondNode = adapter.flatNodes().at(1).toMap();
    QCOMPARE(firstNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:all"));
    QCOMPARE(firstNode.value(QStringLiteral("dragLocked")).toBool(), true);
    QCOMPARE(secondNode.value(QStringLiteral("key")).toString(), QStringLiteral("Brand"));
    QCOMPARE(secondNode.value(QStringLiteral("dragLocked")).toBool(), false);

    adapter.activateKey(QStringLiteral("Brand"));
    QCOMPARE(hierarchyViewModel.selectedIndex(), 1);
    QCOMPARE(adapter.selectedItemKey(), QStringLiteral("Brand"));

    QVERIFY(adapter.renameKey(QStringLiteral("Brand"), QStringLiteral("Branding")));
    QCOMPARE(adapter.labelForKey(QStringLiteral("Brand")), QStringLiteral("Branding"));

    QVERIFY(adapter.canAcceptNoteDrop(QStringLiteral("Brand"), QStringLiteral("note-a")));
    QVERIFY(adapter.assignNoteToKey(QStringLiteral("Brand"), QStringLiteral("note-a")));
    QCOMPARE(hierarchyViewModel.lastDroppedNoteId(), QStringLiteral("note-a"));

    const QVariantList currentNodes = adapter.nodes();
    QVERIFY(adapter.commitEditableNodes(currentNodes, QStringLiteral("Brand")));
    QCOMPARE(hierarchyViewModel.lastCommittedNodes(), currentNodes);
    QCOMPARE(hierarchyViewModel.lastCommittedActiveKey(), QStringLiteral("Brand"));

    adapter.setSearchQuery(QStringLiteral("brand"));
    QCOMPARE(adapter.nodes().size(), 1);
    QVERIFY(!adapter.editable());
    QVERIFY(!adapter.commitEditableNodes(adapter.nodes(), QStringLiteral("Brand")));
}

QTEST_MAIN(SidebarHierarchyModuleTest)

#include "test_sidebar_hierarchy_module.moc"
