#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorRawPushController_pushesOnIdleModifiedCountAndNoteDeparture()
{
    ensureCoreApplication();

    WhatSonEditorRawPushController controller;
    QVERIFY(controller.idleIntervalMs() > 0);
    QStringList pushedReasons;
    QStringList pushedTexts;
    controller.setRawPushCallback(
        [&pushedReasons, &pushedTexts](
            const QString&,
            const QString& editorDocumentText,
            const bool hasEditorDocumentText,
            const QString& reason,
            QString*) -> bool
        {
            pushedReasons.push_back(reason);
            pushedTexts.push_back(hasEditorDocumentText ? editorDocumentText : QStringLiteral("<file>"));
            return true;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPushController::rawPushFinished);

    controller.requestIdlePush(
        QStringLiteral("/tmp/surface.wsnsource"),
        QStringLiteral("idle surface"));
    QTest::qWait(qMin(50, controller.idleIntervalMs() / 2));
    QCOMPARE(finishedSpy.count(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("idle"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("idle surface"));

    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        1,
        QStringLiteral("modified one"));
    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 2, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("modified-count"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("modified one"));

    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        1,
        QStringLiteral("duplicate modified count"));
    QTest::qWait(20);
    QCOMPARE(finishedSpy.count(), 2);

    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        2,
        QStringLiteral("modified two"));
    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 3, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("modified-count"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("modified two"));

    controller.requestIdlePush(
        QStringLiteral("/tmp/surface.wsnsource"),
        QStringLiteral("pending before departure"));
    QVERIFY(controller.pushBeforeNoteDeparture(QStringLiteral("/tmp/surface.wsnsource")));
    QCOMPARE(finishedSpy.count(), 4);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("note-departure"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("pending before departure"));
}

void WhatSonCppRegressionTests::editorRawPushController_keepsPendingModifiedCountWhenIdleSyncArrives()
{
    ensureCoreApplication();

    WhatSonEditorRawPushController controller;
    controller.setIdleIntervalMs(25);

    QStringList pushedReasons;
    QStringList pushedTexts;
    controller.setRawPushCallback(
        [&pushedReasons, &pushedTexts](
            const QString&,
            const QString& editorDocumentText,
            const bool hasEditorDocumentText,
            const QString& reason,
            QString*) -> bool
        {
            pushedReasons.push_back(reason);
            pushedTexts.push_back(hasEditorDocumentText ? editorDocumentText : QStringLiteral("<file>"));
            return true;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPushController::rawPushFinished);
    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        1,
        QStringLiteral("after first backspace"));
    controller.requestIdlePush(
        QStringLiteral("/tmp/surface.wsnsource"),
        QStringLiteral("stale sync snapshot"));

    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("modified-count"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("after first backspace"));
}

void WhatSonCppRegressionTests::editorRawPushController_refreshesPendingModifiedPayloadForSameRevision()
{
    ensureCoreApplication();

    WhatSonEditorRawPushController controller;
    controller.setIdleIntervalMs(25);

    QStringList pushedReasons;
    QStringList pushedTexts;
    controller.setRawPushCallback(
        [&pushedReasons, &pushedTexts](
            const QString&,
            const QString& editorDocumentText,
            const bool hasEditorDocumentText,
            const QString& reason,
            QString*) -> bool
        {
            pushedReasons.push_back(reason);
            pushedTexts.push_back(hasEditorDocumentText ? editorDocumentText : QStringLiteral("<file>"));
            return true;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPushController::rawPushFinished);
    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        5,
        QStringLiteral("면접 대"));
    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        5,
        QStringLiteral("면접 대본"));

    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("modified-count"));
    QCOMPARE(pushedTexts.constLast(), QStringLiteral("면접 대본"));
}

void WhatSonCppRegressionTests::editorRawPushController_discardsPendingPushForAuthoritativeWrite()
{
    ensureCoreApplication();

    WhatSonEditorRawPushController controller;
    controller.setIdleIntervalMs(25);

    QStringList pushedReasons;
    controller.setRawPushCallback(
        [&pushedReasons](
            const QString&,
            const QString&,
            const bool,
            const QString& reason,
            QString*) -> bool
        {
            pushedReasons.push_back(reason);
            return true;
        });

    QSignalSpy finishedSpy(&controller, &WhatSonEditorRawPushController::rawPushFinished);
    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        1,
        QStringLiteral("stale pre-paste surface"));
    QVERIFY(controller.discardPendingPushForFile(QStringLiteral("/tmp/surface.wsnsource")));
    QTest::qWait(50);
    QCOMPARE(finishedSpy.count(), 0);
    QVERIFY(pushedReasons.isEmpty());

    controller.requestModifiedCountPush(
        QStringLiteral("/tmp/surface.wsnsource"),
        1,
        QStringLiteral("post-discard edit"));
    QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 3000);
    QCOMPARE(pushedReasons.constLast(), QStringLiteral("modified-count"));
}
