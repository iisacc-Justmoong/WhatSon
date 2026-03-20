#include "hub/WhatSonHubWriteLease.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

namespace
{
    QString leaseFilePathForHub(const QString& hubPath)
    {
        return QDir(hubPath).filePath(QStringLiteral(".whatson/write-lease.json"));
    }

    bool writeLeaseJson(const QString& hubPath, const QJsonObject& root)
    {
        const QString leasePath = leaseFilePathForHub(hubPath);
        if (!QDir().mkpath(QFileInfo(leasePath).absolutePath()))
        {
            return false;
        }

        QFile file(leasePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }

        return file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) >= 0;
    }

    QJsonObject readLeaseJson(const QString& hubPath)
    {
        QFile file(leaseFilePathForHub(hubPath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        return document.isObject() ? document.object() : QJsonObject{};
    }

    QJsonObject foreignLeaseRecord(const QString& hubPath, const QString& refreshedAtUtc)
    {
        return QJsonObject{
            {QStringLiteral("version"), 1},
            {QStringLiteral("schema"), QStringLiteral("whatson.hub.writeLease")},
            {QStringLiteral("hubPath"), hubPath},
            {QStringLiteral("ownerId"), QStringLiteral("foreign-session")},
            {QStringLiteral("ownerName"), QStringLiteral("Desktop@foreign(pid=42)")},
            {QStringLiteral("hostName"), QStringLiteral("foreign-host")},
            {QStringLiteral("pid"), 42},
            {QStringLiteral("acquiredAtUtc"), refreshedAtUtc},
            {QStringLiteral("refreshedAtUtc"), refreshedAtUtc},
        };
    }
} // namespace

class WhatSonHubWriteLeaseTest final : public QObject
{
    Q_OBJECT

private slots:
    void refreshWriteLeaseForHub_createsCurrentSessionLease();
    void refreshWriteLeaseForHub_rejectsActiveForeignLease();
    void refreshWriteLeaseForHub_reclaimsStaleForeignLease();
    void ensureWriteLeaseForPath_resolvesNestedPaths();
    void releaseWriteLeaseForHub_removesOwnedLeaseFile();
};

void WhatSonHubWriteLeaseTest::refreshWriteLeaseForHub_createsCurrentSessionLease()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QJsonObject root = readLeaseJson(hubPath);
    QCOMPARE(root.value(QStringLiteral("schema")).toString(), QStringLiteral("whatson.hub.writeLease"));
    QCOMPARE(root.value(QStringLiteral("hubPath")).toString(), hubPath);
    QCOMPARE(root.value(QStringLiteral("ownerId")).toString(), WhatSon::HubWriteLease::currentSessionOwnerId());
    QVERIFY(!root.value(QStringLiteral("refreshedAtUtc")).toString().trimmed().isEmpty());
}

void WhatSonHubWriteLeaseTest::refreshWriteLeaseForHub_rejectsActiveForeignLease()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    QVERIFY(writeLeaseJson(
        hubPath,
        foreignLeaseRecord(hubPath, QDateTime::currentDateTimeUtc().toString(Qt::ISODate))));

    QString errorMessage;
    QVERIFY(!WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("currently locked for writing")));

    const QJsonObject root = readLeaseJson(hubPath);
    QCOMPARE(root.value(QStringLiteral("ownerId")).toString(), QStringLiteral("foreign-session"));
}

void WhatSonHubWriteLeaseTest::refreshWriteLeaseForHub_reclaimsStaleForeignLease()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    const QString staleTimestamp = QDateTime::currentDateTimeUtc().addSecs(-120).toString(Qt::ISODate);
    QVERIFY(writeLeaseJson(hubPath, foreignLeaseRecord(hubPath, staleTimestamp)));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QJsonObject root = readLeaseJson(hubPath);
    QCOMPARE(root.value(QStringLiteral("ownerId")).toString(), WhatSon::HubWriteLease::currentSessionOwnerId());
}

void WhatSonHubWriteLeaseTest::ensureWriteLeaseForPath_resolvesNestedPaths()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    const QString nestedDir = QDir(hubPath).filePath(QStringLiteral("Alpha.wscontents/Library.wslibrary/Note.wsnote"));
    QVERIFY(QDir().mkpath(nestedDir));

    QString errorMessage;
    QVERIFY2(
        WhatSon::HubWriteLease::ensureWriteLeaseForPath(
            QDir(nestedDir).filePath(QStringLiteral("note.wsnbody")),
            &errorMessage),
        qPrintable(errorMessage));

    const QJsonObject root = readLeaseJson(hubPath);
    QCOMPARE(root.value(QStringLiteral("ownerId")).toString(), WhatSon::HubWriteLease::currentSessionOwnerId());
}

void WhatSonHubWriteLeaseTest::releaseWriteLeaseForHub_removesOwnedLeaseFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(QFileInfo::exists(leaseFilePathForHub(hubPath)));

    QVERIFY2(WhatSon::HubWriteLease::releaseWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(!QFileInfo::exists(leaseFilePathForHub(hubPath)));
}

QTEST_APPLESS_MAIN(WhatSonHubWriteLeaseTest)

#include "test_whatson_hub_write_lease.moc"
