#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/hierarchy/projects/WhatSonProjectsHierarchyCreator.hpp"
#include "app/models/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "app/models/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"

namespace
{
    void compareProjectEntry(
        const WhatSonFolderDepthEntry& entry,
        const QString& expectedId,
        const QString& expectedLabel,
        int expectedDepth)
    {
        QCOMPARE(entry.id, expectedId);
        QCOMPARE(entry.label, expectedLabel);
        QCOMPARE(entry.depth, expectedDepth);
    }
} // namespace

void WhatSonCppRegressionTests::projectsHierarchyParser_roundTripsNestedProjectTree()
{
    const QString rawText = QStringLiteral(R"json(
{
  "projects": [
    {
      "id": "Client",
      "label": "Client",
      "children": [
        {
          "id": "Phase",
          "label": "Phase",
          "children": [
            {
              "label": "Draft"
            }
          ]
        }
      ]
    },
    {
      "id": "Internal",
      "label": "Internal"
    }
  ]
}
)json");

    WhatSonProjectsHierarchyParser parser;
    WhatSonProjectsHierarchyStore store;
    QString parseError;
    QVERIFY2(parser.parse(rawText, &store, &parseError), qPrintable(parseError));

    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QCOMPARE(entries.size(), 4);
    compareProjectEntry(entries.at(0), QStringLiteral("Client"), QStringLiteral("Client"), 0);
    compareProjectEntry(entries.at(1), QStringLiteral("Client/Phase"), QStringLiteral("Phase"), 1);
    compareProjectEntry(entries.at(2), QStringLiteral("Client/Phase/Draft"), QStringLiteral("Draft"), 2);
    compareProjectEntry(entries.at(3), QStringLiteral("Internal"), QStringLiteral("Internal"), 0);

    WhatSonProjectsHierarchyCreator creator;
    const QString serialized = creator.createText(store);
    QVERIFY(serialized.contains(QStringLiteral("\"children\"")));

    WhatSonProjectsHierarchyStore reparsedStore;
    QVERIFY2(parser.parse(serialized, &reparsedStore, &parseError), qPrintable(parseError));

    const QVector<WhatSonFolderDepthEntry> reparsedEntries = reparsedStore.folderEntries();
    QCOMPARE(reparsedEntries.size(), entries.size());
    for (int index = 0; index < entries.size(); ++index)
    {
        compareProjectEntry(
            reparsedEntries.at(index),
            entries.at(index).id,
            entries.at(index).label,
            entries.at(index).depth);
    }
}

void WhatSonCppRegressionTests::projectsHierarchyController_keepsNestedProjectPolicy()
{
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp"));
    const QString supportSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/projects/ProjectsHierarchyControllerSupport.hpp"));

    QVERIFY(!controllerSource.isEmpty());
    QVERIFY(!supportSource.isEmpty());

    QVERIFY(controllerSource.contains(QStringLiteral("entry.depth = std::max(0, item.depth);")));
    QVERIFY(controllerSource.contains(QStringLiteral("item.depth = std::max(0, node.depth);")));
    QVERIFY(controllerSource.contains(QStringLiteral("projectValueMatchesHierarchyItem")));
    QVERIFY(controllerSource.contains(QStringLiteral("availableProjectKeys.insert(projectsHierarchyItemKey")));
    QVERIFY(controllerSource.contains(QStringLiteral("FolderDropPlacement::Child")));
    QVERIFY(controllerSource.contains(QStringLiteral("newBaseDepth = items.at(targetIndex).depth + 1;")));
    QVERIFY(controllerSource.contains(QStringLiteral("resolveFolderMoveOperationFromLvrsMoveEvent")));
    QVERIFY(controllerSource.contains(QStringLiteral("ProjectsHierarchyController::applyHierarchyMove")));
    QVERIFY(!controllerSource.contains(QStringLiteral("reason=projects are flat")));
    QVERIFY(supportSource.contains(QStringLiteral("createNestedHierarchyFolder")));
    QVERIFY(!supportSource.contains(QStringLiteral("createFlatHierarchyFolder")));
}
