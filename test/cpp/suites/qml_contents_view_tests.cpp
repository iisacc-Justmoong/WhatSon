#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContentsView_keepsOnlyAllowedContentsViews()
{
    const QString contentsRoot = QStringLiteral("src/app/qml/view/contents");
    const QString docsContentsRoot = QStringLiteral("docs/src/app/qml/view/contents");
    const QStringList allowedFiles{
        QStringLiteral("ImageEditor.qml")
    };

    QDir contentsDir(contentsRoot);
    const QStringList qmlFiles = contentsDir.entryList({QStringLiteral("*.qml")}, QDir::Files, QDir::Name);
    QCOMPARE(qmlFiles, allowedFiles);
    QVERIFY(!QFileInfo::exists(contentsDir.filePath(QStringLiteral("DocumentEditor.qml"))));
    QVERIFY(!QFileInfo::exists(contentsDir.filePath(QStringLiteral("Gutter.qml"))));
    QVERIFY(!QFileInfo::exists(contentsDir.filePath(QStringLiteral("Minimap.qml"))));
    QVERIFY(!QFileInfo::exists(contentsDir.filePath(QStringLiteral("TextEditor.qml"))));

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
    QVERIFY(!QFileInfo::exists(QDir(docsContentsRoot).filePath(QStringLiteral("Gutter.qml.md"))));
    QVERIFY(!QFileInfo::exists(QDir(docsContentsRoot).filePath(QStringLiteral("Minimap.qml.md"))));
    QVERIFY(!QFileInfo::exists(QDir(docsContentsRoot).filePath(QStringLiteral("TextEditor.qml.md"))));
}

void WhatSonCppRegressionTests::qmlContentsView_excludesNoteTextEditorSurface()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("LV.TextEditor")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("noteBodyFilePath")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("NoteEditorDocumentSession")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("EditorInputCommandFilter")));
}

void WhatSonCppRegressionTests::qmlContentsViewsStayViewOnlyAndNativeInputSafe()
{
    const QString contentViewLayoutSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/ContentViewLayout.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("ContentsView.ImageEditor {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.TextEditor {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.Gutter {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentsView.Minimap {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("ContentEditorToolbar {")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("noteEditorSession")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("clipboardEditorPaste")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("editorInputCommandFilter")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("insertFormatTagIntoSource")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("requestEditorIdleRawPush")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("TextDocumentModel")));
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
    const QString imageEditorSource = readUtf8SourceFile(QStringLiteral("src/app/qml/view/contents/ImageEditor.qml"));

    QVERIFY(contentViewLayoutSource.contains(QStringLiteral("LV.Theme.gapNone")));
    QVERIFY(imageEditorSource.contains(QStringLiteral("LV.Theme")));
    QVERIFY(!contentViewLayoutSource.contains(QStringLiteral("QtQuick.Controls.Material")));
    QVERIFY(!imageEditorSource.contains(QStringLiteral("QtQuick.Controls.Material")));
}
