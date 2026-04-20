#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::foldersHierarchyParser_escapesLiteralSlashLabelsIntoSingleSegments()
{
    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    QString errorMessage;
    bool uuidMigrationRequired = false;

    const QString rawText = QStringLiteral(R"JSON(
{
  "version": 1,
  "schema": "whatson.folders.tree",
  "folders": [
    {
      "id": "Marketing/Sales",
      "label": "Marketing/Sales",
      "depth": 0,
      "uuid": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
      "children": [
        {
          "id": "Marketing/Sales/Pipeline",
          "label": "Pipeline",
          "depth": 1,
          "uuid": "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        }
      ]
    }
  ]
}
)JSON");

    QVERIFY(parser.parse(rawText, &store, &errorMessage, &uuidMigrationRequired));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(!uuidMigrationRequired);

    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).label, QStringLiteral("Marketing/Sales"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(0).id, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(entries.at(1).label, QStringLiteral("Pipeline"));
    QCOMPARE(entries.at(1).depth, 1);
    QCOMPARE(entries.at(1).id, QStringLiteral("Marketing\\/Sales/Pipeline"));
}
