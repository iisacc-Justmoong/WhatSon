#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/content/display/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "app/models/content/display/ContentsDisplaySessionCoordinator.hpp"

void WhatSonCppRegressionTests::contentsDisplaySessionCoordinator_requiresResolvedSelectedBodyBeforeUsingSnapshot()
{
    ContentsDisplaySessionCoordinator coordinator;

    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QStringLiteral("Selection body"));

    QCOMPARE(coordinator.resolvedDocumentPresentationSourceText(), QString());

    coordinator.setSelectedNoteBodyResolved(true);
    QCOMPARE(
        coordinator.resolvedDocumentPresentationSourceText(),
        QStringLiteral("Selection body"));

    coordinator.setEditorText(QStringLiteral("Editor body"));
    coordinator.setEditorSessionBoundToSelectedNote(true);
    QCOMPARE(
        coordinator.resolvedDocumentPresentationSourceText(),
        QStringLiteral("Editor body"));
}

void WhatSonCppRegressionTests::contentsDisplayCreationPath_emitsCoordinatorTraceForEditorWiring()
{
    const QString sessionCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/display/ContentsDisplaySessionCoordinator.cpp"));
    const QString editorSessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString mountCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.cpp"));

    QVERIFY(!sessionCoordinatorSource.isEmpty());
    QVERIFY(!editorSessionSource.isEmpty());
    QVERIFY(!mountCoordinatorSource.isEmpty());

    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("#include \"app/models/file/WhatSonDebugTrace.hpp\"")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"displaySessionCoordinator\")")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setSelectedNoteId\")")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setSelectedNoteBodyNoteId\")")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setSelectedNoteBodyText\")")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setEditorSessionBoundToSelectedNote\")")));
    QVERIFY(sessionCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setStructuredFlowSourceText\")")));

    QVERIFY(editorSessionSource.contains(
        QStringLiteral("QStringLiteral(\"setSelectionBridge\")")));
    QVERIFY(editorSessionSource.contains(
        QStringLiteral("QStringLiteral(\"selectionBridge={%1}\")")));
    QVERIFY(editorSessionSource.contains(
        QStringLiteral("QStringLiteral(\"setAgendaBackend\")")));
    QVERIFY(editorSessionSource.contains(
        QStringLiteral("QStringLiteral(\"agendaBackend={%1}\")")));

    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setInlineDocumentSurfaceRequested\")")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setInlineDocumentSurfaceReady\")")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setInlineDocumentSurfaceLoading\")")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setStructuredDocumentSurfaceRequested\")")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setStructuredDocumentSurfaceReady\")")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"setPendingMountNoteId\")")));
}

void WhatSonCppRegressionTests::contentsDisplaySelectionFlow_emitsTraceForSelectionAndMountPlans()
{
    const QString selectionSyncSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/display/ContentsDisplaySelectionSyncCoordinator.cpp"));
    const QString mountCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.cpp"));

    QVERIFY(!selectionSyncSource.isEmpty());
    QVERIFY(!mountCoordinatorSource.isEmpty());

    QVERIFY(selectionSyncSource.contains(QStringLiteral("summarizeSelectionSyncPlan")));
    QVERIFY(selectionSyncSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.snapshotPollPlan\")")));
    QVERIFY(selectionSyncSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.snapshotReconcilePlan\")")));
    QVERIFY(selectionSyncSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.flushPlan\")")));
    QVERIFY(selectionSyncSource.contains(QStringLiteral("summarizeSelectionSyncPlan(plan)")));
    QVERIFY(selectionSyncSource.contains(
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 resetSnapshot=%3")));

    QVERIFY(mountCoordinatorSource.contains(QStringLiteral("summarizeMountPlan")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("QStringLiteral(\"selectionFlow.mountPlan\")")));
    QVERIFY(mountCoordinatorSource.contains(QStringLiteral("summarizeMountPlan(plan)")));
    QVERIFY(mountCoordinatorSource.contains(
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 resetSnapshot=%3")));
}

void WhatSonCppRegressionTests::contentsDisplaySelectionSyncCoordinator_blocksUntilSelectedBodyIsResolved()
{
    ensureCoreApplication();

    ContentsDisplaySelectionSyncCoordinator coordinator;
    QSignalSpy flushSpy(
        &coordinator,
        &ContentsDisplaySelectionSyncCoordinator::selectionSyncFlushRequested);

    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QString());
    coordinator.setSelectedNoteBodyResolved(false);
    coordinator.setSelectedNoteBodyLoading(false);

    coordinator.scheduleSelectionSync(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(flushSpy.count(), 1);
    QVariantMap blockedPlan = flushSpy.takeFirst().at(0).toMap();
    QCOMPARE(blockedPlan.value(QStringLiteral("attemptSelectionSync")).toBool(), false);
    QCOMPARE(blockedPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("body-unresolved"));
    QCOMPARE(blockedPlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), false);

    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.scheduleSelectionSync(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(flushSpy.count(), 1);
    const QVariantMap readyPlan = flushSpy.takeFirst().at(0).toMap();
    QCOMPARE(readyPlan.value(QStringLiteral("attemptSelectionSync")).toBool(), true);
    QCOMPARE(readyPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("selection-sync"));
    QCOMPARE(readyPlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), true);
}
