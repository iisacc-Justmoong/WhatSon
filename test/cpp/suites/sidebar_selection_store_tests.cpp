#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals()
{
    SidebarSelectionStore store;
    QSignalSpy selectionChangedSpy(&store, &ISidebarSelectionStore::selectedHierarchyIndexChanged);

    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);

    store.setSelectedHierarchyIndex(999);
    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(selectionChangedSpy.count(), 0);

    store.setSelectedHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        store.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(selectionChangedSpy.count(), 1);

    store.setSelectedHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(selectionChangedSpy.count(), 1);

    store.setSelectedHierarchyIndex(-7);
    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(selectionChangedSpy.count(), 2);
}
