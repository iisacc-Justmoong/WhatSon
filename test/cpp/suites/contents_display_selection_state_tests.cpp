#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/content/display/ContentsDisplaySelectionSyncCoordinator.hpp"
#include "app/models/editor/display/ContentsDisplayDocumentSourceResolver.hpp"

void WhatSonCppRegressionTests::contentsDisplaySessionCoordinator_requiresResolvedSelectedBodyBeforeUsingSnapshot()
{
    ContentsDisplayDocumentSourceResolver resolver;
    QSignalSpy documentSourcePlanChangedSpy(
        &resolver,
        &ContentsDisplayDocumentSourceResolver::documentSourcePlanChanged);
    QSignalSpy documentPresentationSourceTextChangedSpy(
        &resolver,
        &ContentsDisplayDocumentSourceResolver::documentPresentationSourceTextChanged);

    resolver.setSelectedNoteId(QStringLiteral("note-1"));
    resolver.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    resolver.setSelectedNoteBodyText(QStringLiteral("Selection body"));

    const QVariantMap unresolvedPlan = resolver.documentSourcePlan();
    QCOMPARE(unresolvedPlan.value(QStringLiteral("bodyMatchesSelection")).toBool(), true);
    QCOMPARE(unresolvedPlan.value(QStringLiteral("bodyAvailable")).toBool(), true);
    QCOMPARE(unresolvedPlan.value(QStringLiteral("resolvedSourceReady")).toBool(), true);
    QCOMPARE(
        unresolvedPlan.value(QStringLiteral("resolvedSourceText")).toString(),
        QStringLiteral("Selection body"));
    QCOMPARE(resolver.documentPresentationSourceText(), QString());
    QCOMPARE(resolver.resolvedDocumentPresentationSourceText(), QString());
    QVERIFY(documentSourcePlanChangedSpy.count() > 0);
    QCOMPARE(documentPresentationSourceTextChangedSpy.count(), 0);

    documentSourcePlanChangedSpy.clear();
    documentPresentationSourceTextChangedSpy.clear();
    resolver.setSelectedNoteBodyResolved(true);
    const QVariantMap resolvedPlan = resolver.documentSourcePlan();
    QCOMPARE(resolvedPlan.value(QStringLiteral("bodyMatchesSelection")).toBool(), true);
    QCOMPARE(resolvedPlan.value(QStringLiteral("bodyAvailable")).toBool(), true);
    QCOMPARE(resolvedPlan.value(QStringLiteral("resolvedSourceReady")).toBool(), true);
    QCOMPARE(
        resolver.documentPresentationSourceText(),
        QStringLiteral("Selection body"));
    QCOMPARE(
        resolver.resolvedDocumentPresentationSourceText(),
        QStringLiteral("Selection body"));
    QCOMPARE(documentSourcePlanChangedSpy.count(), 0);
    QCOMPARE(documentPresentationSourceTextChangedSpy.count(), 1);

    documentSourcePlanChangedSpy.clear();
    documentPresentationSourceTextChangedSpy.clear();
    resolver.setEditorBoundNoteId(QStringLiteral("note-1"));
    resolver.setEditorText(QStringLiteral("Editor body"));
    const QVariantMap editorPlan = resolver.documentSourcePlan();
    QCOMPARE(editorPlan.value(QStringLiteral("editorSessionBoundToSelectedNote")).toBool(), true);
    QCOMPARE(editorPlan.value(QStringLiteral("preferEditorSessionSource")).toBool(), true);
    QCOMPARE(
        resolver.documentPresentationSourceText(),
        QStringLiteral("Editor body"));
    QCOMPARE(
        resolver.resolvedDocumentPresentationSourceText(),
        QStringLiteral("Editor body"));
    QVERIFY(documentSourcePlanChangedSpy.count() > 0);
    QVERIFY(documentPresentationSourceTextChangedSpy.count() > 0);

    documentSourcePlanChangedSpy.clear();
    documentPresentationSourceTextChangedSpy.clear();
    resolver.setSelectedNoteId(QStringLiteral("note-empty"));
    resolver.setSelectedNoteBodyNoteId(QStringLiteral("note-empty"));
    resolver.setSelectedNoteBodyText(QString());
    resolver.setSelectedNoteBodyResolved(true);
    resolver.setEditorBoundNoteId(QString());
    resolver.setEditorText(QString());

    const QVariantMap emptyBodyPlan = resolver.documentSourcePlan();
    QCOMPARE(emptyBodyPlan.value(QStringLiteral("bodyMatchesSelection")).toBool(), true);
    QCOMPARE(emptyBodyPlan.value(QStringLiteral("bodyAvailable")).toBool(), true);
    QCOMPARE(emptyBodyPlan.value(QStringLiteral("resolvedSourceReady")).toBool(), true);
    QCOMPARE(resolver.documentPresentationSourceText(), QString());
    QCOMPARE(
        resolver.resolvedDocumentPresentationSourceText(),
        QString());
    QVERIFY(documentSourcePlanChangedSpy.count() > 0);
    QVERIFY(documentPresentationSourceTextChangedSpy.count() > 0);
}

void WhatSonCppRegressionTests::contentsDisplayCreationPath_emitsCoordinatorTraceForEditorWiring()
{
    const QString documentSourceResolverSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayDocumentSourceResolver.cpp"));
    const QString editorSessionSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString mountCoordinatorSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/content/display/ContentsDisplayNoteBodyMountCoordinator.cpp"));

    QVERIFY(!documentSourceResolverSource.isEmpty());
    QVERIFY(!editorSessionSource.isEmpty());
    QVERIFY(!mountCoordinatorSource.isEmpty());

    QVERIFY(documentSourceResolverSource.contains(
        QStringLiteral("resolvedDocumentPresentationSourceText")));
    QVERIFY(documentSourceResolverSource.contains(
        QStringLiteral("currentMinimapSourceText")));
    QVERIFY(documentSourceResolverSource.contains(
        QStringLiteral("normalizedDocumentSourceMutation")));
    QVERIFY(documentSourceResolverSource.contains(
        QStringLiteral("editorSessionBoundToSelectedNote() const noexcept")));

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
    QVariantMap firstPlan = flushSpy.takeFirst().at(0).toMap();
    QCOMPARE(firstPlan.value(QStringLiteral("attemptSelectionSync")).toBool(), true);
    QCOMPARE(firstPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("selection-sync"));
    QCOMPARE(firstPlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), false);

    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.scheduleSelectionSync(QVariantMap{});
    QCoreApplication::processEvents();

    QCOMPARE(flushSpy.count(), 1);
    const QVariantMap secondPlan = flushSpy.takeFirst().at(0).toMap();
    QCOMPARE(secondPlan.value(QStringLiteral("attemptSelectionSync")).toBool(), true);
    QCOMPARE(secondPlan.value(QStringLiteral("reason")).toString(), QStringLiteral("selection-sync"));
    QCOMPARE(secondPlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), true);
}

void WhatSonCppRegressionTests::contentsDisplaySelectionSyncCoordinator_snapshotPlansRetainSelectionContext()
{
    ensureCoreApplication();

    ContentsDisplaySelectionSyncCoordinator coordinator;

    coordinator.setVisible(true);
    coordinator.setSelectedNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyNoteId(QStringLiteral("note-1"));
    coordinator.setSelectedNoteBodyText(QStringLiteral("Snapshot body"));
    coordinator.setSelectedNoteBodyResolved(true);
    coordinator.setEditorBoundNoteId(QStringLiteral("note-1"));
    coordinator.setEditorSessionBoundToSelectedNote(true);

    const QVariantMap pollPlan = coordinator.snapshotPollPlan();
    QCOMPARE(pollPlan.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(pollPlan.value(QStringLiteral("selectedNoteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(
        pollPlan.value(QStringLiteral("selectedNoteBodyNoteId")).toString(),
        QStringLiteral("note-1"));
    QCOMPARE(pollPlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), true);
    QCOMPARE(
        pollPlan.value(QStringLiteral("selectedNoteBodyText")).toString(),
        QStringLiteral("Snapshot body"));
    QCOMPARE(pollPlan.value(QStringLiteral("allowSnapshotRefresh")).toBool(), true);
    QCOMPARE(pollPlan.value(QStringLiteral("attemptReconcile")).toBool(), true);

    coordinator.setVisible(false);

    const QVariantMap inactivePlan = coordinator.snapshotReconcilePlan();
    QCOMPARE(inactivePlan.value(QStringLiteral("reason")).toString(), QStringLiteral("inactive-host"));
    QCOMPARE(inactivePlan.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(inactivePlan.value(QStringLiteral("selectedNoteId")).toString(), QStringLiteral("note-1"));
    QCOMPARE(
        inactivePlan.value(QStringLiteral("selectedNoteBodyNoteId")).toString(),
        QStringLiteral("note-1"));
    QCOMPARE(inactivePlan.value(QStringLiteral("selectedNoteBodyResolved")).toBool(), true);
}
