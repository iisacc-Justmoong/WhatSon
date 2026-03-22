#include "sync/WhatSonHubSyncController.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace
{
    bool writeUtf8File(const QString& path, const QString& text)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        return file.write(text.toUtf8()) >= 0;
    }
}

class WhatSonHubSyncControllerTest final : public QObject
{
    Q_OBJECT

private slots:
    void requestSyncHint_reloadsWhenHubSignatureChanges();
    void requestSyncHint_ignoresAppOwnedWriteLeaseChanges();
    void acknowledgeLocalMutation_refreshesBaselineWithoutReload();
};

void WhatSonHubSyncControllerTest::requestSyncHint_reloadsWhenHubSignatureChanges()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    QVERIFY(writeUtf8File(QDir(hubPath).filePath(QStringLiteral("index.wsnindex")), QStringLiteral("alpha")));

    WhatSonHubSyncController controller;
    controller.setDebounceIntervalMs(0);
    controller.setPeriodicIntervalMs(60000);

    QSignalSpy syncReloadedSpy(&controller, &WhatSonHubSyncController::syncReloaded);
    QSignalSpy syncFailedSpy(&controller, &WhatSonHubSyncController::syncFailed);
    int reloadCount = 0;
    controller.setReloadCallback(
        [&reloadCount](const QString& hubPathArg, QString* errorMessage) -> bool
        {
            Q_UNUSED(hubPathArg);
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            ++reloadCount;
            return true;
        });

    controller.setCurrentHubPath(hubPath);
    QCOMPARE(reloadCount, 0);

    QVERIFY(writeUtf8File(QDir(hubPath).filePath(QStringLiteral("index.wsnindex")), QStringLiteral("beta")));
    controller.requestSyncHint();

    QTRY_COMPARE(reloadCount, 1);
    QCOMPARE(syncReloadedSpy.count(), 1);
    QCOMPARE(syncFailedSpy.count(), 0);
}

void WhatSonHubSyncControllerTest::requestSyncHint_ignoresAppOwnedWriteLeaseChanges()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Lease.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    QVERIFY(writeUtf8File(QDir(hubPath).filePath(QStringLiteral("index.wsnindex")), QStringLiteral("alpha")));
    QVERIFY(QDir().mkpath(QDir(hubPath).filePath(QStringLiteral(".whatson"))));
    QVERIFY(
        writeUtf8File(
            QDir(hubPath).filePath(QStringLiteral(".whatson/write-lease.json")),
            QStringLiteral("{\"owner\":\"bootstrap\"}")));

    WhatSonHubSyncController controller;
    controller.setDebounceIntervalMs(0);
    controller.setPeriodicIntervalMs(60000);

    QSignalSpy syncReloadedSpy(&controller, &WhatSonHubSyncController::syncReloaded);
    int reloadCount = 0;
    controller.setReloadCallback(
        [&reloadCount](const QString& hubPathArg, QString* errorMessage) -> bool
        {
            Q_UNUSED(hubPathArg);
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            ++reloadCount;
            return true;
        });

    controller.setCurrentHubPath(hubPath);
    QVERIFY(
        writeUtf8File(
            QDir(hubPath).filePath(QStringLiteral(".whatson/write-lease.json")),
            QStringLiteral("{\"owner\":\"heartbeat\"}")));

    controller.requestSyncHint();

    QTest::qWait(50);
    QCOMPARE(reloadCount, 0);
    QCOMPARE(syncReloadedSpy.count(), 0);
}

void WhatSonHubSyncControllerTest::acknowledgeLocalMutation_refreshesBaselineWithoutReload()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Beta.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    QVERIFY(writeUtf8File(QDir(hubPath).filePath(QStringLiteral("index.wsnindex")), QStringLiteral("alpha")));

    WhatSonHubSyncController controller;
    controller.setDebounceIntervalMs(0);
    controller.setPeriodicIntervalMs(60000);

    QSignalSpy syncReloadedSpy(&controller, &WhatSonHubSyncController::syncReloaded);
    int reloadCount = 0;
    controller.setReloadCallback(
        [&reloadCount](const QString& hubPathArg, QString* errorMessage) -> bool
        {
            Q_UNUSED(hubPathArg);
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            ++reloadCount;
            return true;
        });

    controller.setCurrentHubPath(hubPath);

    QVERIFY(writeUtf8File(QDir(hubPath).filePath(QStringLiteral("index.wsnindex")), QStringLiteral("beta")));
    controller.acknowledgeLocalMutation();
    QTest::qWait(50);
    QCOMPARE(reloadCount, 0);

    controller.requestSyncHint();
    QTest::qWait(50);
    QCOMPARE(reloadCount, 0);
    QCOMPARE(syncReloadedSpy.count(), 0);
}

QTEST_MAIN(WhatSonHubSyncControllerTest)

#include "test_hub_sync_controller.moc"
