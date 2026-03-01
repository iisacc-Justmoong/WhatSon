#include "hub/WhatSonHubParser.hpp"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace
{
    bool writeUtf8File(const QString& filePath, const QByteArray& text)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return false;
        }
        return file.write(text) == text.size();
    }

    bool buildRequiredHubTree(
        const QString& hubPath,
        const QByteArray& statText,
        bool createStatFile)
    {
        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Alpha.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString resourcesPath = QDir(hubPath).filePath(QStringLiteral("Alpha.wsresources"));
        const QString presetPath = QDir(contentsPath).filePath(QStringLiteral("Preset.wspreset"));
        if (!QDir().mkpath(libraryPath)
            || !QDir().mkpath(resourcesPath)
            || !QDir().mkpath(presetPath))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders")),
            "{\n  \"folders\": [{\"id\": \"Brand\", \"label\": \"Brand\", \"depth\": 0}]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj")),
            "{\n  \"projects\": [\"Campaign\", \"Product\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Bookmarks.wsbookmarks")),
            "{\n  \"bookmarks\": [\"note-1\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Tags.wstags")),
            "{\n  \"tags\": [{\"id\": \"tag1\", \"label\": \"Tag 1\", \"depth\": 0}]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Progress.wsprogress")),
            "{\n  \"value\": 1,\n  \"states\": [\"Ready\", \"Done\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            "{\n  \"notes\": [\"note-1\", \"note-2\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(resourcesPath).filePath(QStringLiteral("image.png")),
            "png"))
        {
            return false;
        }

        if (createStatFile)
        {
            if (!writeUtf8File(
                QDir(hubPath).filePath(QStringLiteral("AlphaStat.wsstat")),
                statText))
            {
                return false;
            }
        }

        return true;
    }
} // namespace

class WhatSonHubParserTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void parseFromWshub_readsHubStructureAndDomainValues();
    void parseFromWshub_missingStat_fails();
    void parseFromWshub_emptyStat_succeedsWithDefaultStat();
    void requestParseFromWshub_emitsSignals();
};

void WhatSonHubParserTest::parseFromWshub_readsHubStructureAndDomainValues()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(buildRequiredHubTree(
        hubPath,
        "{\n"
        "  \"noteCount\": 31,\n"
        "  \"resourceCount\": 10,\n"
        "  \"characterCount\": 2048,\n"
        "  \"createdAtUtc\": \"2026-03-01T00:00:00Z\",\n"
        "  \"lastModifiedAtUtc\": \"2026-03-01T01:00:00Z\",\n"
        "  \"participants\": [\"ProfileName\", \"Guest\"],\n"
        "  \"profileLastModifiedAtUtc\": {\n"
        "    \"ProfileName\": \"2026-03-01T01:00:00Z\"\n"
        "  }\n"
        "}\n",
        true));

    WhatSonHubParser parser;
    WhatSonHubStore store;
    QString errorMessage;
    QVERIFY2(parser.parseFromWshub(hubPath, &store, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(store.hubPath(), QDir::cleanPath(hubPath));
    QCOMPARE(store.hubName(), QStringLiteral("Alpha"));
    QCOMPARE(store.stat().noteCount(), 31);
    QCOMPARE(store.stat().resourceCount(), 10);
    QCOMPARE(store.stat().characterCount(), 2048);
    QCOMPARE(store.stat().participants().size(), 2);

    const QVariantMap domains = store.domainValues();
    QCOMPARE(domains.value(QStringLiteral("projects")).toStringList().size(), 2);
    QCOMPARE(domains.value(QStringLiteral("bookmarks")).toStringList().size(), 1);
    QCOMPARE(domains.value(QStringLiteral("resourcePaths")).toStringList().size(), 1);
    QCOMPARE(domains.value(QStringLiteral("libraryNoteIds")).toStringList().size(), 2);
}

void WhatSonHubParserTest::parseFromWshub_missingStat_fails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(buildRequiredHubTree(hubPath, QByteArray{}, false));

    WhatSonHubParser parser;
    WhatSonHubStore store;
    QString errorMessage;
    QVERIFY(!parser.parseFromWshub(hubPath, &store, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral(".wsstat")));
}

void WhatSonHubParserTest::parseFromWshub_emptyStat_succeedsWithDefaultStat()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(buildRequiredHubTree(hubPath, "", true));

    WhatSonHubParser parser;
    WhatSonHubStore store;
    QString errorMessage;
    QVERIFY2(parser.parseFromWshub(hubPath, &store, &errorMessage), qPrintable(errorMessage));
    QCOMPARE(store.stat().noteCount(), 0);
    QCOMPARE(store.stat().resourceCount(), 0);
    QCOMPARE(store.stat().characterCount(), 0);
    QVERIFY(store.stat().createdAtUtc().isEmpty());
    QVERIFY(store.stat().lastModifiedAtUtc().isEmpty());
}

void WhatSonHubParserTest::requestParseFromWshub_emitsSignals()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(buildRequiredHubTree(
        hubPath,
        "{\n  \"noteCount\": 1,\n  \"resourceCount\": 1,\n  \"characterCount\": 3\n}\n",
        true));

    WhatSonHubParser parser;
    QSignalSpy hubSpy(&parser, &WhatSonHubParser::hubParsed);
    QSignalSpy statSpy(&parser, &WhatSonHubParser::hubStatParsed);
    QSignalSpy domainSpy(&parser, &WhatSonHubParser::hubDomainsParsed);
    QSignalSpy failSpy(&parser, &WhatSonHubParser::parseFailed);

    parser.requestParseFromWshub(hubPath);

    QCOMPARE(hubSpy.count(), 1);
    QCOMPARE(statSpy.count(), 1);
    QCOMPARE(domainSpy.count(), 1);
    QCOMPARE(failSpy.count(), 0);

    const QList<QVariant> statSignalArgs = statSpy.takeFirst();
    const QVariantMap statPayload = statSignalArgs.at(1).toMap();
    QCOMPARE(statPayload.value(QStringLiteral("noteCount")).toInt(), 1);
    QCOMPARE(statPayload.value(QStringLiteral("resourceCount")).toInt(), 1);
}

QTEST_APPLESS_MAIN(WhatSonHubParserTest)

#include "test_whatson_hub_parser.moc"
