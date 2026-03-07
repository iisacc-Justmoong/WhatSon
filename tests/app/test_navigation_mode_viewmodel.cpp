#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"

#include <QSignalSpy>
#include <QtTest>

class NavigationModeViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposeControlCase();
    void modeViewModels_mustMapEachEnumState();
    void requestModeChange_mustSwitchActiveMode();
    void requestNextMode_mustWrapAcrossEnum();
    void requestModeChange_invalidValue_mustBeIgnored();
};

void NavigationModeViewModelTest::defaults_mustExposeControlCase()
{
    NavigationModeViewModel viewModel;

    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::Control));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Control"));
    QCOMPARE(viewModel.activeModeViewModel(), viewModel.controlModeViewModel());
    QCOMPARE(viewModel.controlModeViewModel()->property("active").toBool(), true);
    QCOMPARE(viewModel.controlModeViewModel()->property("modeName").toString(), QStringLiteral("Control"));
    QCOMPARE(viewModel.viewModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.editModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.presentationModeViewModel()->property("active").toBool(), false);
}

void NavigationModeViewModelTest::modeViewModels_mustMapEachEnumState()
{
    NavigationModeViewModel viewModel;

    QObject* viewModeVm = viewModel.viewModeViewModel();
    QObject* editModeVm = viewModel.editModeViewModel();
    QObject* controlModeVm = viewModel.controlModeViewModel();
    QObject* presentationModeVm = viewModel.presentationModeViewModel();

    QVERIFY(viewModeVm != nullptr);
    QVERIFY(editModeVm != nullptr);
    QVERIFY(controlModeVm != nullptr);
    QVERIFY(presentationModeVm != nullptr);

    QCOMPARE(viewModeVm->property("modeName").toString(), QStringLiteral("View"));
    QCOMPARE(editModeVm->property("modeName").toString(), QStringLiteral("Edit"));
    QCOMPARE(controlModeVm->property("modeName").toString(), QStringLiteral("Control"));
    QCOMPARE(presentationModeVm->property("modeName").toString(), QStringLiteral("Presentation"));

    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::View)),
        viewModeVm);
    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::Edit)),
        editModeVm);
    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::Control)),
        controlModeVm);
    QCOMPARE(
        viewModel.modeViewModelForState(static_cast<int>(NavigationModeViewModel::NavigationMode::Presentation)),
        presentationModeVm);
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

    viewModel.setActiveMode(static_cast<int>(NavigationModeViewModel::NavigationMode::Presentation));
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
    QCOMPARE(viewModel.activeMode(), static_cast<int>(NavigationModeViewModel::NavigationMode::Control));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Control"));
}

QTEST_APPLESS_MAIN(NavigationModeViewModelTest)

#include "test_navigation_mode_viewmodel.moc"
