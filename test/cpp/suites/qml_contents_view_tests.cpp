#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsView_keepsOnlyAllowedContentsViews()
{
    const QString contentsRoot = QStringLiteral("src/app/qml/view/contents");
    const QString docsContentsRoot = QStringLiteral("docs/src/app/qml/view/contents");
    const QStringList allowedFiles{
        QStringLiteral("Gutter.qml"),
        QStringLiteral("ImageEditor.qml"),
        QStringLiteral("Minimap.qml"),
        QStringLiteral("TextEditor.qml")
    };

    QDir contentsDir(contentsRoot);
    const QStringList qmlFiles = contentsDir.entryList({QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    QCOMPARE(qmlFiles, allowedFiles);
    QVERIFY(!QFileInfo::exists(contentsDir.filePath(QStringLiteral("DocumentEditor.qml"))));

    for (const QString& fileName : allowedFiles)
    {
        QVERIFY2(
            QFileInfo::exists(contentsDir.filePath(fileName)),
            qPrintable(QStringLiteral("Missing contents QML file: %1").arg(fileName)));
        QVERIFY2(
            QFileInfo::exists(QDir(docsContentsRoot).filePath(fileName + QStringLiteral(".md"))),
            qPrintable(QStringLiteral("Missing contents QML document: %1.md").arg(fileName)));
    }
    QVERIFY(!QFileInfo::exists(QDir(docsContentsRoot).filePath(QStringLiteral("DocumentEditor.qml.md"))));
}

void WhatSonCppRegressionTests::qmlContentsTextEditor_returnKeyExtendsEmptyLine()
{
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(textEditorSource.contains(QStringLiteral("LV.TextEditor {")));
    QVERIFY(textEditorSource.contains(QStringLiteral("filePath: textEditor.noteBodyFilePath")));
    QVERIFY(textEditorSource.contains(QStringLiteral("readOnly: textEditor.editorReadOnly")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("NoteEditorDocumentSession")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("noteEditorSession")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("EditorInputCommandFilter")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("clipboardEditorPaste")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onReturnPressed")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Keys.onEnterPressed")));
}

void WhatSonCppRegressionTests::qmlContentsViewsStayViewOnlyAndNativeInputSafe()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.ImageEditor {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter {")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentEditorToolbar {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("noteEditorSession")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("clipboardEditorPaste")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorInputCommandFilter")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("insertFormatTagIntoSource")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("requestEditorIdleRawPush")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("TextDocumentModel")));
}

void WhatSonCppRegressionTests::qmlNavigationCalendarBars_restoreTaskButtonWithoutLegacyHooks()
{
    const QString navigationBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/NavigationBarLayout.qml"));
    const QString statusBarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/StatusBarLayout.qml"));

    QVERIFY(!navigationBarSource.isEmpty());
    QVERIFY(!statusBarSource.isEmpty());
    QVERIFY(navigationBarSource.contains(QStringLiteral("dayCalendarRequested")));
    QVERIFY(navigationBarSource.contains(QStringLiteral("monthCalendarRequested")));
    QVERIFY(navigationBarSource.contains(QStringLiteral("weekCalendarRequested")));
    QVERIFY(navigationBarSource.contains(QStringLiteral("yearCalendarRequested")));
    QVERIFY(!navigationBarSource.contains(QStringLiteral("Legacy")));
}

void WhatSonCppRegressionTests::qmlNavigationCalendarButtons_mountCalendarPagesInContentSurface()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("CalendarView.DayCalendarPage")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("CalendarView.WeekCalendarPage")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("CalendarView.MonthCalendarPage")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("CalendarView.YearCalendarPage")));
    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("openCalendarNote(noteId)")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("onDayCalendarRequested")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("onMonthCalendarRequested")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("onWeekCalendarRequested")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("onYearCalendarRequested")));
}

void WhatSonCppRegressionTests::qmlOnboardingContent_routesMacCreateHubThroughDirectoryDialog()
{
    const QString onboardingContentSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"));

    QVERIFY(!onboardingContentSource.isEmpty());
    QVERIFY(onboardingContentSource.contains(QStringLiteral("createHub")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("openCreateHubDialog")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("FolderDialog")));
}

void WhatSonCppRegressionTests::qmlLvrsTokens_replaceDirectHardcodedVisualTokensOutsideContents()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));
    const QString textEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/TextEditor.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("LV.Theme.gapNone")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.Theme.fontBody")));
    QVERIFY(textEditorSource.contains(QStringLiteral("LV.Theme.textBody")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("QtQuick.Controls.Material")));
    QVERIFY(!textEditorSource.contains(QStringLiteral("QtQuick.Controls.Material")));
}
