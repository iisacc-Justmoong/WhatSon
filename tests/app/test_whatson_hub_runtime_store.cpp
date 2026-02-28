#include "hub/WhatSonHubRuntimeStore.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class WhatSonHubRuntimeStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void loadFromWshub_storesHubAndContentsState();
    void setPlacement_setTagDepthEntries_overrideRuntimeState();
};

void WhatSonHubRuntimeStoreTest::loadFromWshub_storesHubAndContentsState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeHub.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("RuntimeHub.wscontents"));
    const QString manifestDirPath = QDir(hubPath).filePath(QStringLiteral(".whatson"));
    QVERIFY(QDir().mkpath(contentsPath));
    QVERIFY(QDir().mkpath(manifestDirPath));

    QFile manifestFile(QDir(manifestDirPath).filePath(QStringLiteral("hub.json")));
    QVERIFY(manifestFile.open(QIODevice::WriteOnly | QIODevice::Text));
    manifestFile.write(
        "{\n"
        "  \"coordinate\": {\n"
        "    \"x\": 24.0,\n"
        "    \"y\": 88.0\n"
        "  }\n"
        "}\n");
    manifestFile.close();

    QFile tagsFile(QDir(contentsPath).filePath(QStringLiteral("Tags.wstags")));
    QVERIFY(tagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tagsFile.write(
        "{\n"
        "  \"tags\": [\n"
        "    {\"id\": \"root\", \"label\": \"Root\"},\n"
        "    {\"id\": \"next\", \"label\": \"Next\"}\n"
        "  ]\n"
        "}\n");
    tagsFile.close();

    WhatSonHubRuntimeStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(hubPath));

    const WhatSonHubPlacement placement = store.placement(hubPath);
    QCOMPARE(placement.x(), 24.0);
    QCOMPARE(placement.y(), 88.0);
    QCOMPARE(store.tagDepthEntries(hubPath).size(), 2);
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
