#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::libraryHierarchyController_keepsInAppScaffoldIndependentFromHubSnapshots()
{
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/hierarchy/library/LibraryHierarchyController.cpp"));
    const QString controllerDoc = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/hierarchy/library/LibraryHierarchyController.cpp.md"));

    QVERIFY(controllerSource.contains(QStringLiteral("constexpr auto kLibraryDraftLabel = \"Drafts\";")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryAllLabel), LibraryHierarchyItem::SystemBucket::All)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryDraftLabel), LibraryHierarchyItem::SystemBucket::Draft)")));
    QVERIFY(controllerSource.contains(QStringLiteral("makeSystemBucketItem(QLatin1String(kLibraryTodayLabel), LibraryHierarchyItem::SystemBucket::Today)")));
    QVERIFY(controllerSource.contains(QStringLiteral("QVector<LibraryHierarchyItem> prependInAppLibraryScaffold(QVector<LibraryHierarchyItem> items)")));
    QVERIFY(controllerSource.contains(QStringLiteral("bool hasCompleteInAppLibraryScaffold(const QVector<LibraryHierarchyItem>& items)")));
    QVERIFY(controllerSource.contains(QStringLiteral("void LibraryHierarchyController::applyInAppLibraryScaffold()")));
    QVERIFY(controllerSource.contains(QStringLiteral("m_items = prependInAppLibraryScaffold({});")));
    QVERIFY(controllerSource.contains(QStringLiteral("const bool hierarchySourceChanged =\n        !hasCompleteInAppLibraryScaffold(m_items) || !folderDepthEntriesEqual(currentFolderEntries, folderEntries);")));
    QVERIFY(controllerSource.contains(QStringLiteral("applyInAppLibraryScaffold();\n    setSelectedIndex(-1);")));
    QVERIFY(!controllerSource.contains(QStringLiteral("m_items.clear();")));

    QVERIFY(controllerDoc.contains(QStringLiteral("hub-independent in-app scaffold")));
    QVERIFY(controllerDoc.contains(QStringLiteral("`All Library`, `Drafts`, and `Today`")));
}

void WhatSonCppRegressionTests::libraryHierarchyController_appliesLvrsMoveEventAsSingleFolderReparent()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    const QString foldersFilePath = QDir(workspaceDir.path()).filePath(QStringLiteral("Folders.wsfolders"));

    const QString gitUuid =
        QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    const QString branchUuid =
        QStringLiteral("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");

    QVector<WhatSonFolderDepthEntry> entries;
    entries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("git"),
        QStringLiteral("git"),
        0,
        gitUuid,
    });
    entries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("branch"),
        QStringLiteral("branch"),
        0,
        branchUuid,
    });

    LibraryHierarchyController controller;
    QSignalSpy filesystemMutatedSpy(&controller, &LibraryHierarchyController::hubFilesystemMutated);
    controller.applyRuntimeSnapshot(
        QStringLiteral("test.wshub"),
        {},
        {},
        {},
        entries,
        foldersFilePath,
        true);

    const QVariantList initialModel = controller.hierarchyModel();
    QCOMPARE(initialModel.size(), 5);
    QCOMPARE(initialModel.at(3).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("git"));
    QCOMPARE(initialModel.at(4).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("branch"));

    QVERIFY(controller.applyHierarchyMove(3, 4, 1, initialModel.at(3).toMap().value(QStringLiteral("key")).toString()));

    const QVariantList movedModel = controller.hierarchyModel();
    QCOMPARE(movedModel.size(), 5);
    QCOMPARE(movedModel.at(3).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("branch"));
    QCOMPARE(movedModel.at(3).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedModel.at(3).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("branch"));
    QCOMPARE(movedModel.at(4).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("git"));
    QCOMPARE(movedModel.at(4).toMap().value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(movedModel.at(4).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("branch/git"));
    QCOMPARE(filesystemMutatedSpy.count(), 1);

    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    QString parseError;
    bool migrationRequired = false;
    QVERIFY(parser.parse(readUtf8SourceFile(foldersFilePath), &store, &parseError, &migrationRequired));
    QVERIFY2(parseError.isEmpty(), qPrintable(parseError));
    QVERIFY(!migrationRequired);

    const QVector<WhatSonFolderDepthEntry> persistedEntries = store.folderEntries();
    QCOMPARE(persistedEntries.size(), 2);
    QCOMPARE(persistedEntries.at(0).id, QStringLiteral("branch"));
    QCOMPARE(persistedEntries.at(0).label, QStringLiteral("branch"));
    QCOMPARE(persistedEntries.at(0).depth, 0);
    QCOMPARE(persistedEntries.at(0).uuid, branchUuid);
    QCOMPARE(persistedEntries.at(1).id, QStringLiteral("branch/git"));
    QCOMPARE(persistedEntries.at(1).label, QStringLiteral("git"));
    QCOMPARE(persistedEntries.at(1).depth, 1);
    QCOMPARE(persistedEntries.at(1).uuid, gitUuid);
}

void WhatSonCppRegressionTests::libraryHierarchyController_mirrorsFoldersFileAfterHierarchyCommit()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    const QString foldersFilePath = QDir(workspaceDir.path()).filePath(QStringLiteral("Folders.wsfolders"));

    const QString gitUuid =
        QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    const QString branchUuid =
        QStringLiteral("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");

    QVector<WhatSonFolderDepthEntry> entries;
    entries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("git"),
        QStringLiteral("git"),
        0,
        gitUuid,
    });
    entries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("branch"),
        QStringLiteral("branch"),
        0,
        branchUuid,
    });

    LibraryHierarchyController controller;
    controller.applyRuntimeSnapshot(
        QStringLiteral("test.wshub"),
        {},
        {},
        {},
        entries,
        foldersFilePath,
        true);

    const QVariantList initialModel = controller.hierarchyModel();
    QCOMPARE(initialModel.size(), 5);

    QVariantMap gitNode = initialModel.at(3).toMap();
    gitNode.insert(QStringLiteral("sourceIndex"), 3);
    gitNode.insert(QStringLiteral("itemId"), 3);
    gitNode.insert(QStringLiteral("depth"), 0);

    QVariantMap transientNode;
    transientNode.insert(QStringLiteral("key"), QStringLiteral("transient-empty-folder-row"));
    transientNode.insert(QStringLiteral("id"), QStringLiteral("transient-empty-folder-row"));
    transientNode.insert(QStringLiteral("label"), QString());
    transientNode.insert(QStringLiteral("sourceIndex"), 99);
    transientNode.insert(QStringLiteral("itemId"), 99);
    transientNode.insert(QStringLiteral("depth"), 1);
    transientNode.insert(QStringLiteral("expanded"), false);
    transientNode.insert(QStringLiteral("showChevron"), false);

    QVariantMap branchNode = initialModel.at(4).toMap();
    branchNode.insert(QStringLiteral("sourceIndex"), 4);
    branchNode.insert(QStringLiteral("itemId"), 4);
    branchNode.insert(QStringLiteral("depth"), 0);

    QVariantList committedNodes;
    committedNodes.push_back(gitNode);
    committedNodes.push_back(transientNode);
    committedNodes.push_back(branchNode);

    QVERIFY(controller.applyHierarchyNodes(
        committedNodes,
        gitNode.value(QStringLiteral("key")).toString()));

    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    QString parseError;
    bool migrationRequired = false;
    QVERIFY(parser.parse(readUtf8SourceFile(foldersFilePath), &store, &parseError, &migrationRequired));
    QVERIFY2(parseError.isEmpty(), qPrintable(parseError));
    QVERIFY(!migrationRequired);
    const QVector<WhatSonFolderDepthEntry> persistedEntries = store.folderEntries();
    QCOMPARE(persistedEntries.size(), 2);
    QCOMPARE(persistedEntries.at(0).label, QStringLiteral("git"));
    QCOMPARE(persistedEntries.at(1).label, QStringLiteral("branch"));

    const QVariantList mirroredModel = controller.hierarchyModel();
    QCOMPARE(mirroredModel.size(), 5);
    QCOMPARE(mirroredModel.at(3).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("git"));
    QCOMPARE(mirroredModel.at(3).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("git"));
    QCOMPARE(mirroredModel.at(4).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("branch"));
    QCOMPARE(mirroredModel.at(4).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("branch"));
}
