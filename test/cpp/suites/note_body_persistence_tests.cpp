#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QTextDocument>

void WhatSonCppRegressionTests::noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks()
{
    const QString sourceText =
        QStringLiteral("브랜드 사이트 <weblink href=\"www.iisacc.com\">아이작닷컴</weblink>");
    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">아이작닷컴</weblink>")));
    QCOMPARE(WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument), sourceText);

    const QString htmlProjection =
        WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(bodyDocument);
    QVERIFY(htmlProjection.contains(
        QStringLiteral("<a href=\"https://www.iisacc.com\" style=\"color:#8CB4FF;text-decoration: underline;\">")));
    QVERIFY(htmlProjection.contains(QStringLiteral("아이작닷컴</a>")));

    const QString inlineStyleDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("<bold>Al<italic>pha</italic></bold><italic> Beta</italic>"));
    const QString inlineStyleHtmlProjection =
        WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(inlineStyleDocument);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(inlineStyleDocument),
        QStringLiteral("Alpha Beta"));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("<strong style=\"font-weight:900;\">Al")));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("<span style=\"font-style:italic;\">pha</span>")));
    QVERIFY(inlineStyleHtmlProjection.contains(QStringLiteral("</strong><span style=\"font-style:italic;\"> Beta</span>")));
    QVERIFY(!inlineStyleHtmlProjection.contains(QStringLiteral("&lt;bold&gt;")));

    const QString autoWrappedDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("Visit www.iisacc.com"));
    QVERIFY(autoWrappedDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink>")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_projectsSourceToEditorHtmlWithExplicitBreaks()
{
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("note"),
        QStringLiteral("First line\nSecond line"));

    QCOMPARE(editorHtml, QStringLiteral("First line<br/>Second line"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversEditorHtmlBreaksAsCanonicalSourceLines()
{
    const QString editorHtml = QStringLiteral(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /></head>"
        "<body><p>First line</p><p>Second line</p></body></html>");

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            editorHtml),
        QStringLiteral("First line\nSecond line"));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            QStringLiteral("First line<br/>Second line")),
        QStringLiteral("First line\nSecond line"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversEditorFormattingTagsFromRichText()
{
    const QString editorHtml = QStringLiteral(
        "Alpha <strong style=\"font-weight:900;\">Beta</strong> "
        "<span style=\"font-style:italic;\">Gamma</span> "
        "<span style=\"text-decoration: underline;\">Delta</span> "
        "<span style=\"text-decoration: line-through;\">Epsilon</span> "
        "<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">Zeta</span>"
        "<br/>Tail");

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("note"),
            editorHtml),
        QStringLiteral(
            "Alpha <bold>Beta</bold> <italic>Gamma</italic> <underline>Delta</underline> "
            "<strikethrough>Epsilon</strikethrough> <highlight>Zeta</highlight>\nTail"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesCrossParagraphInlineSourceTagsWithoutEscaping()
{
    const QString crossedInlineSource = QStringLiteral("<bold>첫 줄\n둘째 줄</bold>");
    const QString crossedInlineBodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        crossedInlineSource);
    QVERIFY(crossedInlineBodyDocument.contains(QStringLiteral("<paragraph><bold>첫 줄</paragraph>")));
    QVERIFY(crossedInlineBodyDocument.contains(QStringLiteral("<paragraph>둘째 줄</bold></paragraph>")));
    QVERIFY(!crossedInlineBodyDocument.contains(QStringLiteral("&lt;bold&gt;")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(crossedInlineBodyDocument),
        crossedInlineSource);
}

void WhatSonCppRegressionTests::noteBodyPersistence_projectsCalloutAsFigmaBlockAndRecoversSource()
{
    const QString sourceText =
        QStringLiteral(
            "<callout>Alpha <bold>Beta</bold> and <italic>Gamma</italic> wraps into the callout body</callout>");

    const QString editorHtml =
        WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), sourceText);

    QVERIFY(editorHtml.contains(QStringLiteral("<div class=\"whatson-callout\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("class=\"whatson-callout\"")));
    QVERIFY(!editorHtml.contains(QStringLiteral("<span class=\"whatson-callout\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-figma-node-id=\"280:7897\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-content=\"true\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("width=\"100%\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-width-mode=\"fill\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-height-mode=\"hug-contents\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-left=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-top=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-frame-padding-bottom=\"4\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-width=\"3\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-height=\"14\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-bar-radius=\"3\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-content-gap=\"12\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-frame-chrome-height=\"22\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("height:auto")));
    QVERIFY(!editorHtml.contains(QStringLiteral("data-frame-design-height")));
    QVERIFY(editorHtml.contains(QStringLiteral("background-color:#262728")));
    QVERIFY(editorHtml.contains(QStringLiteral("padding:4px 4px")));
    QVERIFY(!editorHtml.contains(QStringLiteral("border-left:3px solid #d9d9d9")));
    QVERIFY(editorHtml.contains(QStringLiteral("class=\"whatson-callout-leading-bar\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("class=\"whatson-callout-content-gap\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("data-callout-frame-chrome=\"true\"")));
    QVERIFY(editorHtml.contains(QStringLiteral("<strong style=\"font-weight:900;\">Beta</strong>")));
    QVERIFY(editorHtml.contains(QStringLiteral("<span style=\"font-style:italic;\">Gamma</span>")));
    QVERIFY(!editorHtml.contains(QStringLiteral("<callout>")));

    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QString editorPlainText = editorDocument.toPlainText().trimmed();
    editorPlainText.remove(QChar::ObjectReplacementCharacter);
    QCOMPARE(editorPlainText.split(QLatin1Char('\n'), Qt::KeepEmptyParts).size(), 1);
    QVERIFY(editorPlainText.contains(QStringLiteral("Alpha Beta and Gamma wraps into the callout body")));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), editorHtml),
        sourceText);

    QTextDocument roundTrippedEditorDocument;
    roundTrippedEditorDocument.setHtml(editorHtml);
    const QString roundTrippedEditorHtml = roundTrippedEditorDocument.toHtml();
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("whatson-callout-source:")));
    QVERIFY(!roundTrippedEditorHtml.contains(QStringLiteral("data-callout-content")));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(QStringLiteral("note"), roundTrippedEditorHtml),
        sourceText);
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesExplicitBlankLineBeforeStandaloneCallout()
{
    const QString sourceText =
        QStringLiteral("Before\n"
                       "\n"
                       "<callout>Inside</callout>\n"
                       "After");
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("callout-explicit-blank-line-note"),
        sourceText);
    QTextDocument editorDocument;
    editorDocument.setHtml(editorHtml);
    QString editorVisiblePlainText = editorDocument.toPlainText();
    editorVisiblePlainText.remove(QChar::ObjectReplacementCharacter);
    editorVisiblePlainText.remove(QChar(0x200B));
    QCOMPARE(editorVisiblePlainText, QStringLiteral("Before\n\nInside\nAfter"));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
            QStringLiteral("callout-explicit-blank-line-note"),
            editorHtml),
        sourceText);
}

void WhatSonCppRegressionTests::noteBodyPersistence_doesNotReplicateParagraphsAroundRepeatedCalloutSaves()
{
    const QString sourceText =
        QStringLiteral("Before\n"
                       "<callout>Inside <bold>callout</bold></callout>\n"
                       "After");
    QString editorHtml =
        WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), sourceText);

    for (int cycle = 0; cycle < 4; ++cycle)
    {
        QTextDocument editorDocument;
        editorDocument.setHtml(editorHtml);
        const QString qtSerializedEditorHtml = editorDocument.toHtml();
        const QString recoveredSource =
            WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
                QStringLiteral("note"),
                qtSerializedEditorHtml);

        QCOMPARE(recoveredSource, sourceText);
        QCOMPARE(recoveredSource.split(QLatin1Char('\n'), Qt::KeepEmptyParts).size(), 3);
        QVERIFY(!recoveredSource.contains(QStringLiteral("\n\n<callout>")));
        QVERIFY(!recoveredSource.contains(QStringLiteral("</callout>\n\n")));

        editorHtml =
            WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), recoveredSource);
    }
}

void WhatSonCppRegressionTests::noteBodyPersistence_persistsCalloutAndAgendaAsParagraphTags()
{
    const QString sourceText =
        QStringLiteral("<callout>Alpha & <bold>Beta</bold></callout>\n"
                       "<agenda><task>Draft & review</task></agenda>");
    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(
        QStringLiteral("    <paragraph><callout>Alpha &amp; <bold>Beta</bold></callout></paragraph>\n")));
    QVERIFY(bodyDocument.contains(
        QStringLiteral("    <paragraph><agenda><task>Draft &amp; review</task></agenda></paragraph>\n")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("    <callout>")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("    <agenda>")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("&lt;callout")));
    QVERIFY(!bodyDocument.contains(QStringLiteral("&lt;agenda")));

    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument),
        sourceText);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocument),
        QStringLiteral("Alpha & Beta\nDraft & review"));

    const QString legacyParagraphWrappedBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <paragraph>&lt;callout&gt;Legacy&lt;/callout&gt;</paragraph>\n"
        "  </body>\n"
        "</contents>\n");
    const QString recoveredSource =
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(legacyParagraphWrappedBodyDocument);
    QCOMPARE(recoveredSource, QStringLiteral("<callout>Legacy</callout>"));
    const QString recoveredBodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), recoveredSource);
    QVERIFY(recoveredBodyDocument.contains(QStringLiteral("    <paragraph><callout>Legacy</callout></paragraph>\n")));
    QVERIFY(!recoveredBodyDocument.contains(QStringLiteral("    <callout>Legacy</callout>\n")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_changedPlainTextSaveAdvancesModifiedCount()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("body-persistence-count-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    QString normalizedBodyText;
    QString normalizedBodySourceText;
    QString lastModifiedAt;
    QString saveError;
    QVERIFY2(
        WhatSon::NoteBodyPersistence::persistBodyPlainText(
            noteId,
            noteDirectoryPath,
            QString(),
            QStringLiteral("after"),
            &normalizedBodyText,
            &normalizedBodySourceText,
            &lastModifiedAt,
            &saveError),
        qPrintable(saveError));
    QCOMPARE(normalizedBodyText, QStringLiteral("after"));
    QCOMPARE(normalizedBodySourceText, QStringLiteral("after"));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read saved note: %1").arg(readError)));
    QCOMPARE(document.headerStore.modifiedCount(), 1);
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("after"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_stripsRenderedHtmlBlockArtifactsFromSourceProjection()
{
    const QString bodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <!--whatson-resource-block:0--><p style=\"margin-top:0px;margin-bottom:0px;\">alpha</p><!--/whatson-resource-block:0-->\n"
        "  </body>\n"
        "</contents>\n");

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument);
    QCOMPARE(sourceText, QStringLiteral("alpha"));
    QVERIFY(!sourceText.contains(QStringLiteral("whatson-resource-block")));
    QVERIFY(!sourceText.contains(QStringLiteral("<p")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_recoversRenderedResourceFrameMarkersAsSourceTags()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    const QString encodedResourceTag = QString::fromLatin1(resourceTag.toUtf8().toHex());
    const QString renderedEditorHtml = QStringLiteral(
                                           "<p>Alpha</p>"
                                           "<!--whatson-resource-source:%1-->"
                                           "<table class=\"whatson-resource-frame\" data-resource-preview=\"image-only-frame\">"
                                           "<tr><td><img class=\"whatson-resource-media\" src=\"data:image/png;base64,ZmFrZQ==\" /></td></tr>"
                                           "</table>"
                                           "<!--/whatson-resource-source-->"
                                           "<p>Beta</p>")
                                           .arg(encodedResourceTag);

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("resource-note"),
        renderedEditorHtml);

    QCOMPARE(
        sourceText,
        QStringLiteral("Alpha\n%1\nBeta").arg(resourceTag));
}

void WhatSonCppRegressionTests::noteBodyPersistence_dropsDeletedSingleResourceObjectMarker()
{
    const QString resourceTag =
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"Workspace.wsresources/capture.wsresource\" id=\"capture\" />");
    const QString encodedResourceTag = QString::fromLatin1(resourceTag.toUtf8().toHex());
    const QString renderedEditorHtml = QStringLiteral(
                                           "<p>Alpha</p>"
                                           "<!--whatson-resource-source:%1-->"
                                           "<!--/whatson-resource-source-->"
                                           "<p>Beta</p>")
                                           .arg(encodedResourceTag);

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        QStringLiteral("resource-note"),
        renderedEditorHtml);

    QCOMPARE(sourceText, QStringLiteral("Alpha\nBeta"));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesEmptyParagraphCursorLineAfterResource()
{
    const QString bodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <resource type=\"image\" path=\"cover.png\" />\n"
        "    <paragraph></paragraph>\n"
        "  </body>\n"
        "</contents>\n");

    const QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument);
    QCOMPARE(sourceText, QStringLiteral("<resource type=\"image\" path=\"cover.png\" />\n"));

    const QString roundTripDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);
    QVERIFY(roundTripDocument.contains(
        QStringLiteral("<resource type=\"image\" path=\"cover.png\" />\n"
                       "    <paragraph></paragraph>")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_preservesEmptyParagraphBoundariesAroundResources()
{
    const QString leadingEmptyParagraphBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <paragraph></paragraph>\n"
        "    <resource type=\"image\" path=\"cover.png\" />\n"
        "  </body>\n"
        "</contents>\n");
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(leadingEmptyParagraphBodyDocument),
        QStringLiteral("\n<resource type=\"image\" path=\"cover.png\" />"));

    const QString interiorEmptyParagraphBodyDocument = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note\">\n"
        "  <body>\n"
        "    <resource type=\"image\" path=\"first.png\" />\n"
        "    <paragraph></paragraph>\n"
        "    <resource type=\"image\" path=\"second.png\" />\n"
        "  </body>\n"
        "</contents>\n");
    QCOMPARE(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(interiorEmptyParagraphBodyDocument),
        QStringLiteral(
            "<resource type=\"image\" path=\"first.png\" />\n"
            "\n"
            "<resource type=\"image\" path=\"second.png\" />"));
}
