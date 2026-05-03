#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsView_composesFigmaFrameFromLvrsParts()
{
    const QString contentsViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/contents/ContentsView.qml"));
    const QString gutterSource = readUtf8SourceFile(QStringLiteral("src/app/qml/contents/Gutter.qml"));
    const QString editorViewSource = readUtf8SourceFile(QStringLiteral("src/app/qml/contents/EditorView.qml"));
    const QString minimapSource = readUtf8SourceFile(QStringLiteral("src/app/qml/contents/Minimap.qml"));

    QVERIFY(!contentsViewSource.isEmpty());
    QVERIFY(!gutterSource.isEmpty());
    QVERIFY(!editorViewSource.isEmpty());
    QVERIFY(!minimapSource.isEmpty());
    QVERIFY(contentsViewSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("import WhatSon.App.Internal 1.0")));
    QVERIFY(gutterSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(editorViewSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(minimapSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("objectName: \"figma-155-4561-ContentsView\"")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("LV.HStack {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("objectName: \"figma-155-5344-HStack\"")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("Gutter {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("EditorView {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("Minimap {")));
    const qsizetype hStackIndex = contentsViewSource.indexOf(QStringLiteral("LV.HStack {"));
    const qsizetype gutterIndex = contentsViewSource.indexOf(QStringLiteral("Gutter {"));
    const qsizetype editorIndex = contentsViewSource.indexOf(QStringLiteral("EditorView {"));
    const qsizetype minimapIndex = contentsViewSource.indexOf(QStringLiteral("Minimap {"));
    QVERIFY(hStackIndex >= 0);
    QVERIFY(gutterIndex > hStackIndex);
    QVERIFY(editorIndex > gutterIndex);
    QVERIFY(minimapIndex > editorIndex);
    QVERIFY(!contentsViewSource.contains(QStringLiteral("component Gutter:")));
    QVERIFY(!contentsViewSource.contains(QStringLiteral("component EditorView:")));
    QVERIFY(!contentsViewSource.contains(QStringLiteral("component Minimap:")));
    QVERIFY(gutterSource.contains(QStringLiteral("objectName: \"figma-155-5345-Gutter\"")));
    QVERIFY(editorViewSource.contains(QStringLiteral("objectName: \"figma-155-5352-EditorView\"")));
    QVERIFY(minimapSource.contains(QStringLiteral("objectName: \"figma-352-8626-Minimap\"")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("ContentsGutterLayoutMetrics {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("ContentsGutterLineNumberGeometry {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("ContentsGutterMarkerGeometry {")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics {")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int lineNumberColumnLeft")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int lineNumberColumnTextWidth")));
    QVERIFY(gutterSource.contains(QStringLiteral("property var lineNumberEntries")));
    QVERIFY(gutterSource.contains(QStringLiteral("width: gutter.lineNumberColumnTextWidth")));
    QVERIFY(gutterSource.contains(QStringLiteral("x: gutter.lineNumberColumnLeft")));
    QVERIFY(gutterSource.contains(QStringLiteral("model: gutter.lineNumberEntries")));
    QVERIFY(gutterSource.contains(QStringLiteral("y: Number(modelData.y)")));
    QVERIFY(!gutterSource.contains(QStringLiteral("Column {")));
    QVERIFY(gutterSource.contains(QStringLiteral("property var markerEntries")));
    QVERIFY(gutterSource.contains(QStringLiteral("model: gutter.markerEntries")));
    QVERIFY(gutterSource.contains(QStringLiteral("markerType === \"cursor\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("gutter.unsavedMarkerColor")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("Layout.preferredWidth: gutterLayoutMetrics.defaultGutterWidth")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("Layout.preferredWidth: minimapLayoutMetrics.defaultMinimapWidth")));
    QVERIFY(contentsViewSource.contains(
        QStringLiteral("property int minimapRowCount: defaultMinimapRowCount")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("property color surfaceColor: LV.Theme.panelBackground02")));
    QVERIFY(gutterSource.contains(QStringLiteral("property color cursorMarkerColor: LV.Theme.accentBlue")));
    QVERIFY(gutterSource.contains(QStringLiteral("property color unsavedMarkerColor: LV.Theme.warning")));
    QVERIFY(editorViewSource.contains(QStringLiteral("selectionColor: LV.Theme.primaryOverlay")));
    QVERIFY(minimapSource.contains(QStringLiteral("property int rowCount: LV.Theme.strokeThin")));
    const QList<QString> qmlSources = {
        contentsViewSource,
        gutterSource,
        editorViewSource,
        minimapSource,
    };
    for (const QString &qmlSource : qmlSources) {
        QVERIFY(!qmlSource.contains(QStringLiteral("scaleMetric(")));
        QVERIFY(!qmlSource.contains(QStringLiteral("Qt.rgba(")));
        QVERIFY(!qmlSource.contains(QStringLiteral("#")));
        const QRegularExpression hardcodedMetricAssignment(QStringLiteral(
            R"((?:^|\n)\s*(?:width|height|x|y|radius|anchors\.\w+Margin|Layout\.preferredWidth|implicitHeight|implicitWidth|font\.pixelSize|activeLineNumber|lineNumberCount|minimapRowCount|rowCount)\s*:\s*-?\d)"));
        const QRegularExpressionMatch match = hardcodedMetricAssignment.match(qmlSource);
        QVERIFY2(!match.hasMatch(),
                 qPrintable(QStringLiteral("Hardcoded contents-view metric token: %1").arg(match.captured())));
    }
}

void WhatSonCppRegressionTests::qmlContentsView_partsKeepEditorProjectionReadOnlyAndNativeInputSafe()
{
    const QString contentsViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/contents/ContentsView.qml"));
    const QString editorViewSource = readUtf8SourceFile(QStringLiteral("src/app/qml/contents/EditorView.qml"));

    QVERIFY(!contentsViewSource.isEmpty());
    QVERIFY(!editorViewSource.isEmpty());
    QVERIFY(editorViewSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(editorViewSource.contains(QStringLiteral("readOnly: true")));
    QVERIFY(editorViewSource.contains(QStringLiteral("selectByMouse: true")));
    QVERIFY(editorViewSource.contains(QStringLiteral("showRenderedOutput: false")));
    QVERIFY(editorViewSource.contains(QStringLiteral("textFormat: TextEdit.PlainText")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("signal viewHookRequested(string reason)")));
    QVERIFY(editorViewSource.contains(QStringLiteral("signal viewHookRequested(string reason)")));
    QVERIFY(contentsViewSource.contains(QStringLiteral("function requestViewHook(reason)")));
    QVERIFY(editorViewSource.contains(QStringLiteral("function requestViewHook(reason)")));
    QVERIFY(!editorViewSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!editorViewSource.contains(QStringLiteral("Keys.onReleased")));
    QVERIFY(!editorViewSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!editorViewSource.contains(QStringLiteral("InputMethod.")));
}

void WhatSonCppRegressionTests::qmlLvrsTokens_replaceDirectHardcodedVisualTokensOutsideContents()
{
    const QList<QString> tokenizedFiles = {
        QStringLiteral("src/app/qml/Main.qml"),
        QStringLiteral("src/app/qml/view/calendar/AgendaPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/CalendarEventCell.qml"),
        QStringLiteral("src/app/qml/view/calendar/CalendarTodayControl.qml"),
        QStringLiteral("src/app/qml/view/calendar/DayCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarDayCell.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarGridSurface.qml"),
        QStringLiteral("src/app/qml/view/calendar/MonthCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/WeekCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/calendar/YearCalendarPage.qml"),
        QStringLiteral("src/app/qml/view/panels/BodyLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/HierarchySidebarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/ListBarHeader.qml"),
        QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/NoteListItem.qml"),
        QStringLiteral("src/app/qml/view/panels/ResourceListItem.qml"),
        QStringLiteral("src/app/qml/view/panels/StatusBarLayout.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailContents.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailFileStatForm.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailMetadataHierarchyPicker.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/NoteDetailPanel.qml"),
        QStringLiteral("src/app/qml/view/panels/detail/RightPanel.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/NavigationModeBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml"),
        QStringLiteral("src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml"),
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"),
        QStringLiteral("src/app/qml/window/Onboarding.qml"),
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"),
        QStringLiteral("src/app/qml/window/TrialStatus.qml"),
    };
    const QRegularExpression hardcodedVisualToken(QStringLiteral(
        R"(#[0-9A-Fa-f]{3,8}|Qt\.rgba\(|color:\s*"transparent"|backgroundColor(?:Disabled|Focused|Hover|Pressed)?:\s*"transparent"|color:\s*"white"|(?:spacing|Layout\.minimumWidth|font\.pixelSize|insetVertical|topPadding|rightPadding|bottomPadding|leftPadding):\s*(?:0|10|22)\b)"));

    for (const QString &filePath : tokenizedFiles) {
        const QString source = readUtf8SourceFile(filePath);
        QVERIFY2(!source.isEmpty(), qPrintable(filePath));
        QVERIFY2(source.contains(QStringLiteral("import LVRS 1.0 as LV")), qPrintable(filePath));
        const QRegularExpressionMatch match = hardcodedVisualToken.match(source);
        QVERIFY2(!match.hasMatch(),
                 qPrintable(QStringLiteral("Hardcoded visual token in %1: %2").arg(filePath, match.captured())));
        if (filePath != QStringLiteral("src/app/qml/view/panels/ListBarLayout.qml")
            && filePath != QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml")) {
            QVERIFY2(!source.contains(QStringLiteral("scaleMetric(")),
                     qPrintable(QStringLiteral("Unexpected scaled pixel literal in %1").arg(filePath)));
        }
    }
}
