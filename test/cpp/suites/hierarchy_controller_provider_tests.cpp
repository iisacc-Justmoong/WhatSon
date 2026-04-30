#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::hierarchyControllerProvider_normalizesMappingsAndAvoidsDuplicateSignals()
{
    HierarchyControllerProvider provider;
    FakeHierarchyController libraryController(QStringLiteral("library"));
    FakeHierarchyController tagsController(QStringLiteral("tags"));

    libraryController.setNodes(QVariantList{QStringLiteral("library-node")});
    tagsController.setNodes(QVariantList{QStringLiteral("tags-node-a"), QStringLiteral("tags-node-b")});

    QSignalSpy mappingsChangedSpy(&provider, &IHierarchyControllerProvider::mappingsChanged);

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
        {999, &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
    QCOMPARE(
        provider.hierarchyController(WhatSon::Sidebar::kHierarchyDefaultIndex),
        static_cast<IHierarchyController*>(&libraryController));
    QCOMPARE(
        provider.hierarchyController(-42),
        static_cast<IHierarchyController*>(&libraryController));
    QCOMPARE(
        provider.hierarchyController(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<IHierarchyController*>(&tagsController));
    QCOMPARE(
        provider.noteListModel(-42),
        static_cast<QObject*>(libraryController.hierarchyNoteListModel()));
    QCOMPARE(
        provider.noteListModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<QObject*>(tagsController.hierarchyNoteListModel()));

    const QVector<HierarchyControllerProvider::Mapping> exportedMappings = provider.mappings();
    QCOMPARE(exportedMappings.size(), 2);
    QCOMPARE(exportedMappings.at(0).hierarchyIndex, WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(exportedMappings.at(0).controller, static_cast<IHierarchyController*>(&libraryController));
    QCOMPARE(
        exportedMappings.at(1).hierarchyIndex,
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(exportedMappings.at(1).controller, static_cast<IHierarchyController*>(&tagsController));

    provider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsController},
        {999, &libraryController},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryController},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
}
