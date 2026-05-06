#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorPersistenceController_definesEditorPersistenceBoundary()
{
    const QString persistenceHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorPersistenceController.hpp"));
    const QString persistenceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp"));
    const QString saveCoordinatorHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorSaveCoordinator.hpp"));
    const QString saveCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/persistence/ContentsEditorSaveCoordinator.cpp"));
    const QString oldIdleSyncSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/ContentsEditorIdleSyncController.cpp"));
    const QString selectionBridgeHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"));
    const QString selectionBridgeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp"));
    const QString sessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString testCMakeSource = readUtf8SourceFile(QStringLiteral("test/cpp/CMakeLists.txt"));

    QVERIFY(!persistenceHeader.isEmpty());
    QVERIFY(!persistenceSource.isEmpty());
    QVERIFY(oldIdleSyncSource.isEmpty());

    QVERIFY(persistenceHeader.contains(QStringLiteral("class ContentsEditorPersistenceController final : public QObject")));
    QVERIFY(persistenceHeader.contains(QStringLiteral(
        "contentPersistenceContractAvailable READ contentPersistenceContractAvailable")));
    QVERIFY(persistenceHeader.contains(QStringLiteral(
        "directPersistenceContractAvailable READ directPersistenceAvailable")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("Q_INVOKABLE bool stageEditorTextForPersistence")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("Q_INVOKABLE bool stageEditorTextForIdleSync")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("Q_INVOKABLE bool flushEditorTextForNote")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("pendingEditorTextForNote")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("noteBodyTextLoaded")));
    QVERIFY(persistenceHeader.contains(QStringLiteral("viewSessionSnapshotReconciled")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("class ContentsEditorSaveCoordinator final : public QObject")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("Q_INVOKABLE bool commitEditedSourceText")));
    QVERIFY(saveCoordinatorHeader.contains(QStringLiteral("Q_INVOKABLE bool syncEditorSessionFromSelection")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("new ContentsEditorPersistenceController(this)")));
    QVERIFY(saveCoordinatorSource.contains(QStringLiteral("flushPendingEditorText")));

    QVERIFY(persistenceSource.contains(QStringLiteral("ContentsNoteManagementCoordinator")));
    QVERIFY(persistenceSource.contains(QStringLiteral("constexpr int kEditorPersistenceFetchIntervalMs = 1000")));
    QVERIFY(persistenceSource.contains(QStringLiteral("m_bufferedSnapshotsByNote")));
    QVERIFY(persistenceSource.contains(QStringLiteral("m_lastPersistedTextByNote")));
    QVERIFY(persistenceSource.contains(QStringLiteral("QStringLiteral(\"editorPersistence\")")));
    QVERIFY(!persistenceSource.contains(QStringLiteral("QStringLiteral(\"idleSync\")")));

    QVERIFY(selectionBridgeHeader.contains(QStringLiteral("class ContentsEditorPersistenceController;")));
    QVERIFY(selectionBridgeHeader.contains(QStringLiteral(
        "QPointer<ContentsEditorPersistenceController> m_persistenceController;")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("ContentsEditorIdleSyncController")));
    QVERIFY(selectionBridgeSource.contains(QStringLiteral(
        "#include \"app/models/editor/persistence/ContentsEditorPersistenceController.hpp\"")));
    QVERIFY(selectionBridgeSource.contains(QStringLiteral("new ContentsEditorPersistenceController(this)")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("flushEditorTextForNote")));
    QVERIFY(!selectionBridgeHeader.contains(QStringLiteral("stageEditorTextForIdleSync")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("models/file/sync/ContentsEditorIdleSyncController")));
    QVERIFY(!selectionBridgeSource.contains(QStringLiteral("m_idleSyncController")));

    QVERIFY(!sessionSource.contains(QStringLiteral("stageEditorTextForIdleSync")));
    QVERIFY(!sessionSource.contains(QStringLiteral("flushEditorTextForNote")));
    QVERIFY(!sessionSource.contains(QStringLiteral("ContentsNoteManagementCoordinator")));

    QVERIFY(testCMakeSource.contains(QStringLiteral(
        "src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp")));
    QVERIFY(testCMakeSource.contains(QStringLiteral(
        "src/app/models/editor/persistence/ContentsEditorSaveCoordinator.cpp")));
    QVERIFY(!testCMakeSource.contains(QStringLiteral(
        "src/app/models/file/sync/ContentsEditorIdleSyncController.cpp")));
}
