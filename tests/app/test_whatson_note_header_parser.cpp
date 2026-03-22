#include "note/WhatSonNoteHeaderCreator.hpp"
#include "note/WhatSonNoteHeaderParser.hpp"
#include "note/WhatSonNoteHeaderStore.hpp"

#include <QtTest>

class WhatSonNoteHeaderParserTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void parse_readsWsnHeadFieldsWithExpectedTypes();
    void parse_mapsProgressEnumLabelToInteger();
    void parse_resolvesTemplateTokensInFolderAndTagArrays();
    void parse_dropsReservedTodayFolderTokens();
    void parse_resolvesTemplateTokensInSingleFields();
    void createHeaderText_roundTripsThroughParser();
};

void WhatSonNoteHeaderParserTest::parse_readsWsnHeadFieldsWithExpectedTypes()
{
    const QString input =
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=note-001>\n"
            "  <head>\n"
            "    <title>Brand Plan</title>\n"
            "    <created>2026-03-01-11-12-13</created>\n"
            "    <author>Planner</author>\n"
            "    <lastModified>2026-03-02-01-02-03</lastModified>\n"
            "    <modifiedBy>Editor</modifiedBy>\n"
            "    <folders>\n"
            "      <folder>Inbox</folder>\n"
            "      <folder>Q1/Brand</folder>\n"
            "    </folders>\n"
            "    <project>Main Project</project>\n"
            "    <bookmarks state=true colors={blue,#ef4444,\"purple\"} />\n"
            "    <tags>\n"
            "      <tag>creative</tag>\n"
            "      <tag>launch</tag>\n"
            "    </tags>\n"
            "    <progress enums={Ready,Pending,InProgress,Done}>2</progress>\n"
            "    <isPreset>false</isPreset>\n"
            "  </head>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore store;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(input, &store, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(store.noteId(), QStringLiteral("note-001"));
    QCOMPARE(store.createdAt(), QStringLiteral("2026-03-01-11-12-13"));
    QCOMPARE(store.author(), QStringLiteral("Planner"));
    QCOMPARE(store.lastModifiedAt(), QStringLiteral("2026-03-02-01-02-03"));
    QCOMPARE(store.modifiedBy(), QStringLiteral("Editor"));
    QCOMPARE(store.folders(), QStringList({QStringLiteral("Inbox"), QStringLiteral("Q1/Brand")}));
    QCOMPARE(store.project(), QStringLiteral("Main Project"));
    QVERIFY(store.isBookmarked());
    QCOMPARE(store.bookmarkColors(), QStringList({
                 QStringLiteral("blue"), QStringLiteral("#EF4444"), QStringLiteral("purple")
             }));
    QCOMPARE(store.tags(), QStringList({QStringLiteral("creative"), QStringLiteral("launch")}));
    QCOMPARE(store.progress(), 2);
    QVERIFY(!store.isPreset());
}

void WhatSonNoteHeaderParserTest::parse_mapsProgressEnumLabelToInteger()
{
    const QString input =
        QStringLiteral(
            "<contents id=note-002>\n"
            "  <head>\n"
            "    <progress enums={Ready,Pending,InProgress,Done}>InProgress</progress>\n"
            "  </head>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore store;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(input, &store, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(store.progress(), 2);
}

void WhatSonNoteHeaderParserTest::parse_resolvesTemplateTokensInFolderAndTagArrays()
{
    const QString input =
        QStringLiteral(
            "<contents id=${id}>\n"
            "  <head>\n"
            "    <folders>\n"
            "      <folder>%{folder1}</folder>\n"
            "      <folder>Folder-A</folder>\n"
            "    </folders>\n"
            "    <tags>\n"
            "      <tag>${tag1}</tag>\n"
            "      <tag>tag-a</tag>\n"
            "    </tags>\n"
            "  </head>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore store;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(input, &store, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(store.folders(), QStringList({QStringLiteral("folder1"), QStringLiteral("Folder-A")}));
    QCOMPARE(store.tags(), QStringList({QStringLiteral("tag1"), QStringLiteral("tag-a")}));
}

void WhatSonNoteHeaderParserTest::parse_dropsReservedTodayFolderTokens()
{
    const QString input =
        QStringLiteral(
            "<contents id=note-003>\n"
            "  <head>\n"
            "    <folders>\n"
            "      <folder>Today</folder>\n"
            "      <folder>Research/Today</folder>\n"
            "      <folder>Inbox</folder>\n"
            "    </folders>\n"
            "  </head>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore store;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(input, &store, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(store.folders(), QStringList({QStringLiteral("Inbox")}));
}

void WhatSonNoteHeaderParserTest::parse_resolvesTemplateTokensInSingleFields()
{
    const QString input =
        QStringLiteral(
            "<contents id=${id}>\n"
            "  <head>\n"
            "    <title>Placeholder</title>\n"
            "    <created>YYYY-MM-DD-hh-mm-ss</created>\n"
            "    <author>${ProfileName}</author>\n"
            "    <lastModified>YYYY-MM-DD-hh-mm-ss</lastModified>\n"
            "    <modifiedBy>${ProfileName}</modifiedBy>\n"
            "    <project>${projectName}</project>\n"
            "  </head>\n"
            "</contents>\n");

    WhatSonNoteHeaderStore store;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(input, &store, &errorMessage), qPrintable(errorMessage));

    QVERIFY(store.noteId().startsWith(QStringLiteral("note-")));
    QCOMPARE(store.author(), QStringLiteral("ProfileName"));
    QCOMPARE(store.modifiedBy(), QStringLiteral("ProfileName"));
    QCOMPARE(store.project(), QStringLiteral("projectName"));

    QCOMPARE(store.createdAt(), QString());
    QCOMPARE(store.lastModifiedAt(), QString());
}

void WhatSonNoteHeaderParserTest::createHeaderText_roundTripsThroughParser()
{
    WhatSonNoteHeaderStore seed;
    seed.setNoteId(QStringLiteral("seed-note"));
    seed.setCreatedAt(QStringLiteral("2026-03-01-00-00-00"));
    seed.setAuthor(QStringLiteral("Author"));
    seed.setLastModifiedAt(QStringLiteral("2026-03-01-10-10-10"));
    seed.setModifiedBy(QStringLiteral("Editor"));
    seed.setFolders(QStringList({QStringLiteral("One"), QStringLiteral("Two")}));
    seed.setProject(QStringLiteral("Alpha"));
    seed.setBookmarked(true);
    seed.setBookmarkColors(QStringList({QStringLiteral("blue"), QStringLiteral("pink")}));
    seed.setTags(QStringList({QStringLiteral("tag1"), QStringLiteral("tag2")}));
    seed.setProgress(3);
    seed.setPreset(true);

    const WhatSonNoteHeaderCreator creator(QStringLiteral("/tmp"));
    const QString encoded = creator.createHeaderText(seed);
    QVERIFY(!encoded.contains(QStringLiteral("<title>")));

    WhatSonNoteHeaderStore decoded;
    WhatSonNoteHeaderParser parser;
    QString errorMessage;
    QVERIFY2(parser.parse(encoded, &decoded, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(decoded.noteId(), seed.noteId());
    QCOMPARE(decoded.folders(), seed.folders());
    QCOMPARE(decoded.bookmarkColors(), seed.bookmarkColors());
    QCOMPARE(decoded.tags(), seed.tags());
    QCOMPARE(decoded.progress(), seed.progress());
    QCOMPARE(decoded.isBookmarked(), seed.isBookmarked());
    QCOMPARE(decoded.isPreset(), seed.isPreset());
}

QTEST_APPLESS_MAIN(WhatSonNoteHeaderParserTest)

#include "test_whatson_note_header_parser.moc"
