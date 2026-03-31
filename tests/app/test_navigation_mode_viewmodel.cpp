#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"

#include <QSignalSpy>
#include <QtTest>

class NavigationModeViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposeViewCase();
    void modeViewModels_mustMapEachEnumState();
    void requestModeChange_mustSwitchActiveMode();
    void requestNextMode_mustWrapAcrossEnum();
    void requestModeChange_invalidValue_mustBeIgnored();
};

void NavigationModeViewModelTest::defaults_mustExposeViewCase()
{
    NavigationModeViewModel viewModel;

    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));
    QCOMPARE(viewModel.activeModeViewModel(), viewModel.viewModeViewModel());
    QCOMPARE(viewModel.controlModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.controlModeViewModel()->property("modeName").toString(), QStringLiteral("Control"));
    QCOMPARE(viewModel.viewModeViewModel()->property("active").toBool(), true);
    QCOMPARE(viewModel.editModeViewModel()->property("active").toBool(), false);
}

void NavigationModeViewModelTest::modeViewModels_mustMapEachEnumState()
{
    NavigationModeViewModel viewModel;

    QObject* viewModeVm = viewModel.viewModeViewModel();
    QObject* editModeVm = viewModel.editModeViewModel();
    QObject* controlModeVm = viewModel.controlModeViewModel();

    QVERIFY(viewModeVm != nullptr);
    QVERIFY(editModeVm != nullptr);
    QVERIFY(controlModeVm != nullptr);

    QCOMPARE(viewModeVm->property("modeName").toString(), QStringLiteral("View"));
    QCOMPARE(editModeVm->property("modeName").toString(), QStringLiteral("Edit"));
    QCOMPARE(controlModeVm->property("modeName").toString(), QStringLiteral("Control"));

    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::View)),
        viewModeVm);
    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::Edit)),
        editModeVm);
    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::Control)),
        controlModeVm);
    QCOMPARE(viewModel.modeViewModelForState(3), nullptr);
    QCOMPARE(viewModel.modeViewModelForState(-1), nullptr);
}

void NavigationModeViewModelTest::requestModeChange_mustSwitchActiveMode()
{
    NavigationModeViewModel viewModel;

    QSignalSpy activeModeSpy(&viewModel, &NavigationModeViewModel::activeModeChanged);

    viewModel.requestModeChange(static_cast<int>(NavigationModeViewModel::NavigationMode::Edit));

    QCOMPARE(activeModeSpy.count(), 1);
    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::Edit));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Edit"));
    QCOMPARE(viewModel.activeModeViewModel(), viewModel.editModeViewModel());
    QCOMPARE(viewModel.editModeViewModel()->property("active").toBool(), true);
    QCOMPARE(viewModel.controlModeViewModel()->property("active").toBool(), false);
}

void NavigationModeViewModelTest::requestNextMode_mustWrapAcrossEnum()
{
    NavigationModeViewModel viewModel;

    viewModel.setActiveMode(static_cast<int>(NavigationModeViewModel::NavigationMode::Control));
    viewModel.requestNextMode();

    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));
    QCOMPARE(viewModel.activeModeViewModel(), viewModel.viewModeViewModel());
}

void NavigationModeViewModelTest::requestModeChange_invalidValue_mustBeIgnored()
{
    NavigationModeViewModel viewModel;

    QSignalSpy activeModeSpy(&viewModel, &NavigationModeViewModel::activeModeChanged);

    viewModel.requestModeChange(999);

    QCOMPARE(activeModeSpy.count(), 0);
    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));

    viewModel.requestModeChange(3);

    QCOMPARE(activeModeSpy.count(), 0);
    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));
}

QTEST_APPLESS_MAIN(NavigationModeViewModelTest)

#include "test_navigation_mode_viewmodel.moc"
