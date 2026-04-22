#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorSessionController_preservesLocalEditorAuthorityAgainstSameNoteModelSync()
{
    ContentsEditorSessionController controller;

    controller.setEditorBoundNoteId(QStringLiteral("note-1"));
    controller.setEditorText(QStringLiteral("editor-owned text"));
    controller.setLocalEditorAuthority(true);
    controller.setLastLocalEditTimestampMs(0);
    controller.setTypingIdleThresholdMs(1000);
    controller.setPendingBodySave(false);

    QVERIFY(!controller.isTypingSessionActive());
    QVERIFY(!controller.requestSyncEditorTextFromSelection(
        QStringLiteral("note-1"),
        QStringLiteral("raw-owned text"),
        QStringLiteral("note-1")));
    QCOMPARE(controller.editorBoundNoteId(), QStringLiteral("note-1"));
    QCOMPARE(controller.editorText(), QStringLiteral("editor-owned text"));
    QVERIFY(controller.localEditorAuthority());

    controller.setLocalEditorAuthority(false);
    QVERIFY(controller.requestSyncEditorTextFromSelection(
        QStringLiteral("note-1"),
        QStringLiteral("raw-owned text"),
        QStringLiteral("note-1")));
    QCOMPARE(controller.editorText(), QStringLiteral("raw-owned text"));
}

void WhatSonCppRegressionTests::editorSessionController_rebindsWhenSameNoteIdUsesDifferentPackagePath()
{
    ContentsEditorSessionController controller;

    controller.setEditorBoundNoteId(QStringLiteral("note-1"));
    controller.setEditorBoundNoteDirectoryPath(QStringLiteral("/tmp/wsnote-a"));
    controller.setEditorText(QStringLiteral("editor-owned text"));
    controller.setLocalEditorAuthority(true);
    controller.setLastLocalEditTimestampMs(0);
    controller.setTypingIdleThresholdMs(1000);
    controller.setPendingBodySave(false);

    QVERIFY(controller.requestSyncEditorTextFromSelection(
        QStringLiteral("note-1"),
        QStringLiteral("raw-owned text"),
        QStringLiteral("note-1"),
        QStringLiteral("/tmp/wsnote-b")));
    QCOMPARE(controller.editorBoundNoteId(), QStringLiteral("note-1"));
    QCOMPARE(controller.editorBoundNoteDirectoryPath(), QStringLiteral("/tmp/wsnote-b"));
    QCOMPARE(controller.editorText(), QStringLiteral("raw-owned text"));
    QVERIFY(!controller.localEditorAuthority());
}
