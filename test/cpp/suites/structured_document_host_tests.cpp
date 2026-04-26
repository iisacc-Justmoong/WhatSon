#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::structuredDocumentHost_tracksSelectionClearRevisionAcrossInteractions()
{
    ContentsStructuredDocumentHost host;

    QSignalSpy activeBlockIndexSpy(&host, &ContentsStructuredDocumentHost::activeBlockIndexChanged);
    QSignalSpy activeBlockCursorRevisionSpy(
        &host,
        &ContentsStructuredDocumentHost::activeBlockCursorRevisionChanged);
    QSignalSpy selectionClearRevisionSpy(
        &host,
        &ContentsStructuredDocumentHost::selectionClearRevisionChanged);
    QSignalSpy selectionClearRetainedBlockIndexSpy(
        &host,
        &ContentsStructuredDocumentHost::selectionClearRetainedBlockIndexChanged);

    QCOMPARE(host.activeBlockIndex(), -1);
    QCOMPARE(host.activeBlockCursorRevision(), 0);
    QCOMPARE(host.selectionClearRevision(), 0);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), -1);

    host.requestSelectionClear(3);
    QCOMPARE(host.selectionClearRevision(), 1);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 3);
    QCOMPARE(selectionClearRevisionSpy.count(), 1);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 1);

    host.requestSelectionClear(3);
    QCOMPARE(host.selectionClearRevision(), 2);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 3);
    QCOMPARE(selectionClearRevisionSpy.count(), 2);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 1);

    host.noteActiveBlockInteraction(5);
    QCOMPARE(host.activeBlockIndex(), 5);
    QCOMPARE(host.activeBlockCursorRevision(), 1);
    QCOMPARE(host.selectionClearRevision(), 3);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 5);
    QCOMPARE(activeBlockIndexSpy.count(), 1);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 1);
    QCOMPARE(selectionClearRevisionSpy.count(), 3);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 2);

    host.noteActiveBlockInteraction(5);
    QCOMPARE(host.activeBlockIndex(), 5);
    QCOMPARE(host.activeBlockCursorRevision(), 2);
    QCOMPARE(host.selectionClearRevision(), 4);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 5);
    QCOMPARE(activeBlockIndexSpy.count(), 1);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 2);
    QCOMPARE(selectionClearRevisionSpy.count(), 4);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 2);

    host.noteActiveBlockCursorInteraction(5);
    QCOMPARE(host.activeBlockIndex(), 5);
    QCOMPARE(host.activeBlockCursorRevision(), 3);
    QCOMPARE(host.selectionClearRevision(), 4);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 5);
    QCOMPARE(activeBlockIndexSpy.count(), 1);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 3);
    QCOMPARE(selectionClearRevisionSpy.count(), 4);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 2);

    host.noteActiveBlockCursorInteraction(6);
    QCOMPARE(host.activeBlockIndex(), 6);
    QCOMPARE(host.activeBlockCursorRevision(), 4);
    QCOMPARE(host.selectionClearRevision(), 5);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 6);
    QCOMPARE(activeBlockIndexSpy.count(), 2);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 4);
    QCOMPARE(selectionClearRevisionSpy.count(), 5);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 3);

    host.requestSelectionClear(-1);
    QCOMPARE(host.selectionClearRevision(), 6);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), -1);
    QCOMPARE(selectionClearRevisionSpy.count(), 6);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 4);
}
