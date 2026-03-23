#include "policy/ArchitecturePolicyLock.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QObject>
#include <QtTest/QtTest>

class StubHierarchyViewModel final : public IHierarchyViewModel
{
    Q_OBJECT

public:
    explicit StubHierarchyViewModel(QObject* parent = nullptr)
        : IHierarchyViewModel(parent)
    {
        initializeHierarchyInterfaceSignalBridge();
    }

    QObject* itemModel() noexcept override { return nullptr; }
    int selectedIndex() const noexcept override { return -1; }
    void setSelectedIndex(int index) override { Q_UNUSED(index); }
    int itemCount() const noexcept override { return 0; }
    bool loadSucceeded() const noexcept override { return true; }
    QString lastLoadError() const override { return {}; }
    QVariantList hierarchyModel() const override { return {}; }
    QString itemLabel(int index) const override { Q_UNUSED(index); return {}; }
    bool canRenameItem(int index) const override { Q_UNUSED(index); return false; }
    bool renameItem(int index, const QString& displayName) override
    {
        Q_UNUSED(index);
        Q_UNUSED(displayName);
        return false;
    }
    void createFolder() override {}
    void deleteSelectedFolder() override {}
    bool renameEnabled() const noexcept override { return false; }
    bool createFolderEnabled() const noexcept override { return false; }
    bool deleteFolderEnabled() const noexcept override { return false; }

signals:
    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
};

class ArchitecturePolicyLockTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void dependencyMatrix_mustFollowFixedMvvmFilesystemPolicy();
    void lock_mustRejectProviderAndSidebarRewiring();
};

void ArchitecturePolicyLockTest::dependencyMatrix_mustFollowFixedMvvmFilesystemPolicy()
{
    using Layer = WhatSon::Policy::Layer;

    QVERIFY(WhatSon::Policy::isDependencyAllowed(Layer::View, Layer::ViewModel));
    QVERIFY(!WhatSon::Policy::isDependencyAllowed(Layer::View, Layer::Store));
    QVERIFY(!WhatSon::Policy::isDependencyAllowed(Layer::View, Layer::FileSystem));

    QVERIFY(WhatSon::Policy::isDependencyAllowed(Layer::ViewModel, Layer::Store));
    QVERIFY(WhatSon::Policy::isDependencyAllowed(Layer::ViewModel, Layer::Parser));
    QVERIFY(WhatSon::Policy::isDependencyAllowed(Layer::ViewModel, Layer::Creator));
    QVERIFY(!WhatSon::Policy::isDependencyAllowed(Layer::ViewModel, Layer::FileSystem));

    QVERIFY(WhatSon::Policy::isDependencyAllowed(Layer::Store, Layer::FileSystem));
    QVERIFY(!WhatSon::Policy::isDependencyAllowed(Layer::Parser, Layer::FileSystem));
    QVERIFY(!WhatSon::Policy::isDependencyAllowed(Layer::Creator, Layer::FileSystem));
}

void ArchitecturePolicyLockTest::lock_mustRejectProviderAndSidebarRewiring()
{
    StubHierarchyViewModel libraryVmA;
    StubHierarchyViewModel libraryVmB;
    HierarchyViewModelProvider providerA;
    HierarchyViewModelProvider providerB;

    HierarchyViewModelProvider::Targets targetsA;
    targetsA.libraryViewModel = &libraryVmA;
    providerA.setTargets(targetsA);
    QCOMPARE(providerA.hierarchyViewModel(0), &libraryVmA);

    SidebarSelectionStore storeA;
    SidebarSelectionStore storeB;
    SidebarHierarchyViewModel sidebarVm;
    sidebarVm.setSelectionStore(&storeA);
    sidebarVm.setViewModelProvider(&providerA);
    QCOMPARE(sidebarVm.selectionStore(), static_cast<ISidebarSelectionStore*>(&storeA));
    QCOMPARE(sidebarVm.viewModelProvider(), static_cast<IHierarchyViewModelProvider*>(&providerA));

    WhatSon::Policy::ArchitecturePolicyLock::lock();
    QVERIFY(WhatSon::Policy::ArchitecturePolicyLock::isLocked());

    HierarchyViewModelProvider::Targets targetsB;
    targetsB.libraryViewModel = &libraryVmB;
    providerA.setTargets(targetsB);
    QCOMPARE(providerA.hierarchyViewModel(0), &libraryVmA);

    sidebarVm.setSelectionStore(&storeB);
    sidebarVm.setViewModelProvider(&providerB);
    QCOMPARE(sidebarVm.selectionStore(), static_cast<ISidebarSelectionStore*>(&storeA));
    QCOMPARE(sidebarVm.viewModelProvider(), static_cast<IHierarchyViewModelProvider*>(&providerA));
}

QTEST_MAIN(ArchitecturePolicyLockTest)

#include "test_architecture_policy_lock.moc"
