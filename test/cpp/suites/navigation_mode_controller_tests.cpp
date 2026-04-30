#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::navigationModeController_cyclesActiveSections()
{
    NavigationModeController controller;
    auto* viewSection =
        qobject_cast<NavigationModeSectionController*>(controller.viewModeController());
    auto* editSection =
        qobject_cast<NavigationModeSectionController*>(controller.editModeController());
    auto* controlSection =
        qobject_cast<NavigationModeSectionController*>(controller.controlModeController());

    QVERIFY(viewSection != nullptr);
    QVERIFY(editSection != nullptr);
    QVERIFY(controlSection != nullptr);

    QSignalSpy activeModeChangedSpy(&controller, &NavigationModeController::activeModeChanged);

    QCOMPARE(
        controller.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QCOMPARE(controller.activeModeName(), QStringLiteral("View"));
    QCOMPARE(
        controller.activeModeController(),
        static_cast<QObject*>(viewSection));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QVERIFY(controller.modeControllerForState(999) == nullptr);

    controller.requestNextMode();
    QCOMPARE(
        controller.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Edit));
    QCOMPARE(controller.activeModeName(), QStringLiteral("Edit"));
    QCOMPARE(
        controller.activeModeController(),
        static_cast<QObject*>(editSection));
    QVERIFY(!viewSection->active());
    QVERIFY(editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 1);

    controller.requestModeChange(WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Control));
    QCOMPARE(controller.activeModeName(), QStringLiteral("Control"));
    QVERIFY(!viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 2);

    controller.setActiveMode(999);
    QCOMPARE(activeModeChangedSpy.count(), 2);

    controller.requestNextMode();
    QCOMPARE(
        controller.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 3);
}
