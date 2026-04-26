#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/editor/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

void WhatSonCppRegressionTests::noteBodyMountCoordinator_retriesRefreshBeforeFailingMount()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap refreshPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), true);
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), false);
    QCOMPARE(
        refreshPlan.value(QStringLiteral("reason")).toString(),
        QStringLiteral("refresh-after-body-source-pending"));
    QVERIFY(coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());

    coordinator.handleSnapshotRefreshFinished(QStringLiteral("note-1"), false);

    QVERIFY(!coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(coordinator.mountFailed());
    QCOMPARE(coordinator.mountFailureReason(), QStringLiteral("body-source-unavailable"));
    QCOMPARE(
        coordinator.mountFailureMessage(),
        QStringLiteral("No usable note body source was available for the current selection."));
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_requestsEditorSessionMountFromResolvedSnapshot()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QStringLiteral("Resolved body"));
    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(
        QVariantMap{
            {QStringLiteral("focusEditor"), true},
            {QStringLiteral("scheduleReconcile"), true}});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap mountPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(mountPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), false);
    QCOMPARE(mountPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), true);
    QCOMPARE(mountPlan.value(QStringLiteral("focusEditorForSelectedNote")).toBool(), true);
    QCOMPARE(mountPlan.value(QStringLiteral("scheduleSnapshotReconcile")).toBool(), true);
    QCOMPARE(mountPlan.value(QStringLiteral("selectedNoteBodyText")).toString(), QStringLiteral("Resolved body"));
    QCOMPARE(mountPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("mount-editor-session"));

    QVERIFY(!coordinator.mountPending());
    QVERIFY(coordinator.parseMounted());
    QVERIFY(coordinator.sourceMounted());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());
    QVERIFY(coordinator.surfaceVisible());
    QVERIFY(!coordinator.commandSurfaceEnabled());

    const QVariantMap mountState = coordinator.currentMountState();
    QCOMPARE(mountState.value(QStringLiteral("documentSourceReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("selectionSnapshotReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("editorSessionRequiresSelectionMount")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("editorSessionSynchronizedToSelectedSource")).toBool(), false);
    QCOMPARE(mountState.value(QStringLiteral("parseMounted")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("sourceMounted")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("noteMounted")).toBool(), false);

    coordinator.setEditorBoundNoteId(QStringLiteral("note-1"));
    coordinator.setEditorSessionBoundToSelectedNote(true);
    coordinator.setEditorText(QStringLiteral("Resolved body"));

    QVERIFY(coordinator.noteMounted());
    QVERIFY(coordinator.commandSurfaceEnabled());
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_failsMountAfterAcceptedRefreshWhenBodyRemainsUnavailable()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap refreshPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), true);
    QCOMPARE(
        refreshPlan.value(QStringLiteral("reason")).toString(),
        QStringLiteral("refresh-after-body-source-pending"));

    coordinator.handleSnapshotRefreshFinished(QStringLiteral("note-1"), true);
    QVERIFY(coordinator.mountPending());

    coordinator.scheduleMount(
        QVariantMap{
            {QStringLiteral("fallbackRefresh"), true},
            {QStringLiteral("scheduleReconcile"), true}});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap failurePlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(failurePlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), false);
    QCOMPARE(failurePlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), false);
    QCOMPARE(failurePlan.value(QStringLiteral("fallbackRefreshIfMountSkipped")).toBool(), true);
    QCOMPARE(failurePlan.value(QStringLiteral("scheduleSnapshotReconcile")).toBool(), true);
    QCOMPARE(
        failurePlan.value(QStringLiteral("reason")).toString(),
        QStringLiteral("body-source-unavailable"));

    QVERIFY(!coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(coordinator.mountFailed());
    QCOMPARE(coordinator.mountFailureReason(), QStringLiteral("body-source-unavailable"));
    QCOMPARE(
        coordinator.mountFailureMessage(),
        QStringLiteral("No usable note body source was available for the current selection."));
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_reportsSurfaceSpecificFailureMessage()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QStringLiteral("Resolved body"));
    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.setStructuredDocumentSurfaceRequested(true);
    coordinator.setStructuredDocumentSurfaceReady(false);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap mountPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(mountPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), false);
    QCOMPARE(mountPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), true);
    QCOMPARE(mountPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("mount-editor-session"));

    QVERIFY(!coordinator.mountPending());
    QVERIFY(coordinator.parseMounted());
    QVERIFY(coordinator.sourceMounted());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(coordinator.mountFailed());
    QCOMPARE(
        coordinator.mountFailureReason(),
        QStringLiteral("structured-document-surface-unavailable"));
    QCOMPARE(
        coordinator.mountFailureMessage(),
        QStringLiteral("The structured document surface did not become ready for the selected note."));

    const QVariantMap mountState = coordinator.currentMountState();
    QCOMPARE(
        mountState.value(QStringLiteral("mountFailureReason")).toString(),
        QStringLiteral("structured-document-surface-unavailable"));
    QCOMPARE(
        mountState.value(QStringLiteral("mountFailureMessage")).toString(),
        QStringLiteral("The structured document surface did not become ready for the selected note."));
    QCOMPARE(coordinator.exceptionReason(), QStringLiteral("structured-document-surface-unavailable"));
    QCOMPARE(coordinator.exceptionTitle(), QStringLiteral("Document surface unavailable"));
    QCOMPARE(
        coordinator.exceptionMessage(),
        QStringLiteral("The structured document surface did not become ready for the selected note."));
    QVERIFY(!coordinator.exceptionVisible());
    QVERIFY(!coordinator.commandSurfaceEnabled());
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_acceptsResolvedEmptySelectedBody()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QString());
    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.setStructuredDocumentSurfaceRequested(true);
    coordinator.setStructuredDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap mountPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(mountPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), false);
    QCOMPARE(mountPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), true);
    QCOMPARE(mountPlan.value(QStringLiteral("selectedNoteBodyText")).toString(), QString());
    QCOMPARE(mountPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("mount-editor-session"));

    QVERIFY(!coordinator.mountPending());
    QVERIFY(coordinator.parseMounted());
    QVERIFY(coordinator.sourceMounted());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());
    QVERIFY(coordinator.surfaceVisible());
    QVERIFY(!coordinator.exceptionVisible());
    QVERIFY(!coordinator.commandSurfaceEnabled());

    const QVariantMap mountState = coordinator.currentMountState();
    QCOMPARE(mountState.value(QStringLiteral("documentSourceReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("selectionSnapshotReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("parseMounted")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("noteMounted")).toBool(), false);

    coordinator.setEditorBoundNoteId(QStringLiteral("note-1"));
    coordinator.setEditorSessionBoundToSelectedNote(true);
    coordinator.setEditorText(QString());

    QVERIFY(coordinator.noteMounted());
    QVERIFY(coordinator.commandSurfaceEnabled());
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_hidesExceptionUntilPendingMountSettles()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap refreshPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), true);
    QCOMPARE(
        refreshPlan.value(QStringLiteral("reason")).toString(),
        QStringLiteral("refresh-after-body-source-pending"));

    QVERIFY(coordinator.mountPending());
    QVERIFY(!coordinator.mountFailed());
    QVERIFY(!coordinator.parseMounted());
    QVERIFY(!coordinator.exceptionVisible());

    coordinator.handleSnapshotRefreshFinished(QStringLiteral("note-1"), false);

    QVERIFY(!coordinator.mountPending());
    QVERIFY(coordinator.mountFailed());
    QVERIFY(coordinator.exceptionVisible());
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_waitsForPresentationReadySourceBeforeMounting()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setEditorBoundNoteId(QStringLiteral("note-1"));
    coordinator.setEditorSessionBoundToSelectedNote(true);
    coordinator.setEditorText(QString());
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap refreshPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), true);
    QCOMPARE(refreshPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), false);
    QCOMPARE(
        refreshPlan.value(QStringLiteral("reason")).toString(),
        QStringLiteral("refresh-after-body-source-pending"));

    QVERIFY(coordinator.mountPending());
    QVERIFY(!coordinator.parseMounted());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());
}

void WhatSonCppRegressionTests::noteBodyMountCoordinator_remountsSameNoteWhenEditorSessionTextIsStale()
{
    ensureCoreApplication();

    ContentsDisplayNoteBodyMountCoordinator coordinator;
    QSignalSpy mountFlushRequestedSpy(
        &coordinator,
        &ContentsDisplayNoteBodyMountCoordinator::mountFlushRequested);

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QStringLiteral("Resolved body"));
    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.setEditorBoundNoteId(QStringLiteral("note-1"));
    coordinator.setEditorSessionBoundToSelectedNote(true);
    coordinator.setEditorText(QStringLiteral("Stale body"));
    coordinator.setInlineDocumentSurfaceRequested(true);
    coordinator.setInlineDocumentSurfaceReady(true);

    coordinator.scheduleMount(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(mountFlushRequestedSpy.count(), 1);
    const QVariantMap mountPlan = mountFlushRequestedSpy.takeFirst().at(0).toMap();
    QCOMPARE(mountPlan.value(QStringLiteral("attemptSnapshotRefresh")).toBool(), false);
    QCOMPARE(mountPlan.value(QStringLiteral("attemptEditorSessionMount")).toBool(), true);
    QCOMPARE(
        mountPlan.value(QStringLiteral("selectedNoteBodyText")).toString(),
        QStringLiteral("Resolved body"));
    QCOMPARE(mountPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("mount-editor-session"));

    QVERIFY(coordinator.parseMounted());
    QVERIFY(coordinator.sourceMounted());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.commandSurfaceEnabled());

    const QVariantMap mountState = coordinator.currentMountState();
    QCOMPARE(mountState.value(QStringLiteral("selectedBodyReadyForPresentation")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("editorSessionBoundToSelectedNoteId")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("editorSessionRequiresSelectionMount")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("editorSessionSynchronizedToSelectedSource")).toBool(), false);

    coordinator.setEditorText(QStringLiteral("Resolved body"));

    QVERIFY(coordinator.noteMounted());
    QVERIFY(coordinator.commandSurfaceEnabled());
    QCOMPARE(
        coordinator.currentMountState().value(QStringLiteral("editorSessionSynchronizedToSelectedSource")).toBool(),
        true);
}
