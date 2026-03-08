#include "viewmodel/navigationbar/EditorViewModeViewModel.hpp"

#include <QSignalSpy>
#include <QtTest>

class EditorViewModeViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposePlainCase();
    void viewModeViewModels_mustMapEachEnumState();
    void requestViewModeChange_mustSwitchActiveViewMode();
    void requestNextViewMode_mustWrapAcrossEnum();
    void requestViewModeChange_invalidValue_mustBeIgnored();
};

void EditorViewModeViewModelTest::defaults_mustExposePlainCase()
{
    EditorViewModeViewModel viewModel;

    QCOMPARE(viewModel.activeViewMode(), static_cast<int>(EditorViewModeViewModel::EditorView::Plain));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QCOMPARE(viewModel.activeViewModeViewModel(), viewModel.plainViewModeViewModel());
    QCOMPARE(viewModel.plainViewModeViewModel()->property("active").toBool(), true);
    QCOMPARE(viewModel.plainViewModeViewModel()->property("editorViewName").toString(), QStringLiteral("Plain"));
    QCOMPARE(viewModel.pageViewModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.printViewModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.webViewModeViewModel()->property("active").toBool(), false);
    QCOMPARE(viewModel.presentationViewModeViewModel()->property("active").toBool(), false);
}

void EditorViewModeViewModelTest::viewModeViewModels_mustMapEachEnumState()
{
    EditorViewModeViewModel viewModel;

    QObject* plainVm = viewModel.plainViewModeViewModel();
    QObject* pageVm = viewModel.pageViewModeViewModel();
    QObject* printVm = viewModel.printViewModeViewModel();
    QObject* webVm = viewModel.webViewModeViewModel();
    QObject* presentationVm = viewModel.presentationViewModeViewModel();

    QVERIFY(plainVm != nullptr);
    QVERIFY(pageVm != nullptr);
    QVERIFY(printVm != nullptr);
    QVERIFY(webVm != nullptr);
    QVERIFY(presentationVm != nullptr);

    QCOMPARE(plainVm->property("editorViewName").toString(), QStringLiteral("Plain"));
    QCOMPARE(pageVm->property("editorViewName").toString(), QStringLiteral("Page"));
    QCOMPARE(printVm->property("editorViewName").toString(), QStringLiteral("Print"));
    QCOMPARE(webVm->property("editorViewName").toString(), QStringLiteral("Web"));
    QCOMPARE(presentationVm->property("editorViewName").toString(), QStringLiteral("Presentation"));

    QCOMPARE(
        viewModel.viewModeViewModelForState(static_cast<int>(EditorViewModeViewModel::EditorView::Plain)),
        plainVm);
    QCOMPARE(
        viewModel.viewModeViewModelForState(static_cast<int>(EditorViewModeViewModel::EditorView::Page)),
        pageVm);
    QCOMPARE(
        viewModel.viewModeViewModelForState(static_cast<int>(EditorViewModeViewModel::EditorView::Print)),
        printVm);
    QCOMPARE(
        viewModel.viewModeViewModelForState(static_cast<int>(EditorViewModeViewModel::EditorView::Web)),
        webVm);
    QCOMPARE(
        viewModel.viewModeViewModelForState(static_cast<int>(EditorViewModeViewModel::EditorView::Presentation)),
        presentationVm);
    QCOMPARE(viewModel.viewModeViewModelForState(-1), nullptr);
}

void EditorViewModeViewModelTest::requestViewModeChange_mustSwitchActiveViewMode()
{
    EditorViewModeViewModel viewModel;

    QSignalSpy activeViewModeSpy(&viewModel, &EditorViewModeViewModel::activeViewModeChanged);

    viewModel.requestViewModeChange(static_cast<int>(EditorViewModeViewModel::EditorView::Presentation));

    QCOMPARE(activeViewModeSpy.count(), 1);
    QCOMPARE(viewModel.activeViewMode(), static_cast<int>(EditorViewModeViewModel::EditorView::Presentation));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Presentation"));
    QCOMPARE(viewModel.activeViewModeViewModel(), viewModel.presentationViewModeViewModel());
    QCOMPARE(viewModel.presentationViewModeViewModel()->property("active").toBool(), true);
    QCOMPARE(viewModel.plainViewModeViewModel()->property("active").toBool(), false);
}

void EditorViewModeViewModelTest::requestNextViewMode_mustWrapAcrossEnum()
{
    EditorViewModeViewModel viewModel;

    viewModel.setActiveViewMode(static_cast<int>(EditorViewModeViewModel::EditorView::Web));
    viewModel.requestNextViewMode();

    QCOMPARE(viewModel.activeViewMode(), static_cast<int>(EditorViewModeViewModel::EditorView::Presentation));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Presentation"));
    QCOMPARE(viewModel.activeViewModeViewModel(), viewModel.presentationViewModeViewModel());

    viewModel.requestNextViewMode();

    QCOMPARE(viewModel.activeViewMode(), static_cast<int>(EditorViewModeViewModel::EditorView::Plain));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QCOMPARE(viewModel.activeViewModeViewModel(), viewModel.plainViewModeViewModel());
}

void EditorViewModeViewModelTest::requestViewModeChange_invalidValue_mustBeIgnored()
{
    EditorViewModeViewModel viewModel;

    QSignalSpy activeViewModeSpy(&viewModel, &EditorViewModeViewModel::activeViewModeChanged);

    viewModel.requestViewModeChange(999);

    QCOMPARE(activeViewModeSpy.count(), 0);
    QCOMPARE(viewModel.activeViewMode(), static_cast<int>(EditorViewModeViewModel::EditorView::Plain));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
}

QTEST_APPLESS_MAIN(EditorViewModeViewModelTest)

#include "test_editor_view_mode_viewmodel.moc"
