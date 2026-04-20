#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::hierarchyViewModelProvider_normalizesMappingsAndAvoidsDuplicateSignals()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel tagsViewModel(QStringLiteral("tags"));

    libraryViewModel.setNodes(QVariantList{QStringLiteral("library-node")});
    tagsViewModel.setNodes(QVariantList{QStringLiteral("tags-node-a"), QStringLiteral("tags-node-b")});

    QSignalSpy mappingsChangedSpy(&provider, &IHierarchyViewModelProvider::mappingsChanged);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
        {999, &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
    QCOMPARE(
        provider.hierarchyViewModel(WhatSon::Sidebar::kHierarchyDefaultIndex),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        provider.hierarchyViewModel(-42),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        provider.hierarchyViewModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<IHierarchyViewModel*>(&tagsViewModel));
    QCOMPARE(
        provider.noteListModel(-42),
        static_cast<QObject*>(libraryViewModel.hierarchyNoteListModel()));
    QCOMPARE(
        provider.noteListModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<QObject*>(tagsViewModel.hierarchyNoteListModel()));

    const QVector<HierarchyViewModelProvider::Mapping> exportedMappings = provider.mappings();
    QCOMPARE(exportedMappings.size(), 2);
    QCOMPARE(exportedMappings.at(0).hierarchyIndex, WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(exportedMappings.at(0).viewModel, static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        exportedMappings.at(1).hierarchyIndex,
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(exportedMappings.at(1).viewModel, static_cast<IHierarchyViewModel*>(&tagsViewModel));

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
        {999, &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
}
