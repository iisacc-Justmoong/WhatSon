#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsView_keepsOnlyGutterTextEditorMinimapViews()
{
    const QDir contentsDir(QStringLiteral("src/app/qml/view/contents"));
    const QString gutterSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Gutter.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));
    const QString minimapSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Minimap.qml"));
    const QString contentViewLayoutSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(contentsDir.exists());
    const QStringList contentQmlFiles = contentsDir.entryList(QStringList{QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    QCOMPARE(contentQmlFiles, QStringList({QStringLiteral("Gutter.qml"), QStringLiteral("Minimap.qml"), QStringLiteral("TextEditor.qml")}));
    QVERIFY(!QDir(contentsDir.filePath(QStringLiteral("editor"))).exists());

    QVERIFY(!gutterSource.isEmpty());
    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!minimapSource.isEmpty());
    QVERIFY(!contentViewLayoutSource.isEmpty());
    QVERIFY(gutterSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int parsedLineCount: 0")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string sourceFilePath: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string selectedNoteId: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("property string selectedNoteDirectoryPath: \"\"")));
    QVERIFY(gutterSource.contains(QStringLiteral("readonly property bool hasSelectedSource")));
    QVERIFY(gutterSource.contains(QStringLiteral("gutter.contentY")));
    QVERIFY(!gutterSource.contains(QStringLiteral("Rectangle {")));
    QVERIFY(!gutterSource.contains(QStringLiteral("separatorColor")));
    QVERIFY(!gutterSource.contains(QStringLiteral("anchors.right: parent.right")));
    QVERIFY(textEditorSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(minimapSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property bool editorReadOnly: false")));
    QVERIFY(textEditorSource.contains(QStringLiteral("property string noteBodyFilePath: \"\"")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property string editorDocumentText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("String(textEditor.text)")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real viewportContentY")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real editorVisualLineHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.editorItem")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.contentHeight")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorSurface.lineCount")));
    QVERIFY(textEditorSource.contains(QStringLiteral("editorViewportFlickable")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function replaceEditorDocumentText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("function pasteNativeClipboardText")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.cursorPosition = Math.max")));
    QVERIFY(textEditorSource.contains(QStringLiteral("textEditor.paste()")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("preferNativeGestures: LV.Theme.mobileTarget")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("preferNativeGestures: true")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("property var documentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("property var imeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("textDocumentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorImeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("saveFile(")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("filePath: \"\"")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("property var noteEditorSession: null")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property string editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("readonly property int editorParsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteEditorSession.editorFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteEditorSession.parsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("importClipboardImageForEditor")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("insertImportedResourcesIntoSource")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("replaceEditorDocumentText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("refreshClipboardImageAvailabilitySnapshot")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("reloadImportedResources")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("insertion.editorDocumentText")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sequence: StandardKey.Paste")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("sourceFilePath: contentViewLayout.editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("selectedNoteId: contentViewLayout.editorActiveNoteId")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("selectedNoteDirectoryPath: contentViewLayout.editorActiveNoteDirectoryPath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineCount: contentViewLayout.editorParsedLineCount")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("contentY: contentsTextEditor.viewportContentY")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("lineHeight: contentsTextEditor.editorVisualLineHeight")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath: contentViewLayout.editorSourceFilePath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("persistEditorFile(path)")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath: contentViewLayout.activeNoteBodyPath")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("import WhatSon.App.Internal 1.0")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("CalendarView.")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("StackLayout")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorViewModeController")));

    const QList<QString> qmlSources = {gutterSource, textEditorSource, minimapSource};
    for (const QString &qmlSource : qmlSources) {
        QVERIFY(!qmlSource.contains(QStringLiteral("scaleMetric(")));
        QVERIFY(!qmlSource.contains(QStringLiteral("#")));
        const QRegularExpression hardcodedMetricAssignment(QStringLiteral(
            R"((?:^|\n)\s*(?:width|height|x|y|radius|anchors\.\w+Margin|Layout\.preferredWidth|implicitHeight|implicitWidth|font\.pixelSize)\s*:\s*-?\d)"));
        const QRegularExpressionMatch match = hardcodedMetricAssignment.match(qmlSource);
        QVERIFY2(!match.hasMatch(),
                 qPrintable(QStringLiteral("Hardcoded contents-view metric token: %1").arg(match.captured())));
    }
}

void WhatSonCppRegressionTests::qmlContentsView_threePartsStayViewOnlyAndNativeInputSafe()
{
    const QString gutterSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Gutter.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));
    const QString minimapSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/Minimap.qml"));
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString combinedSource = gutterSource + textEditorSource + minimapSource;

    QVERIFY(!gutterSource.isEmpty());
    QVERIFY(!textEditorSource.isEmpty());
    QVERIFY(!minimapSource.isEmpty());
    QVERIFY(!mainSource.isEmpty());
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("LV.CodeEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readonly property real viewportContentY")));
    QVERIFY(gutterSource.contains(QStringLiteral("property int parsedLineCount: 0")));
    QVERIFY(!combinedSource.contains(QStringLiteral("import WhatSon.App.Internal 1.0")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsEditorDisplayBackend")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsStructuredDocumentFlow")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsResourceEditorView")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsMinimapLayoutMetrics")));
    QVERIFY(!combinedSource.contains(QStringLiteral("ContentsLineNumberRailMetrics")));
    QVERIFY(!combinedSource.contains(QStringLiteral("sourceText")));
    QVERIFY(!combinedSource.contains(QStringLiteral("renderedText")));
    QVERIFY(!combinedSource.contains(QStringLiteral("projection")));
    QVERIFY(!combinedSource.contains(QStringLiteral("persistence")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TextEdit {")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onReleased")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("textDocumentModel")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("editorImeAdapter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("saveFile(")));
    QVERIFY(mainSource.contains(QStringLiteral("desktopMainLayoutComponent")));
    QVERIFY(mainSource.contains(QStringLiteral("mobileMainLayoutComponent")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.BodyLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.StatusBarLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("BodyPanelView.NavigationBarLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("MobilePageView.MobileHierarchyPage {")));
    QVERIFY(!mainSource.contains(QStringLiteral("editorOnlyWorkspaceComponent")));
    QVERIFY(!mainSource.contains(QStringLiteral("BodyPanelView.ContentViewLayout {")));
    QVERIFY(mainSource.contains(QStringLiteral("rootEditorViewModeController")));
    QVERIFY(mainSource.contains(QStringLiteral("editorViewModeController: applicationWindow.rootEditorViewModeController")));
}

void WhatSonCppRegressionTests::qmlOnboardingContent_routesMacCreateHubThroughDirectoryDialog()
{
    const QString onboardingSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"));

    QVERIFY(!onboardingSource.isEmpty());
    QVERIFY(onboardingSource.contains(QStringLiteral("import LVRS 1.0 as LV")));
    QVERIFY(onboardingSource.contains(
        QStringLiteral("readonly property bool useDirectoryCreateHubFlow: root.isMobilePlatform || Qt.platform.os === \"osx\"")));
    QVERIFY(!onboardingSource.contains(QStringLiteral("useMobileCreateDirectoryFlow")));
    QVERIFY(onboardingSource.contains(QStringLiteral("if (root.useDirectoryCreateHubFlow)\n                root.openCreateHubDirectoryDialog();")));
    QVERIFY(onboardingSource.contains(QStringLiteral("if (root.useDirectoryCreateHubFlow)\n            return null;")));
    QVERIFY(onboardingSource.contains(QStringLiteral("FolderDialog {\n        id: createHubDirectoryDialog")));
    QVERIFY(onboardingSource.contains(QStringLiteral("root.hubSessionController.createHubInDirectoryUrl(selectedFolder, root.requestedCreateHubFileName);")));
    QVERIFY(onboardingSource.contains(QStringLiteral("fileMode: FileDialog.SaveFile")));
    QVERIFY(onboardingSource.contains(QStringLiteral("root.createHubDialogInstance = createHubDialogComponent.createObject(root);")));
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
