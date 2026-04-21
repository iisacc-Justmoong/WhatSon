#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::foldersHierarchySessionService_preservesEscapedLiteralSlashFolderPaths()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString contentsPath = workspaceDir.filePath(QStringLiteral("Workspace.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString noteDirectoryPath = QDir(contentsPath).filePath(QStringLiteral("Notes/Current"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    QFile foldersFile(foldersFilePath);
    QVERIFY(foldersFile.open(QIODevice::WriteOnly | QIODevice::Text));
    foldersFile.write(R"JSON(
{
  "version": 1,
  "schema": "whatson.folders.tree",
  "folders": [
    {
      "id": "Marketing\\/Sales",
      "label": "Marketing/Sales",
      "depth": 0,
      "uuid": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    }
  ]
}
)JSON");
    foldersFile.close();

    WhatSonFoldersHierarchySessionService service;
    WhatSonFoldersHierarchySessionService::FolderResolution resolution;
    QString errorMessage;

    QVERIFY(service.ensureFolderEntry(
        noteDirectoryPath,
        QStringLiteral("Marketing\\/Sales"),
        &resolution,
        &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(resolution.folderPath, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(resolution.folderUuid, QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    QCOMPARE(resolution.foldersFilePath, foldersFilePath);
    QVERIFY(!resolution.folderCreated);
    QVERIFY(!resolution.hierarchyChanged);

    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    bool uuidMigrationRequired = false;

    QVERIFY(foldersFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString persistedText = QString::fromUtf8(foldersFile.readAll());
    foldersFile.close();

    QVERIFY(parser.parse(persistedText, &store, &errorMessage, &uuidMigrationRequired));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(!uuidMigrationRequired);

    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.constFirst().id, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(entries.constFirst().label, QStringLiteral("Marketing/Sales"));
    QCOMPARE(entries.constFirst().depth, 0);
}
