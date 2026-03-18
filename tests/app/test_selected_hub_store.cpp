#include "SelectedHubStore.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class SelectedHubStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void init();
    void selectedHubPath_persistsNormalizedPath();
    void startupHubPath_prefersPersistedSelection();
    void startupHubPath_fallsBackWhenStoredSelectionIsMissing();
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

void SelectedHubStoreTest::startupHubPath_fallsBackWhenStoredSelectionIsMissing()
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
    QCOMPARE(store.selectedHubPath(), QString());
    QCOMPARE(store.startupHubPath(blueprintHubPath), QDir::cleanPath(blueprintHubPath));
}

QTEST_APPLESS_MAIN(SelectedHubStoreTest)

#include "test_selected_hub_store.moc"
