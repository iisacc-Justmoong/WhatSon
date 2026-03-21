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
    void refreshWriteLeaseForHub_allowsConcurrentSessions();
    void refreshWriteLeaseForHub_ignoresLegacyForeignLease();
    void ensureWriteLeaseForPath_allowsNestedPaths();
    void releaseWriteLeaseForHub_removesLegacyLeaseFile();
};

void WhatSonHubWriteLeaseTest::refreshWriteLeaseForHub_allowsConcurrentSessions()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(readLeaseJson(hubPath).isEmpty());
}

void WhatSonHubWriteLeaseTest::refreshWriteLeaseForHub_ignoresLegacyForeignLease()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));
    QVERIFY(writeLeaseJson(
        hubPath,
        foreignLeaseRecord(hubPath, QDateTime::currentDateTimeUtc().toString(Qt::ISODate))));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::refreshWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QJsonObject root = readLeaseJson(hubPath);
    QCOMPARE(root.value(QStringLiteral("ownerId")).toString(), QStringLiteral("foreign-session"));
}

void WhatSonHubWriteLeaseTest::ensureWriteLeaseForPath_allowsNestedPaths()
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
    QVERIFY(readLeaseJson(hubPath).isEmpty());
}

void WhatSonHubWriteLeaseTest::releaseWriteLeaseForHub_removesLegacyLeaseFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    QVERIFY(writeLeaseJson(
        hubPath,
        foreignLeaseRecord(hubPath, QDateTime::currentDateTimeUtc().toString(Qt::ISODate))));

    QVERIFY(QFileInfo::exists(leaseFilePathForHub(hubPath)));

    QString errorMessage;
    QVERIFY2(WhatSon::HubWriteLease::releaseWriteLeaseForHub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(!QFileInfo::exists(leaseFilePathForHub(hubPath)));
}

QTEST_APPLESS_MAIN(WhatSonHubWriteLeaseTest)

#include "test_whatson_hub_write_lease.moc"
