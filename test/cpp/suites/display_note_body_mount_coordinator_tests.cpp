#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

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
        QStringLiteral("refresh-after-body-note-mismatch"));
    QVERIFY(coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());

    coordinator.handleSnapshotRefreshFinished(QStringLiteral("note-1"), false);

    QVERIFY(!coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(coordinator.mountFailed());
    QCOMPARE(coordinator.mountFailureMessage(), QStringLiteral("No document opened"));
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
    QVERIFY(coordinator.noteMounted());
    QVERIFY(!coordinator.mountFailed());
    QVERIFY(coordinator.surfaceVisible());

    const QVariantMap mountState = coordinator.currentMountState();
    QCOMPARE(mountState.value(QStringLiteral("documentSourceReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("selectionSnapshotReady")).toBool(), true);
    QCOMPARE(mountState.value(QStringLiteral("noteMounted")).toBool(), true);
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
        QStringLiteral("refresh-after-body-note-mismatch"));

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
        QStringLiteral("body-note-mismatch"));

    QVERIFY(!coordinator.mountPending());
    QVERIFY(!coordinator.noteMounted());
    QVERIFY(coordinator.mountFailed());
    QCOMPARE(coordinator.mountFailureMessage(), QStringLiteral("No document opened"));
}
