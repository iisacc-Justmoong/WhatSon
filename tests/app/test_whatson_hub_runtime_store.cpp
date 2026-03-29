#include "hub/WhatSonHubRuntimeStore.hpp"
#include "hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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

    bool createResourcePackage(
        const QString& resourcesPath,
        const QString& resourceId,
        const QString& assetFileName,
        const QByteArray& assetPayload = QByteArray("payload"))
    {
        const QString packageDirectoryPath = QDir(resourcesPath).filePath(resourceId + QStringLiteral(".wsresource"));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return false;
        }

        const QString resourcePath = QStringLiteral("%1/%2")
                                         .arg(QFileInfo(resourcesPath).fileName(), QFileInfo(packageDirectoryPath).fileName());
        WhatSon::Resources::ResourcePackageMetadata metadata = WhatSon::Resources::buildMetadataForAssetFile(
            assetFileName,
            resourceId,
            resourcePath);

        return writeUtf8File(QDir(packageDirectoryPath).filePath(assetFileName), assetPayload)
            && writeUtf8File(
                QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
                WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8());
    }

    bool buildRequiredRuntimeHub(
        const QString& hubPath,
        bool includeTagsFile,
        QByteArray manifestText = QByteArray())
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

        if (manifestText.isEmpty())
        {
            manifestText =
                "{\n"
                "  \"coordinate\": {\n"
                "    \"x\": 24.0,\n"
                "    \"y\": 88.0\n"
                "  }\n"
                "}\n";
        }

        if (!writeUtf8File(
            QDir(manifestDirPath).filePath(QStringLiteral("hub.json")),
            manifestText))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders")),
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.folders.tree\",\n"
            "  \"folders\": [\n"
            "    {\"id\": \"Brand\", \"label\": \"Brand\"}\n"
            "  ]\n"
            "}\n"))
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
        if (!createResourcePackage(
            resourcesPath,
            QStringLiteral("image"),
            QStringLiteral("image.png"),
            QByteArray("png")))
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
    void loadFromWshub_emptyStat_succeeds();
    void loadFromWshub_missingRequiredEntry_fails();
    void loadFromWshub_allOrNothing_keepsPreviousStateOnLateFailure();
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

void WhatSonHubRuntimeStoreTest::loadFromWshub_emptyStat_succeeds()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeHub.wshub"));
    QVERIFY(buildRequiredRuntimeHub(hubPath, true));
    QVERIFY(writeUtf8File(
        QDir(hubPath).filePath(QStringLiteral("RuntimeHubStat.wsstat")),
        QByteArray{}));

    WhatSonHubRuntimeStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(hubPath));
    QCOMPARE(store.hubStat(hubPath).noteCount(), 0);
    QCOMPARE(store.hubStat(hubPath).resourceCount(), 0);
    QCOMPARE(store.hubStat(hubPath).characterCount(), 0);
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

void WhatSonHubRuntimeStoreTest::loadFromWshub_allOrNothing_keepsPreviousStateOnLateFailure()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString validHubPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeHub.wshub"));
    QVERIFY(buildRequiredRuntimeHub(validHubPath, true));

    WhatSonHubRuntimeStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(validHubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(validHubPath));

    const QString brokenHubPath = QDir(tempDir.path()).filePath(QStringLiteral("BrokenHub.wshub"));
    QVERIFY(buildRequiredRuntimeHub(
        brokenHubPath,
        true,
        "{\n"
        "  \"coordinate\": { \"x\": 10.0, \"y\": 10.0,\n"
        "}\n"));

    errorMessage.clear();
    QVERIFY(!store.loadFromWshub(brokenHubPath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Invalid hub manifest JSON")));

    QVERIFY(!store.contains(brokenHubPath));
    QVERIFY(store.contains(validHubPath));

    const QStringList paths = store.hubPaths();
    QCOMPARE(paths.size(), 1);
    QVERIFY(paths.contains(QDir::cleanPath(validHubPath)));

    QCOMPARE(store.placement(validHubPath).x(), 24.0);
    QCOMPARE(store.placement(validHubPath).y(), 88.0);
    QCOMPARE(store.tagDepthEntries(validHubPath).size(), 2);
    QCOMPARE(store.hubStat(validHubPath).noteCount(), 9);
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
