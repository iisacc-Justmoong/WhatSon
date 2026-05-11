#include "test/cpp/whatson_cpp_regression_tests.hpp"

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
