#include "IO/WhatSonIoEventListener.hpp"
#include "IO/WhatSonIoRuntimeController.hpp"
#include "IO/WhatSonSystemIoGateway.hpp"

#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

class WhatSonIoRuntimeTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void eventListener_filtersByPrefixAndQueuesEvents();
    void systemIoGateway_roundTripUtf8FileOperations();
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
