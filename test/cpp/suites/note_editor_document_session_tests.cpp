#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

namespace
{
    QString readUtf8FileForNoteEditorSessionTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    bool writeUtf8FileForNoteEditorSessionTest(const QString& path, const QString& text)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        file.write(text.toUtf8());
        return true;
    }
} // namespace

void WhatSonCppRegressionTests::noteEditorDocumentSession_mountsEditorHtmlFileAndPersistsBodyDocument()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("session-note"),
        QStringLiteral("Visible line\n<resource path=\"asset.png\" />"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("session-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorFilePath = session.editorFilePath();
    QVERIFY(!editorFilePath.isEmpty());
    QVERIFY(!editorFilePath.endsWith(QStringLiteral(".wsnbody")));
    QVERIFY(QFileInfo(editorFilePath).isFile());
    QCOMPARE(session.activeNoteId(), QStringLiteral("session-note"));
    QCOMPARE(session.activeNoteDirectoryPath(), QDir::cleanPath(noteDirectoryPath));
    QCOMPARE(session.parsedLineCount(), 2);
    QVERIFY(!session.readOnly());

    const QString mountedEditorSource = readUtf8FileForNoteEditorSessionTest(editorFilePath);
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<?xml")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("<contents")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("Visible line<br/>")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(mountedEditorSource.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("&lt;resource path=&quot;asset.png&quot; /&gt;")));
    QVERIFY(!mountedEditorSource.contains(QStringLiteral("Visible line\n<resource")));

    const QString editedSource = QStringLiteral("Changed line\nInserted line\n<resource path=\"asset.png\" />");
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, editedSource));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(editorFilePath));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);
    QCOMPARE(session.parsedLineCount(), 3);

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(bodyPath);
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<contents id=\"session-note\">")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph>Changed line</paragraph>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<paragraph>Inserted line</paragraph>")));
    QVERIFY(persistedBodyDocument.contains(QStringLiteral("<resource path=\"asset.png\" />")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        editedSource);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_keepsSessionSourceWhenSameNoteIsReselected()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("reselected-note"),
        QStringLiteral("Persisted line"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("reselected-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorFilePath = session.editorFilePath();
    QVERIFY(!editorFilePath.isEmpty());
    QCOMPARE(session.parsedLineCount(), 1);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(editorFilePath, QStringLiteral("Unsaved session line")));

    QVERIFY(session.openNoteForEditing(QStringLiteral("reselected-note"), noteDirectoryPath));
    QCOMPARE(session.editorFilePath(), editorFilePath);
    QCOMPARE(readUtf8FileForNoteEditorSessionTest(editorFilePath), QStringLiteral("Unsaved session line"));
    QCOMPARE(loadedSpy.count(), 1);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_buildsInlineFormatSourceInsertion()
{
    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        QStringLiteral("Alpha Beta\nGamma"));

    const auto verifyInlineInsertion =
        [&session, &editorHtml](const QString& tagName, const QString& expectedSource, const QString& expectedHtml)
    {
        const QVariantMap result = session.insertFormatTagIntoSource(
            tagName,
            editorHtml,
            6,
            4);

        QVERIFY(result.value(QStringLiteral("valid")).toBool());
        QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
        QCOMPARE(result.value(QStringLiteral("bodySourceText")).toString(), expectedSource);
        QVERIFY(result.value(QStringLiteral("editorDocumentText")).toString().contains(expectedHtml));
        QCOMPARE(
            result.value(QStringLiteral("sourceCursorPosition")).toInt(),
            expectedSource.left(expectedSource.indexOf(QLatin1Char('\n'))).size());
        QCOMPARE(
            result.value(QStringLiteral("cursorPosition")).toInt(),
            QStringLiteral("Alpha Beta").size());
    };

    verifyInlineInsertion(
        QStringLiteral("bold"),
        QStringLiteral("Alpha <bold>Beta</bold>\nGamma"),
        QStringLiteral("<strong style=\"font-weight:900;\">Beta</strong>"));
    verifyInlineInsertion(
        QStringLiteral("italic"),
        QStringLiteral("Alpha <italic>Beta</italic>\nGamma"),
        QStringLiteral("<span style=\"font-style:italic;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("underline"),
        QStringLiteral("Alpha <underline>Beta</underline>\nGamma"),
        QStringLiteral("<span style=\"text-decoration: underline;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("strikethrough"),
        QStringLiteral("Alpha <strikethrough>Beta</strikethrough>\nGamma"),
        QStringLiteral("<span style=\"text-decoration: line-through;\">Beta</span>"));
    verifyInlineInsertion(
        QStringLiteral("highlight"),
        QStringLiteral("Alpha <highlight>Beta</highlight>\nGamma"),
        QStringLiteral("<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">Beta</span>"));

    const QString formattedEditorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        QStringLiteral("<bold>Alpha</bold> Beta Gamma"));
    const QVariantMap offsetMappedResult = session.insertFormatTagIntoSource(
        QStringLiteral("italic"),
        formattedEditorHtml,
        6,
        4);
    QVERIFY(offsetMappedResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        offsetMappedResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold>Alpha</bold> <italic>Beta</italic> Gamma"));
    QCOMPARE(offsetMappedResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha Beta").size());

    const QVariantMap nestedFormatResult = session.insertFormatTagIntoSource(
        QStringLiteral("italic"),
        formattedEditorHtml,
        0,
        5);
    QVERIFY(nestedFormatResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        nestedFormatResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold><italic>Alpha</italic></bold> Beta Gamma"));
    QCOMPARE(nestedFormatResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha").size());

    const QVariantMap toggledFormatResult = session.insertFormatTagIntoSource(
        QStringLiteral("bold"),
        formattedEditorHtml,
        0,
        5);
    QVERIFY(toggledFormatResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        toggledFormatResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha Beta Gamma"));
    QCOMPARE(toggledFormatResult.value(QStringLiteral("toggledOff")).toBool(), true);
    QCOMPARE(toggledFormatResult.value(QStringLiteral("cursorPosition")).toInt(), QStringLiteral("Alpha").size());

    const QVariantMap breakResult = session.insertFormatTagIntoSource(
        QStringLiteral("break"),
        editorHtml,
        5,
        0);

    QVERIFY(breakResult.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        breakResult.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha\n</break>\n Beta\nGamma"));
    QVERIFY(breakResult.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("<br/>")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_usesSelectedTextToRepairDriftedFormatSelection()
{
    NoteEditorDocumentSession session;
    const QString selectedParagraph =
        QStringLiteral("병렬 에이전트 운용에서 가장 실용적인 최소 세트는 다음이다.");
    const QString bodySource = selectedParagraph + QStringLiteral("\n\ngit checkout main");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("format-note"),
        bodySource);

    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        editorHtml,
        QStringLiteral("병렬 에이전").size(),
        selectedParagraph.size(),
        selectedParagraph);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<highlight>병렬 에이전트 운용에서 가장 실용적인 최소 세트는 다음이다.</highlight>\n\ngit checkout main"));
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), 0);
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), selectedParagraph.size());
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("git </highlight>")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_mapsLogicalSelectionAgainstLoadedBodySourceBreaks()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("logical-selection-note"),
        QStringLiteral("Alpha Beta Gamma"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    const QString bodyPath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(
        bodyPath,
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=\"logical-selection-note\">\n"
            "  <body>\n"
            "    <paragraph>Alpha <next />Beta Gamma</paragraph>\n"
            "  </body>\n"
            "</contents>\n")));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("logical-selection-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("logical-selection-note"),
        QStringLiteral("Alpha <next />Beta Gamma"));
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        editorHtml,
        QStringLiteral("Alpha \n").size(),
        QStringLiteral("Beta").size(),
        QStringLiteral("Beta"));

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Alpha <next /><highlight>Beta</highlight> Gamma"));
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), QStringLiteral("Alpha <next />").size());
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), QStringLiteral("Beta").size());
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("B<highlight>eta ")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_formatsSelectionAgainstBodySourceWhenEditorHtmlDropsBlankLines()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("blank-line-selection-note"),
        QStringLiteral("Intro\n\n여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.\n\ngit checkout main"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("blank-line-selection-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);
    QCOMPARE(session.activeNoteId(), QStringLiteral("blank-line-selection-note"));
    QVERIFY(session.hasActiveNote());
    QCOMPARE(session.parsedLineCount(), 5);

    const QString selectedLine =
        QStringLiteral("여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.");
    const QString collapsedEditorHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /></head>"
        "<body><p>Intro</p><p>%1</p><p>git checkout main</p></body></html>").arg(selectedLine);
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("underline"),
        collapsedEditorHtml,
        QStringLiteral("Intro\n\n").size(),
        selectedLine.size());

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("selectionStart")).toInt(), QStringLiteral("Intro\n\n").size());
    QCOMPARE(result.value(QStringLiteral("selectionLength")).toInt(), selectedLine.size());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("Intro\n\n<underline>여러 에이전트를 실제로 동시에 실행하려면 단순 브랜치보다 다음 형태가 더 안정하다.</underline>\n\ngit checkout main"));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("여러 <underline>에이전트")));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("git </underline>")));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_formatsAgainstLoadedBodySourceWhenEditorProjectionDropsRawTags()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("active-source-note"),
        QStringLiteral("<bold>Alpha</bold> Beta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("active-source-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString projectionWithoutRawTags = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("active-source-note"),
        QStringLiteral("Alpha Beta"));
    const QVariantMap result = session.insertFormatTagIntoSource(
        QStringLiteral("highlight"),
        projectionWithoutRawTags,
        0,
        QStringLiteral("Alpha").size(),
        QStringLiteral("Alpha"));

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral("<bold><highlight>Alpha</highlight></bold> Beta"));
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_buildsStandaloneResourceSourceInsertion()
{
    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("resource-note"),
        QStringLiteral("Alpha\nBeta"));

    QVariantMap importedResource;
    importedResource.insert(QStringLiteral("resourceId"), QStringLiteral("capture-1"));
    importedResource.insert(
        QStringLiteral("resourcePath"),
        QStringLiteral("Workspace.wsresources/capture-1.wsresource"));
    importedResource.insert(QStringLiteral("type"), QStringLiteral("image"));
    importedResource.insert(QStringLiteral("format"), QStringLiteral(".png"));
    importedResource.insert(QStringLiteral("bucket"), QStringLiteral("Image"));

    const QVariantMap result = session.insertImportedResourcesIntoSource(
        editorHtml,
        5,
        0,
        QVariantList{importedResource});

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(
        result.value(QStringLiteral("insertedText")).toString(),
        QStringLiteral(
            "\n<resource type=\"image\" format=\".png\" "
            "path=\"Workspace.wsresources/capture-1.wsresource\" id=\"capture-1\" />"));
    QCOMPARE(
        result.value(QStringLiteral("bodySourceText")).toString(),
        QStringLiteral(
            "Alpha\n<resource type=\"image\" format=\".png\" "
            "path=\"Workspace.wsresources/capture-1.wsresource\" id=\"capture-1\" />\nBeta"));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(!result.value(QStringLiteral("bodySourceText")).toString().contains(QStringLiteral("&lt;resource")));
    const QString editorDocumentText = result.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(editorDocumentText.contains(QStringLiteral("Alpha<br/>")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource type=&quot;image&quot;")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("<br/>Beta")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("resource-note"),
            editorDocumentText),
        result.value(QStringLiteral("bodySourceText")).toString());
    QCOMPARE(session.parsedLineCount(), 3);
}

void WhatSonCppRegressionTests::noteEditorDocumentSession_rendersImportedClipboardImageResourceFrame()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardFrameHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-frame-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QImage clipboardImage(QSize(16, 10), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(18, 110, 190, 255));

    InAppClipboard clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    clipboard.setCurrentHubPath(hubPath);
    const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
    QVERIFY2(importedEntries.size() == 1, qPrintable(clipboard.lastError()));

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-frame-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    const QString editorHtml = readUtf8FileForNoteEditorSessionTest(session.editorFilePath());
    const QVariantMap result = session.insertImportedResourcesIntoSource(
        editorHtml,
        QStringLiteral("Alpha").size(),
        0,
        importedEntries);

    QVERIFY(result.value(QStringLiteral("valid")).toBool());
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    const QString bodySourceText = result.value(QStringLiteral("bodySourceText")).toString();
    const QString editorDocumentText = result.value(QStringLiteral("editorDocumentText")).toString();
    const QVariantMap importedResource = importedEntries.constFirst().toMap();

    QVERIFY(bodySourceText.contains(QStringLiteral("<resource type=\"image\" format=\".png\"")));
    QVERIFY(bodySourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-source")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("width=\"16\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("height=\"10\"")));
    QVERIFY(!editorDocumentText.contains(QStringLiteral("&lt;resource")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("clipboard-frame-note"),
            editorDocumentText),
        bodySourceText);

    QTextDocument richTextDocument;
    richTextDocument.setHtml(editorDocumentText);
    QVERIFY(writeUtf8FileForNoteEditorSessionTest(session.editorFilePath(), richTextDocument.toHtml()));

    QSignalSpy persistedSpy(&session, &NoteEditorDocumentSession::editorSourcePersistFinished);
    QVERIFY(session.persistEditorFile(session.editorFilePath()));
    QTRY_COMPARE_WITH_TIMEOUT(persistedSpy.count(), 1, 3000);
    QCOMPARE(persistedSpy.takeFirst().at(1).toBool(), true);

    const QString persistedBodyDocument = readUtf8FileForNoteEditorSessionTest(
        WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath));
    QVERIFY(persistedBodyDocument.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(persistedBodyDocument),
        bodySourceText);
}
