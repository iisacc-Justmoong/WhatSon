#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::editorViewModeController_cyclesActiveSections()
{
    EditorViewModeController controller;
    auto* plainSection =
        qobject_cast<EditorViewSectionController*>(controller.plainViewModeController());
    auto* pageSection =
        qobject_cast<EditorViewSectionController*>(controller.pageViewModeController());
    auto* printSection =
        qobject_cast<EditorViewSectionController*>(controller.printViewModeController());
    auto* webSection =
        qobject_cast<EditorViewSectionController*>(controller.webViewModeController());
    auto* presentationSection =
        qobject_cast<EditorViewSectionController*>(controller.presentationViewModeController());

    QVERIFY(plainSection != nullptr);
    QVERIFY(pageSection != nullptr);
    QVERIFY(printSection != nullptr);
    QVERIFY(webSection != nullptr);
    QVERIFY(presentationSection != nullptr);

    QSignalSpy activeViewModeChangedSpy(&controller, &EditorViewModeController::activeViewModeChanged);

    QCOMPARE(
        controller.activeViewMode(),
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Plain));
    QCOMPARE(controller.activeViewModeName(), QStringLiteral("Plain"));
    QCOMPARE(
        controller.activeViewModeController(),
        static_cast<QObject*>(plainSection));
    QVERIFY(plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(!presentationSection->active());
    QVERIFY(controller.viewModeControllerForState(999) == nullptr);

    controller.requestNextViewMode();
    QCOMPARE(controller.activeViewModeName(), QStringLiteral("Page"));
    QVERIFY(!plainSection->active());
    QVERIFY(pageSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 1);

    controller.requestViewModeChange(
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Presentation));
    QCOMPARE(controller.activeViewModeName(), QStringLiteral("Presentation"));
    QVERIFY(!plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    controller.setActiveViewMode(-1);
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    controller.requestNextViewMode();
    QCOMPARE(controller.activeViewModeName(), QStringLiteral("Plain"));
    QVERIFY(plainSection->active());
    QVERIFY(!presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 3);
}
