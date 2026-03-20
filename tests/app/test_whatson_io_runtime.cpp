#include "IO/WhatSonIoEventListener.hpp"
#include "IO/WhatSonIoRuntimeController.hpp"
#include "IO/WhatSonSystemIoGateway.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

namespace
{
    bool writeForeignLease(const QString& hubPath)
    {
        const QString leasePath = QDir(hubPath).filePath(QStringLiteral(".whatson/write-lease.json"));
        if (!QDir().mkpath(QFileInfo(leasePath).absolutePath()))
        {
            return false;
        }

        const QString refreshedAtUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        const QJsonObject root{
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

        QFile file(leasePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }

        return file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) >= 0;
    }
} // namespace

class WhatSonIoRuntimeTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void eventListener_filtersByPrefixAndQueuesEvents();
    void systemIoGateway_roundTripUtf8FileOperations();
    void systemIoGateway_rejectsForeignWriteLease();
    void runtimeController_processesIoEventsEndToEnd();
};

void WhatSonIoRuntimeTest::eventListener_filtersByPrefixAndQueuesEvents()
{
    WhatSonIoEventListener listener;
    listener.setAcceptedEventPrefixes({QStringLiteral("io.")});

    listener.pushLvrsEvent(QStringLiteral("ui.clicked"), QVariantMap{
                               {QStringLiteral("path"), QStringLiteral("/tmp/ignored.txt")}
                           });
    QCOMPARE(listener.pendingCount(), 0);

    listener.pushLvrsEvent(QStringLiteral("io.writeUtf8"), QVariantMap{
                               {QStringLiteral("path"), QStringLiteral("/tmp/accepted.txt")},
                               {QStringLiteral("text"), QStringLiteral("payload")}
                           });
    QCOMPARE(listener.pendingCount(), 1);
    QVERIFY(listener.hasPendingEvents());

    const WhatSonIoEvent event = listener.takeNextEvent();
    QCOMPARE(event.name, QStringLiteral("io.writeUtf8"));
    QCOMPARE(event.payload.value(QStringLiteral("text")).toString(), QStringLiteral("payload"));
    QCOMPARE(listener.pendingCount(), 0);
}

void WhatSonIoRuntimeTest::systemIoGateway_roundTripUtf8FileOperations()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString nestedDirPath = QDir(tempDir.path()).filePath(QStringLiteral("nested"));
    const QString filePath = QDir(nestedDirPath).filePath(QStringLiteral("note.txt"));

    WhatSonSystemIoGateway gateway;
    QString errorMessage;

    QVERIFY2(gateway.ensureDirectory(nestedDirPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY2(gateway.writeUtf8File(filePath, QStringLiteral("Alpha"), &errorMessage), qPrintable(errorMessage));
    QVERIFY(gateway.exists(filePath));

    QString text;
    QVERIFY2(gateway.readUtf8File(filePath, &text, &errorMessage), qPrintable(errorMessage));
    QCOMPARE(text, QStringLiteral("Alpha"));

    QVERIFY2(gateway.appendUtf8File(filePath, QStringLiteral("Beta"), &errorMessage), qPrintable(errorMessage));
    QVERIFY2(gateway.readUtf8File(filePath, &text, &errorMessage), qPrintable(errorMessage));
    QCOMPARE(text, QStringLiteral("AlphaBeta"));

    const QStringList fileNames = gateway.listFileNames(nestedDirPath);
    QVERIFY(fileNames.contains(QStringLiteral("note.txt")));

    QVERIFY2(gateway.removeFile(filePath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(!gateway.exists(filePath));
}

void WhatSonIoRuntimeTest::systemIoGateway_rejectsForeignWriteLease()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Alpha.wshub"));
    const QString noteDirPath = QDir(hubPath).filePath(QStringLiteral("Alpha.wscontents/Library.wslibrary"));
    const QString filePath = QDir(noteDirPath).filePath(QStringLiteral("note.txt"));

    QVERIFY(QDir().mkpath(noteDirPath));
    QVERIFY(writeForeignLease(hubPath));

    WhatSonSystemIoGateway gateway;
    QString errorMessage;
    QVERIFY(!gateway.writeUtf8File(filePath, QStringLiteral("Blocked"), &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("currently locked for writing")));
    QVERIFY(!QFileInfo::exists(filePath));
}

void WhatSonIoRuntimeTest::runtimeController_processesIoEventsEndToEnd()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString filePath = QDir(tempDir.path()).filePath(QStringLiteral("runtime.txt"));

    WhatSonIoRuntimeController controller;
    controller.enqueueLvrsEvent(QStringLiteral("io.writeUtf8"), QVariantMap{
                                    {QStringLiteral("path"), filePath},
                                    {QStringLiteral("text"), QStringLiteral("RuntimePayload")}
                                });
    controller.enqueueLvrsEvent(QStringLiteral("io.readUtf8"), QVariantMap{
                                    {QStringLiteral("path"), filePath}
                                });

    QString errorMessage;
    QVERIFY2(controller.processNext(&errorMessage), qPrintable(errorMessage));
    const QVariantMap writeResult = controller.lastResult();
    QCOMPARE(writeResult.value(QStringLiteral("ok")).toBool(), true);
    QCOMPARE(writeResult.value(QStringLiteral("action")).toString(), QStringLiteral("io.writeUtf8"));
    QCOMPARE(writeResult.value(QStringLiteral("path")).toString(), filePath);

    QVERIFY2(controller.processNext(&errorMessage), qPrintable(errorMessage));
    const QVariantMap readResult = controller.lastResult();
    QCOMPARE(readResult.value(QStringLiteral("ok")).toBool(), true);
    QCOMPARE(readResult.value(QStringLiteral("action")).toString(), QStringLiteral("io.readUtf8"));
    QCOMPARE(readResult.value(QStringLiteral("text")).toString(), QStringLiteral("RuntimePayload"));

    controller.enqueueLvrsEvent(QStringLiteral("io.unsupported"), QVariantMap{});
    QVERIFY(!controller.processNext(&errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Unsupported IO action")));
}

QTEST_APPLESS_MAIN(WhatSonIoRuntimeTest)

#include "test_whatson_io_runtime.moc"
