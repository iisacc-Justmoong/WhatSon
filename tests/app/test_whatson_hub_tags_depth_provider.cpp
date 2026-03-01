#include "hierarchy/tags/WhatSonHubTagsDepthProvider.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class WhatSonHubTagsDepthProviderTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void loadFromWshub_flattensNestedTagsAsDepthZero();
    void loadFromWshub_normalizesExplicitDepthToZero();
    void loadFromWshub_acceptsRootArrayFormat();
    void loadFromWshub_readsTagsFromNoteHeadersAndWritesTagsFile();
    void loadFromWshub_failsWhenNoTagsSourceExists();
    void loadFromWshub_failsWhenPathIsNotWshub();
};

void WhatSonHubTagsDepthProviderTest::loadFromWshub_flattensNestedTagsAsDepthZero()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Sample.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Sample.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString tagsPath = QDir(contentsPath).filePath(QStringLiteral("Tags.wstags"));
    QFile tagsFile(tagsPath);
    QVERIFY(tagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tagsFile.write(
        "{\n"
        "  \"version\": 1,\n"
        "  \"tags\": [\n"
        "    {\n"
        "      \"id\": \"root\",\n"
        "      \"label\": \"Root\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"root/child\",\n"
        "          \"label\": \"Child\",\n"
        "          \"children\": [\n"
        "            {\"id\": \"root/child/grand\", \"label\": \"Grand\"}\n"
        "          ]\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n");
    tagsFile.close();

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY2(provider.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QVector<WhatSonTagDepthEntry> entries = provider.tagDepthEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(entries.at(0).id, QStringLiteral("root"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(1).id, QStringLiteral("root/child"));
    QCOMPARE(entries.at(1).depth, 0);
    QCOMPARE(entries.at(2).id, QStringLiteral("root/child/grand"));
    QCOMPARE(entries.at(2).depth, 0);
}

void WhatSonHubTagsDepthProviderTest::loadFromWshub_normalizesExplicitDepthToZero()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Sample.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Sample.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString tagsPath = QDir(contentsPath).filePath(QStringLiteral("Tags.wstags"));
    QFile tagsFile(tagsPath);
    QVERIFY(tagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tagsFile.write(
        "{\n"
        "  \"tags\": [\n"
        "    {\"id\": \"alpha\", \"label\": \"Alpha\", \"depth\": 0},\n"
        "    {\"id\": \"alpha/beta\", \"name\": \"Beta\", \"depth\": 1},\n"
        "    {\"id\": \"alpha/beta/gamma\", \"title\": \"Gamma\", \"dpeth\": 2}\n"
        "  ]\n"
        "}\n");
    tagsFile.close();

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY2(provider.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QVector<WhatSonTagDepthEntry> entries = provider.tagDepthEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(entries.at(0).label, QStringLiteral("Alpha"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(1).label, QStringLiteral("Beta"));
    QCOMPARE(entries.at(1).depth, 0);
    QCOMPARE(entries.at(2).label, QStringLiteral("Gamma"));
    QCOMPARE(entries.at(2).depth, 0);
}

void WhatSonHubTagsDepthProviderTest::loadFromWshub_acceptsRootArrayFormat()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Sample.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Sample.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString tagsPath = QDir(contentsPath).filePath(QStringLiteral("Tags.wstags"));
    QFile tagsFile(tagsPath);
    QVERIFY(tagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tagsFile.write(
        "[\n"
        "  {\n"
        "    \"id\": \"root\",\n"
        "    \"name\": \"Root\",\n"
        "    \"children\": [\n"
        "      {\"id\": \"root/child\", \"text\": \"Child\"}\n"
        "    ]\n"
        "  }\n"
        "]\n");
    tagsFile.close();

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY2(provider.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QVector<WhatSonTagDepthEntry> entries = provider.tagDepthEntries();
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).label, QStringLiteral("Root"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(1).label, QStringLiteral("Child"));
    QCOMPARE(entries.at(1).depth, 0);
}

void WhatSonHubTagsDepthProviderTest::loadFromWshub_readsTagsFromNoteHeadersAndWritesTagsFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Sample.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Sample.wscontents"));
    const QString notePath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary/One.wsnote"));
    QVERIFY(QDir().mkpath(notePath));

    QFile noteHeadFile(QDir(notePath).filePath(QStringLiteral("One.wsnhead")));
    QVERIFY(noteHeadFile.open(QIODevice::WriteOnly | QIODevice::Text));
    noteHeadFile.write(
        "<contents>\n"
        "  <head>\n"
        "    <tags>\n"
        "      <tag>Brand</tag>\n"
        "      <tag>Product/Beta</tag>\n"
        "      <tag>brand</tag>\n"
        "      <tag>${tag1}</tag>\n"
        "    </tags>\n"
        "  </head>\n"
        "</contents>\n");
    noteHeadFile.close();

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY2(provider.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QVector<WhatSonTagDepthEntry> entries = provider.tagDepthEntries();
    QCOMPARE(entries.size(), 3);
    QCOMPARE(entries.at(0).id, QStringLiteral("Brand"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(1).id, QStringLiteral("Product/Beta"));
    QCOMPARE(entries.at(1).depth, 0);
    QCOMPARE(entries.at(2).id, QStringLiteral("tag1"));
    QCOMPARE(entries.at(2).depth, 0);

    QFile tagsFile(QDir(contentsPath).filePath(QStringLiteral("Tags.wstags")));
    QVERIFY(tagsFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QJsonDocument tagsDocument = QJsonDocument::fromJson(tagsFile.readAll());
    tagsFile.close();
    QVERIFY(tagsDocument.isObject());

    const QJsonObject rootObject = tagsDocument.object();
    QCOMPARE(rootObject.value(QStringLiteral("schema")).toString(), QStringLiteral("whatson.tags.tree"));
    const QJsonArray tagsArray = rootObject.value(QStringLiteral("tags")).toArray();
    QCOMPARE(tagsArray.size(), 3);
    QCOMPARE(tagsArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));
    QCOMPARE(tagsArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Product/Beta"));
    QCOMPARE(tagsArray.at(2).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("tag1"));
}

void WhatSonHubTagsDepthProviderTest::loadFromWshub_failsWhenNoTagsSourceExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Sample.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Sample.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY(!provider.loadFromWshub(hubPath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Tags.wstags")));
}

void WhatSonHubTagsDepthProviderTest::loadFromWshub_failsWhenPathIsNotWshub()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    WhatSonHubTagsDepthProvider provider;
    QString errorMessage;
    QVERIFY(!provider.loadFromWshub(tempDir.path(), &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral(".wshub")));
}

QTEST_APPLESS_MAIN(WhatSonHubTagsDepthProviderTest)

#include "test_whatson_hub_tags_depth_provider.moc"
