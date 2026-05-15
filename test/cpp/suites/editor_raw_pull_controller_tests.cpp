#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorRawPullController_requestsNoteEntryAndOpenPulls()
{
    ensureCoreApplication();

    WhatSonEditorRawPullController controller;
    QStringList pulledNotes;
    QStringList pulledPaths;
    QStringList pulledReasons;
    quint64 nextSequence = 40;
    controller.setRawPullCallback(
        [&pulledNotes, &pulledPaths, &pulledReasons, &nextSequence](
            const QString& noteId,
            const QString& noteDirectoryPath,
            const QString& reason,
            QString*) -> quint64
        {
            pulledNotes.push_back(noteId);
            pulledPaths.push_back(noteDirectoryPath);
            pulledReasons.push_back(reason);
            return ++nextSequence;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPullController::rawPullFinished);

    QCOMPARE(
        controller.requestNoteEntryPull(
            QStringLiteral("  entry-note  "),
            QStringLiteral("/tmp/../tmp/entry-note.wsnote")),
        quint64(41));
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(pulledNotes.constLast(), QStringLiteral("entry-note"));
    QCOMPARE(pulledPaths.constLast(), QStringLiteral("/tmp/entry-note.wsnote"));
    QCOMPARE(pulledReasons.constLast(), QStringLiteral("note-entry"));

    QCOMPARE(
        controller.requestNoteOpenPull(
            QStringLiteral("open-note"),
            QStringLiteral("/tmp/open-note.wsnote")),
        quint64(42));
    QCOMPARE(finishedSpy.count(), 2);
    QCOMPARE(pulledReasons.constLast(), QStringLiteral("note-open"));

    QCOMPARE(controller.requestNoteOpenPull(QString(), QStringLiteral("/tmp/missing.wsnote")), quint64(0));
    QCOMPARE(finishedSpy.count(), 2);
}

void WhatSonCppRegressionTests::editorRawPullController_pullsActiveNoteEveryIdleInterval()
{
    ensureCoreApplication();

    WhatSonEditorRawPullController controller;
    QCOMPARE(controller.idlePullIntervalMs(), 5000);
    controller.setIdlePullIntervalMs(25);
    QCOMPARE(controller.idlePullIntervalMs(), 25);

    QStringList pulledReasons;
    QStringList pulledPaths;
    quint64 nextSequence = 90;
    controller.setRawPullCallback(
        [&pulledReasons, &pulledPaths, &nextSequence](
            const QString&,
            const QString& noteDirectoryPath,
            const QString& reason,
            QString*) -> quint64
        {
            pulledReasons.push_back(reason);
            pulledPaths.push_back(noteDirectoryPath);
            return ++nextSequence;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPullController::rawPullFinished);
    controller.setActiveNoteForIdlePull(
        QStringLiteral("  idle-note  "),
        QStringLiteral("/tmp/../tmp/idle-note.wsnote"));

    QTRY_VERIFY_WITH_TIMEOUT(finishedSpy.count() >= 2, 300);
    QCOMPARE(pulledReasons.at(0), QStringLiteral("idle"));
    QCOMPARE(pulledReasons.at(1), QStringLiteral("idle"));
    QCOMPARE(pulledPaths.at(0), QStringLiteral("/tmp/idle-note.wsnote"));

    controller.recordUserActivity();
    const int countAfterActivity = finishedSpy.count();
    QTest::qWait(10);
    QCOMPARE(finishedSpy.count(), countAfterActivity);
    QTRY_VERIFY_WITH_TIMEOUT(finishedSpy.count() > countAfterActivity, 150);

    controller.clearActiveNoteForIdlePull();
    const int countAfterClear = finishedSpy.count();
    QTest::qWait(60);
    QCOMPARE(finishedSpy.count(), countAfterClear);
}
