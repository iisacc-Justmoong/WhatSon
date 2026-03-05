#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"

#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class DetailPanelViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposeFileInfoState();
    void detailContentViewModels_mustMapEachStateToDedicatedViewModel();
    void requestStateChange_mustUpdateActiveStateAndToolbarSelection();
    void requestStateChange_invalidValue_mustBeIgnored();
};

void DetailPanelViewModelTest::defaults_mustExposeFileInfoState()
{
    DetailPanelViewModel viewModel;

    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::FileInfo));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("fileInfo"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    const QStringList expectedIconNames = {
        QStringLiteral("generalprojectStructure"),
        QStringLiteral("chartBar"),
        QStringLiteral("dataFile"),
        QStringLiteral("generalhistory"),
        QStringLiteral("cwmPermissionView"),
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

    QObject* fileInfoVm = viewModel.fileInfoViewModel();
    QObject* fileStatVm = viewModel.fileStatViewModel();
    QObject* fileFormatVm = viewModel.fileFormatViewModel();
    QObject* fileHistoryVm = viewModel.fileHistoryViewModel();
    QObject* appearanceVm = viewModel.appearanceViewModel();
    QObject* helpVm = viewModel.helpViewModel();

    QVERIFY(fileInfoVm != nullptr);
    QVERIFY(fileStatVm != nullptr);
    QVERIFY(fileFormatVm != nullptr);
    QVERIFY(fileHistoryVm != nullptr);
    QVERIFY(appearanceVm != nullptr);
    QVERIFY(helpVm != nullptr);

    QCOMPARE(fileInfoVm->property("stateName").toString(), QStringLiteral("fileInfo"));
    QCOMPARE(fileStatVm->property("stateName").toString(), QStringLiteral("fileStat"));
    QCOMPARE(fileFormatVm->property("stateName").toString(), QStringLiteral("fileFormat"));
    QCOMPARE(fileHistoryVm->property("stateName").toString(), QStringLiteral("fileHistory"));
    QCOMPARE(appearanceVm->property("stateName").toString(), QStringLiteral("appearance"));
    QCOMPARE(helpVm->property("stateName").toString(), QStringLiteral("help"));

    QCOMPARE(viewModel.activeContentViewModel(), fileInfoVm);
    QCOMPARE(fileInfoVm->property("active").toBool(), true);
    QCOMPARE(appearanceVm->property("active").toBool(), false);

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Appearance));

    QCOMPARE(viewModel.activeContentViewModel(), appearanceVm);
    QCOMPARE(fileInfoVm->property("active").toBool(), false);
    QCOMPARE(appearanceVm->property("active").toBool(), true);
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

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Appearance));

    QCOMPARE(activeStateSpy.count(), 1);
    QCOMPARE(toolbarItemsSpy.count(), 1);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Appearance));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("appearance"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    for (int index = 0; index < toolbarItems.size(); ++index)
    {
        const QVariantMap item = toolbarItems.at(index).toMap();
        QCOMPARE(item.value(QStringLiteral("selected")).toBool(), index == 4);
    }
}

void DetailPanelViewModelTest::requestStateChange_invalidValue_mustBeIgnored()
{
    DetailPanelViewModel viewModel;

    QSignalSpy activeStateSpy(&viewModel, &DetailPanelViewModel::activeStateChanged);
    QSignalSpy toolbarItemsSpy(&viewModel, &DetailPanelViewModel::toolbarItemsChanged);

    viewModel.requestStateChange(999);

    QCOMPARE(activeStateSpy.count(), 0);
    QCOMPARE(toolbarItemsSpy.count(), 0);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::FileInfo));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("fileInfo"));
}

QTEST_APPLESS_MAIN(DetailPanelViewModelTest)

#include "test_detail_panel_viewmodel.moc"
