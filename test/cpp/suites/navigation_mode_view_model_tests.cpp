#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::navigationModeViewModel_cyclesActiveSections()
{
    NavigationModeViewModel viewModel;
    auto* viewSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.viewModeViewModel());
    auto* editSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.editModeViewModel());
    auto* controlSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.controlModeViewModel());

    QVERIFY(viewSection != nullptr);
    QVERIFY(editSection != nullptr);
    QVERIFY(controlSection != nullptr);

    QSignalSpy activeModeChangedSpy(&viewModel, &NavigationModeViewModel::activeModeChanged);

    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));
    QCOMPARE(
        viewModel.activeModeViewModel(),
        static_cast<QObject*>(viewSection));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QVERIFY(viewModel.modeViewModelForState(999) == nullptr);

    viewModel.requestNextMode();
    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Edit));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Edit"));
    QCOMPARE(
        viewModel.activeModeViewModel(),
        static_cast<QObject*>(editSection));
    QVERIFY(!viewSection->active());
    QVERIFY(editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 1);

    viewModel.requestModeChange(WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Control));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Control"));
    QVERIFY(!viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 2);

    viewModel.setActiveMode(999);
    QCOMPARE(activeModeChangedSpy.count(), 2);

    viewModel.requestNextMode();
    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 3);
}
