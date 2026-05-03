#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::noteHeaderParser_usesIiXmlDocumentTreeForWsnHead()
{
    const QString parserSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/note/WhatSonNoteHeaderParser.cpp"));

    QVERIFY(!parserSource.isEmpty());
    QVERIFY(parserSource.contains(QStringLiteral("#include <iiXml.h>")));
    QVERIFY(parserSource.contains(QStringLiteral("iiXml::Parser::TagParser")));
    QVERIFY(parserSource.contains(QStringLiteral("ParseAllDocumentResult")));
    QVERIFY(!parserSource.contains(QStringLiteral("QRegularExpression")));

    const QString headerText = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE WHATSONNOTE>\n"
        "<contents id=\"note-alpha\">\n"
        "  <head>\n"
        "    <created>2026-05-01-10-00-00</created>\n"
        "    <author>Alice &amp; Bob</author>\n"
        "    <lastModified>2026-05-01-11-00-00</lastModified>\n"
        "    <lastOpened>2026-05-01T12:00:00Z</lastOpened>\n"
        "    <modifiedBy>Carol</modifiedBy>\n"
        "    <folders>\n"
        "      <folder uuid=\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB\">Library/Research</folder>\n"
        "      <folder>Inbox</folder>\n"
        "    </folders>\n"
        "    <project>Parser Migration</project>\n"
        "    <bookmarks state=\"true\" colors=\"red, blue\" />\n"
        "    <tags>\n"
        "      <tag>xml</tag>\n"
        "      <tag>html-block</tag>\n"
        "    </tags>\n"
        "    <fileStat>\n"
        "      <totalFolders>2</totalFolders>\n"
        "      <totalTags>2</totalTags>\n"
        "      <letterCount>12</letterCount>\n"
        "      <wordCount>3</wordCount>\n"
        "      <sentenceCount>1</sentenceCount>\n"
        "      <paragraphCount>1</paragraphCount>\n"
        "      <spaceCount>2</spaceCount>\n"
        "      <indentCount>4</indentCount>\n"
        "      <lineCount>8</lineCount>\n"
        "      <openCount>5</openCount>\n"
        "      <modifiedCount>6</modifiedCount>\n"
        "      <backlinkToCount>7</backlinkToCount>\n"
        "      <backlinkByCount>8</backlinkByCount>\n"
        "      <includedResourceCount>9</includedResourceCount>\n"
        "    </fileStat>\n"
        "    <progress enums=\"{First draft, Review, Done}\">Review</progress>\n"
        "    <isPreset value=\"true\" />\n"
        "  </head>\n"
        "</contents>\n");

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &headerStore, &parseError), qPrintable(parseError));

    QCOMPARE(headerStore.noteId(), QStringLiteral("note-alpha"));
    QCOMPARE(headerStore.author(), QStringLiteral("Alice & Bob"));
    QCOMPARE(headerStore.folders(), QStringList({QStringLiteral("Library/Research"), QStringLiteral("Inbox")}));
    QCOMPARE(
        headerStore.folderUuids(),
        QStringList({QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"), QString()}));
    QCOMPARE(headerStore.project(), QStringLiteral("Parser Migration"));
    QVERIFY(headerStore.isBookmarked());
    QCOMPARE(headerStore.tags(), QStringList({QStringLiteral("xml"), QStringLiteral("html-block")}));
    QCOMPARE(headerStore.totalFolders(), 2);
    QCOMPARE(headerStore.totalTags(), 2);
    QCOMPARE(headerStore.letterCount(), 12);
    QCOMPARE(headerStore.wordCount(), 3);
    QCOMPARE(headerStore.sentenceCount(), 1);
    QCOMPARE(headerStore.paragraphCount(), 1);
    QCOMPARE(headerStore.spaceCount(), 2);
    QCOMPARE(headerStore.indentCount(), 4);
    QCOMPARE(headerStore.lineCount(), 8);
    QCOMPARE(headerStore.openCount(), 5);
    QCOMPARE(headerStore.modifiedCount(), 6);
    QCOMPARE(headerStore.backlinkToCount(), 7);
    QCOMPARE(headerStore.backlinkByCount(), 8);
    QCOMPARE(headerStore.includedResourceCount(), 9);
    QCOMPARE(
        headerStore.progressEnums(),
        QStringList({QStringLiteral("First draft"), QStringLiteral("Review"), QStringLiteral("Done")}));
    QCOMPARE(headerStore.progress(), 1);
    QVERIFY(headerStore.isPreset());
}

void WhatSonCppRegressionTests::localNoteFileStore_usesIiXmlDocumentTreeForWsnBodyRead()
{
    const QString storeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/note/WhatSonLocalNoteFileStore.cpp"));

    QVERIFY(!storeSource.isEmpty());
    QVERIFY(storeSource.contains(QStringLiteral("#include <iiXml.h>")));
    QVERIFY(storeSource.contains(QStringLiteral("iiXml::Parser::TagParser")));
    QVERIFY(storeSource.contains(QStringLiteral("ParseAllDocumentResult")));
    QVERIFY(!storeSource.contains(QStringLiteral("bodyPattern")));
    QVERIFY(!storeSource.contains(QStringLiteral("resourcePattern")));

    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("iixml-body-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("<paragraph>Alpha</paragraph>\n<resource type=\"image\" path=\"cover.png\" />"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create iiXml body note fixture: %1").arg(createError)));

    QFile coverFile(QDir(noteDirectoryPath).filePath(QStringLiteral("cover.png")));
    QVERIFY2(
        coverFile.open(QIODevice::WriteOnly | QIODevice::Truncate),
        qPrintable(QStringLiteral("Failed to create cover fixture: %1").arg(coverFile.fileName())));
    QVERIFY(coverFile.write("png") > 0);
    coverFile.close();

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read iiXml body note fixture: %1").arg(readError)));

    QVERIFY(document.bodyHasResource);
    QVERIFY(document.bodyFirstResourceThumbnailUrl.contains(QStringLiteral("cover.png")));
    QVERIFY(document.bodySourceText.contains(QStringLiteral("Alpha")));
    QVERIFY(document.bodySourceText.contains(QStringLiteral("<resource ")));
}
