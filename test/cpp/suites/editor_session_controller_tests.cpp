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

void WhatSonCppRegressionTests::editorSessionController_commitsRawMutationsThroughSessionAuthority()
{
    ContentsEditorSessionController controller;
    QSignalSpy editorTextChangedSpy(&controller, &ContentsEditorSessionController::editorTextChanged);
    QSignalSpy localAuthorityChangedSpy(&controller, &ContentsEditorSessionController::localEditorAuthorityChanged);

    controller.setEditorBoundNoteId(QStringLiteral("note-1"));
    QVERIFY(controller.commitRawEditorTextMutation(QStringLiteral("<task></task>")));
    QCOMPARE(controller.editorText(), QStringLiteral("<task> </task>"));
    QVERIFY(controller.localEditorAuthority());
    QVERIFY(controller.pendingBodySave());
    QVERIFY(editorTextChangedSpy.count() >= 1);
    QCOMPARE(localAuthorityChangedSpy.count(), 1);

    editorTextChangedSpy.clear();
    localAuthorityChangedSpy.clear();
    QVERIFY(!controller.commitRawEditorTextMutation(QStringLiteral("<task> </task>")));
    QCOMPARE(controller.editorText(), QStringLiteral("<task> </task>"));
    QCOMPARE(editorTextChangedSpy.count(), 0);
    QCOMPARE(localAuthorityChangedSpy.count(), 0);
}

void WhatSonCppRegressionTests::editorSessionBoundary_usesCppControllerWithoutQmlWrapper()
{
    const QString sessionControllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.hpp"));
    const QString sessionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSessionController.cpp"));
    const QString sessionQmlWrapper = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/session/ContentsEditorSession.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString sessionReadme = readUtf8SourceFile(
        QStringLiteral("docs/src/app/models/editor/session/README.md"));
    QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    repositoryRoot.cdUp();
    const QDir sessionDirectory(repositoryRoot.filePath(QStringLiteral("src/app/models/editor/session")));

    QVERIFY(!sessionControllerHeader.isEmpty());
    QVERIFY(!sessionControllerSource.isEmpty());
    QVERIFY(sessionQmlWrapper.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!sessionReadme.isEmpty());
    QVERIFY(sessionDirectory.entryList(QStringList{QStringLiteral("*.qml")}, QDir::Files).isEmpty());

    QVERIFY(sessionControllerHeader.contains(QStringLiteral("class ContentsEditorSessionController : public QObject")));
    QVERIFY(sessionControllerSource.contains(QStringLiteral("queueCurrentEditorTextForPersistence")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsEditorSessionController {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("ContentsEditorSession {")));
    QVERIFY(sessionReadme.contains(QStringLiteral("must not contain QML wrappers")));
}
