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
