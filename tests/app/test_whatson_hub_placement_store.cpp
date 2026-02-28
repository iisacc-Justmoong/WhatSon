#include "WhatSonHubPlacement.hpp"
#include "WhatSonHubPlacementStore.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class WhatSonHubPlacementStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void placement_getterSetter_roundTripsValues();
    void loadFromWshub_withoutManifest_defaultsToOrigin();
    void loadFromWshub_withManifestCoordinate_readsValues();
    void loadFromWshub_rejectsNonWshubPath();
};

void WhatSonHubPlacementStoreTest::placement_getterSetter_roundTripsValues()
{
    WhatSonHubPlacement placement;
    placement.setHubPath(QStringLiteral("/tmp/demo.wshub"));
    placement.setX(320.5);
    placement.setY(180.25);

    QCOMPARE(placement.hubPath(), QStringLiteral("/tmp/demo.wshub"));
    QCOMPARE(placement.x(), 320.5);
    QCOMPARE(placement.y(), 180.25);
}

void WhatSonHubPlacementStoreTest::loadFromWshub_withoutManifest_defaultsToOrigin()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    WhatSonHubPlacementStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(hubPath));

    const WhatSonHubPlacement placement = store.placement(hubPath);
    QCOMPARE(placement.hubPath(), QDir::cleanPath(hubPath));
    QCOMPARE(placement.x(), 0.0);
    QCOMPARE(placement.y(), 0.0);
}

void WhatSonHubPlacementStoreTest::loadFromWshub_withManifestCoordinate_readsValues()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("beta.wshub"));
    const QString dotWhatsonPath = QDir(hubPath).filePath(QStringLiteral(".whatson"));
    QVERIFY(QDir().mkpath(dotWhatsonPath));

    const QString manifestPath = QDir(dotWhatsonPath).filePath(QStringLiteral("hub.json"));
    QFile manifestFile(manifestPath);
    QVERIFY(manifestFile.open(QIODevice::WriteOnly | QIODevice::Text));
    manifestFile.write(
        "{\n"
        "  \"format\": \"wshub\",\n"
        "  \"coordinate\": {\n"
        "    \"x\": 144.75,\n"
        "    \"y\": 96.5\n"
        "  }\n"
        "}\n");
    manifestFile.close();

    WhatSonHubPlacementStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const WhatSonHubPlacement placement = store.placement(hubPath);
    QCOMPARE(placement.x(), 144.75);
    QCOMPARE(placement.y(), 96.5);
}

void WhatSonHubPlacementStoreTest::loadFromWshub_rejectsNonWshubPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString regularDirectory = QDir(tempDir.path()).filePath(QStringLiteral("regular-dir"));
    QVERIFY(QDir().mkpath(regularDirectory));

    WhatSonHubPlacementStore store;
    QString errorMessage;
    QVERIFY(!store.loadFromWshub(regularDirectory, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral(".wshub")));
}

QTEST_APPLESS_MAIN(WhatSonHubPlacementStoreTest)

#include "test_whatson_hub_placement_store.moc"
