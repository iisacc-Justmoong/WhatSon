#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorRawPushController_pushesOnIdleModifiedCountAndNoteDeparture()
{
    ensureCoreApplication();

    WhatSonEditorRawPushController controller;
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
