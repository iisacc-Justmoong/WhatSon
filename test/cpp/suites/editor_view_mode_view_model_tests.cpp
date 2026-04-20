#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorViewModeViewModel_cyclesActiveSections()
{
    EditorViewModeViewModel viewModel;
    auto* plainSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.plainViewModeViewModel());
    auto* pageSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.pageViewModeViewModel());
    auto* printSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.printViewModeViewModel());
    auto* webSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.webViewModeViewModel());
    auto* presentationSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.presentationViewModeViewModel());

    QVERIFY(plainSection != nullptr);
    QVERIFY(pageSection != nullptr);
    QVERIFY(printSection != nullptr);
    QVERIFY(webSection != nullptr);
    QVERIFY(presentationSection != nullptr);

    QSignalSpy activeViewModeChangedSpy(&viewModel, &EditorViewModeViewModel::activeViewModeChanged);

    QCOMPARE(
        viewModel.activeViewMode(),
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Plain));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QCOMPARE(
        viewModel.activeViewModeViewModel(),
        static_cast<QObject*>(plainSection));
    QVERIFY(plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(!presentationSection->active());
    QVERIFY(viewModel.viewModeViewModelForState(999) == nullptr);

    viewModel.requestNextViewMode();
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Page"));
    QVERIFY(!plainSection->active());
    QVERIFY(pageSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 1);

    viewModel.requestViewModeChange(
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Presentation));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Presentation"));
    QVERIFY(!plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    viewModel.setActiveViewMode(-1);
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    viewModel.requestNextViewMode();
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QVERIFY(plainSection->active());
    QVERIFY(!presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 3);
}
