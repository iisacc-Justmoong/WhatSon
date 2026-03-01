#include "hub/WhatSonHubRuntimeStore.hpp"

#include <QDir>
#include <QFile>
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

    bool buildRequiredRuntimeHub(
        const QString& hubPath,
        bool includeTagsFile)
    {
        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("RuntimeHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString resourcesPath = QDir(hubPath).filePath(QStringLiteral("RuntimeHub.wsresources"));
        const QString presetPath = QDir(contentsPath).filePath(QStringLiteral("Preset.wspreset"));
        const QString manifestDirPath = QDir(hubPath).filePath(QStringLiteral(".whatson"));

        if (!QDir().mkpath(libraryPath)
            || !QDir().mkpath(resourcesPath)
            || !QDir().mkpath(presetPath)
            || !QDir().mkpath(manifestDirPath))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(manifestDirPath).filePath(QStringLiteral("hub.json")),
            "{\n"
            "  \"coordinate\": {\n"
            "    \"x\": 24.0,\n"
            "    \"y\": 88.0\n"
            "  }\n"
            "}\n"))
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
            "{\n  \"projects\": [\"Campaign\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Bookmarks.wsbookmarks")),
            "{\n  \"bookmarks\": [\"note-1\"]\n}\n"))
        {
            return false;
        }
        if (includeTagsFile)
        {
            if (!writeUtf8File(
                QDir(contentsPath).filePath(QStringLiteral("Tags.wstags")),
                "{\n"
                "  \"tags\": [\n"
                "    {\"id\": \"root\", \"label\": \"Root\"},\n"
                "    {\"id\": \"next\", \"label\": \"Next\"}\n"
                "  ]\n"
                "}\n"))
            {
                return false;
            }
        }
        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Progress.wsprogress")),
            "{\n  \"value\": 1,\n  \"states\": [\"Ready\", \"Done\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            "{\n  \"notes\": [\"note-1\"]\n}\n"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(resourcesPath).filePath(QStringLiteral("image.png")),
            "png"))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(hubPath).filePath(QStringLiteral("RuntimeHubStat.wsstat")),
            "{\n"
            "  \"noteCount\": 9,\n"
            "  \"resourceCount\": 4,\n"
            "  \"characterCount\": 200,\n"
            "  \"participants\": [\"ProfileA\", \"ProfileB\"]\n"
            "}\n"))
        {
            return false;
        }

        return true;
    }
} // namespace

class WhatSonHubRuntimeStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void loadFromWshub_storesHubAndContentsState();
    void loadFromWshub_missingRequiredEntry_fails();
    void setPlacement_setTagDepthEntries_overrideRuntimeState();
};

void WhatSonHubRuntimeStoreTest::loadFromWshub_storesHubAndContentsState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeHub.wshub"));
    QVERIFY(buildRequiredRuntimeHub(hubPath, true));

    WhatSonHubRuntimeStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(hubPath));

    const WhatSonHubPlacement placement = store.placement(hubPath);
    QCOMPARE(placement.x(), 24.0);
    QCOMPARE(placement.y(), 88.0);
    QCOMPARE(store.tagDepthEntries(hubPath).size(), 2);
    QCOMPARE(store.hubStat(hubPath).noteCount(), 9);
    QCOMPARE(store.hubStat(hubPath).resourceCount(), 4);
    QCOMPARE(store.hubStat(hubPath).participants().size(), 2);
}

void WhatSonHubRuntimeStoreTest::loadFromWshub_missingRequiredEntry_fails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeHub.wshub"));
    QVERIFY(buildRequiredRuntimeHub(hubPath, false));

    WhatSonHubRuntimeStore store;
    QString errorMessage;
    QVERIFY(!store.loadFromWshub(hubPath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Tags.wstags")));
}

void WhatSonHubRuntimeStoreTest::setPlacement_setTagDepthEntries_overrideRuntimeState()
{
    const QString hubPath = QStringLiteral("/tmp/runtime-override.wshub");

    WhatSonHubRuntimeStore store;
    store.setPlacement(WhatSonHubPlacement(hubPath, 100.0, 200.0));

    QVector<WhatSonTagDepthEntry> entries;
    entries.push_back(WhatSonTagDepthEntry{QStringLiteral("a"), QStringLiteral("A"), 0});
    store.setTagDepthEntries(hubPath, entries);

    QVERIFY(store.contains(hubPath));
    QCOMPARE(store.placement(hubPath).x(), 100.0);
    QCOMPARE(store.placement(hubPath).y(), 200.0);
    QCOMPARE(store.tagDepthEntries(hubPath).size(), 1);

    store.remove(hubPath);
    QVERIFY(!store.contains(hubPath));
}

QTEST_APPLESS_MAIN(WhatSonHubRuntimeStoreTest)

#include "test_whatson_hub_runtime_store.moc"
