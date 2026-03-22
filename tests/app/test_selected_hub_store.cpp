#include "SelectedHubStore.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class SelectedHubStoreTest final : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void selectedHubPath_persistsNormalizedPath();
    void selectedHubSelection_persistsAccessBookmark();
    void selectedHubPath_convertsAndroidMountedHubToSourceUri();
    void startupHubPath_prefersPersistedSelection();
    void startupHubPath_preservesAndroidSourceUriSelection();
    void startupHubPath_preservesMissingStoredSelectionForMountAttempt();
    void startupHubPath_fallsBackWhenNoStoredSelectionExists();
};

void SelectedHubStoreTest::init()
{
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSonTest"));
    QCoreApplication::setApplicationName(QStringLiteral("SelectedHubStoreTest"));
}

void SelectedHubStoreTest::selectedHubPath_persistsNormalizedPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    SelectedHubStore store;
    store.setSelectedHubPath(hubPath);

    SelectedHubStore reloadedStore;
    QCOMPARE(reloadedStore.selectedHubPath(), QDir::cleanPath(hubPath));
}

void SelectedHubStoreTest::selectedHubSelection_persistsAccessBookmark()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    const QByteArray bookmarkData("bookmark-data");
    SelectedHubStore store;
    store.setSelectedHubSelection(hubPath, bookmarkData);

    SelectedHubStore reloadedStore;
    QCOMPARE(reloadedStore.selectedHubPath(), QDir::cleanPath(hubPath));
    QCOMPARE(reloadedStore.selectedHubAccessBookmark(), bookmarkData);

    reloadedStore.clearSelectedHubPath();
    QCOMPARE(reloadedStore.selectedHubPath(), QString());
    QCOMPARE(reloadedStore.selectedHubAccessBookmark(), QByteArray());
}

void SelectedHubStoreTest::selectedHubPath_convertsAndroidMountedHubToSourceUri()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString mountedHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Mounted.wshub"));
    QVERIFY(QDir().mkpath(mountedHubPath));

    const QString metadataPath = mountedHubPath + QStringLiteral(".whatson-android-mount.json");
    QFile metadataFile(metadataPath);
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    const QString sourceUri = QStringLiteral("content://whatson.provider/tree/primary%3ADownload/document/primary%3ADownload%2FMounted.wshub");
    const QJsonObject metadata{
        {QStringLiteral("version"), 1},
        {QStringLiteral("schema"), QStringLiteral("whatson.android.mount")},
        {QStringLiteral("mountedHubPath"), QDir::cleanPath(mountedHubPath)},
        {QStringLiteral("sourceUri"), sourceUri},
        {QStringLiteral("displayName"), QStringLiteral("Mounted.wshub")}
    };
    QVERIFY(metadataFile.write(QJsonDocument(metadata).toJson(QJsonDocument::Indented)) >= 0);
    metadataFile.close();

    SelectedHubStore store;
    store.setSelectedHubPath(mountedHubPath);

    SelectedHubStore reloadedStore;
    QCOMPARE(reloadedStore.selectedHubPath(), sourceUri);
}

void SelectedHubStoreTest::startupHubPath_prefersPersistedSelection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString persistedHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Persisted.wshub"));
    const QString blueprintHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Blueprint.wshub"));
    QVERIFY(QDir().mkpath(persistedHubPath));
    QVERIFY(QDir().mkpath(blueprintHubPath));

    SelectedHubStore store;
    store.setSelectedHubPath(persistedHubPath);

    QCOMPARE(store.startupHubPath(blueprintHubPath), QDir::cleanPath(persistedHubPath));
}

void SelectedHubStoreTest::startupHubPath_preservesAndroidSourceUriSelection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString blueprintHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Blueprint.wshub"));
    QVERIFY(QDir().mkpath(blueprintHubPath));

    const QString sourceUri = QStringLiteral("content://whatson.provider/tree/primary%3ADownload/document/primary%3ADownload%2FMounted.wshub");
    SelectedHubStore store;
    store.setSelectedHubPath(sourceUri);

    QCOMPARE(store.selectedHubPath(), sourceUri);
    QCOMPARE(store.startupHubPath(blueprintHubPath), sourceUri);
}

void SelectedHubStoreTest::startupHubPath_preservesMissingStoredSelectionForMountAttempt()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString missingStoredHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Missing.wshub"));
    const QString blueprintHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Blueprint.wshub"));
    QVERIFY(QDir().mkpath(blueprintHubPath));

    SelectedHubStore store;
    store.setSelectedHubPath(missingStoredHubPath);
    QCOMPARE(store.selectedHubPath(), QDir::cleanPath(missingStoredHubPath));
    QCOMPARE(store.startupHubPath(blueprintHubPath), QDir::cleanPath(missingStoredHubPath));
}

void SelectedHubStoreTest::startupHubPath_fallsBackWhenNoStoredSelectionExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir.path());
    QSettings settings;
    settings.clear();
    settings.sync();

    const QString blueprintHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Blueprint.wshub"));
    QVERIFY(QDir().mkpath(blueprintHubPath));

    SelectedHubStore store;
    QCOMPARE(store.selectedHubPath(), QString());
    QCOMPARE(store.startupHubPath(blueprintHubPath), QDir::cleanPath(blueprintHubPath));
}

QTEST_APPLESS_MAIN(SelectedHubStoreTest)

#include "test_selected_hub_store.moc"
