#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"

#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class DetailPanelViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposePropertiesState();
    void detailContentViewModels_mustMapEachStateToDedicatedViewModel();
    void requestStateChange_mustUpdateActiveStateAndToolbarSelection();
    void requestStateChange_invalidValue_mustBeIgnored();
};

void DetailPanelViewModelTest::defaults_mustExposePropertiesState()
{
    DetailPanelViewModel viewModel;

    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Properties));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("properties"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    const QStringList expectedIconNames = {
        QStringLiteral("config"),
        QStringLiteral("chartBar"),
        QStringLiteral("generaladd"),
        QStringLiteral("toolwindowdependencies"),
        QStringLiteral("toolWindowClock"),
        QStringLiteral("featureAnswer")
    };

    for (int index = 0; index < toolbarItems.size(); ++index)
    {
        const QVariantMap item = toolbarItems.at(index).toMap();
        QVERIFY(item.contains(QStringLiteral("iconName")));
        QVERIFY(item.contains(QStringLiteral("stateValue")));
        QVERIFY(item.contains(QStringLiteral("selected")));
        QCOMPARE(item.value(QStringLiteral("iconName")).toString(), expectedIconNames.at(index));
        QCOMPARE(item.value(QStringLiteral("selected")).toBool(), index == 0);
    }
}

void DetailPanelViewModelTest::detailContentViewModels_mustMapEachStateToDedicatedViewModel()
{
    DetailPanelViewModel viewModel;

    QObject* propertiesVm = viewModel.propertiesViewModel();
    QObject* fileStatVm = viewModel.fileStatViewModel();
    QObject* insertVm = viewModel.insertViewModel();
    QObject* fileHistoryVm = viewModel.fileHistoryViewModel();
    QObject* layerVm = viewModel.layerViewModel();
    QObject* helpVm = viewModel.helpViewModel();

    QVERIFY(propertiesVm != nullptr);
    QVERIFY(fileStatVm != nullptr);
    QVERIFY(insertVm != nullptr);
    QVERIFY(fileHistoryVm != nullptr);
    QVERIFY(layerVm != nullptr);
    QVERIFY(helpVm != nullptr);

    QCOMPARE(propertiesVm->property("stateName").toString(), QStringLiteral("properties"));
    QCOMPARE(fileStatVm->property("stateName").toString(), QStringLiteral("fileStat"));
    QCOMPARE(insertVm->property("stateName").toString(), QStringLiteral("insert"));
    QCOMPARE(fileHistoryVm->property("stateName").toString(), QStringLiteral("fileHistory"));
    QCOMPARE(layerVm->property("stateName").toString(), QStringLiteral("layer"));
    QCOMPARE(helpVm->property("stateName").toString(), QStringLiteral("help"));

    QCOMPARE(viewModel.activeContentViewModel(), propertiesVm);
    QCOMPARE(propertiesVm->property("active").toBool(), true);
    QCOMPARE(layerVm->property("active").toBool(), false);

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));

    QCOMPARE(viewModel.activeContentViewModel(), layerVm);
    QCOMPARE(propertiesVm->property("active").toBool(), false);
    QCOMPARE(layerVm->property("active").toBool(), true);
    QCOMPARE(
        viewModel.contentViewModelForState(static_cast<int>(DetailPanelViewModel::DetailContentState::Help)),
        helpVm);
    QCOMPARE(viewModel.contentViewModelForState(-1), nullptr);
}

void DetailPanelViewModelTest::requestStateChange_mustUpdateActiveStateAndToolbarSelection()
{
    DetailPanelViewModel viewModel;

    QSignalSpy activeStateSpy(&viewModel, &DetailPanelViewModel::activeStateChanged);
    QSignalSpy toolbarItemsSpy(&viewModel, &DetailPanelViewModel::toolbarItemsChanged);

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));

    QCOMPARE(activeStateSpy.count(), 1);
    QCOMPARE(toolbarItemsSpy.count(), 1);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("layer"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    for (int index = 0; index < toolbarItems.size(); ++index)
    {
        const QVariantMap item = toolbarItems.at(index).toMap();
        QCOMPARE(item.value(QStringLiteral("selected")).toBool(), index == 3);
    }
    QCOMPARE(toolbarItems.at(3).toMap().value(QStringLiteral("iconName")).toString(),
             QStringLiteral("toolwindowdependencies"));
    QCOMPARE(toolbarItems.at(4).toMap().value(QStringLiteral("iconName")).toString(),
             QStringLiteral("toolWindowClock"));
}

void DetailPanelViewModelTest::requestStateChange_invalidValue_mustBeIgnored()
{
    DetailPanelViewModel viewModel;

    QSignalSpy activeStateSpy(&viewModel, &DetailPanelViewModel::activeStateChanged);
    QSignalSpy toolbarItemsSpy(&viewModel, &DetailPanelViewModel::toolbarItemsChanged);

    viewModel.requestStateChange(999);

    QCOMPARE(activeStateSpy.count(), 0);
    QCOMPARE(toolbarItemsSpy.count(), 0);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Properties));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("properties"));
}

QTEST_APPLESS_MAIN(DetailPanelViewModelTest)

#include "test_detail_panel_viewmodel.moc"
