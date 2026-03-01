#include "viewmodel/bridge/WhatSonBackendBridge.hpp"

#include <QSignalSpy>
#include <QtTest>

class WhatSonBackendBridgeTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void publish_updatesStateAndEmitsBackendEvent();
    void request_emitsCommandRequested();
    void clear_resetsState();
};

void WhatSonBackendBridgeTest::publish_updatesStateAndEmitsBackendEvent()
{
    WhatSonBackendBridge bridge;
    QSignalSpy eventSpy(&bridge, &WhatSonBackendBridge::backendEvent);
    QSignalSpy countSpy(&bridge, &WhatSonBackendBridge::eventCountChanged);
    QSignalSpy lastSpy(&bridge, &WhatSonBackendBridge::lastEventChanged);

    bridge.publish(QStringLiteral("library"), QStringLiteral("load.success"), QVariantMap{
                       {QStringLiteral("ok"), true},
                       {QStringLiteral("itemCount"), 7}
                   });

    QCOMPARE(bridge.lastDomain(), QStringLiteral("library"));
    QCOMPARE(bridge.lastEventName(), QStringLiteral("load.success"));
    QCOMPARE(bridge.lastPayload().value(QStringLiteral("ok")).toBool(), true);
    QCOMPARE(bridge.eventCount(), 1);
    QCOMPARE(eventSpy.count(), 1);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(lastSpy.count(), 1);
}

void WhatSonBackendBridgeTest::request_emitsCommandRequested()
{
    WhatSonBackendBridge bridge;
    QSignalSpy commandSpy(&bridge, &WhatSonBackendBridge::commandRequested);

    bridge.request(QStringLiteral("hierarchy.select"), QVariantMap{
                       {QStringLiteral("domain"), QStringLiteral("library")},
                       {QStringLiteral("index"), 3}
                   });

    QCOMPARE(commandSpy.count(), 1);
    const QList<QVariant> args = commandSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("hierarchy.select"));
    QCOMPARE(args.at(1).toMap().value(QStringLiteral("index")).toInt(), 3);
}

void WhatSonBackendBridgeTest::clear_resetsState()
{
    WhatSonBackendBridge bridge;
    bridge.publish(QStringLiteral("domain"), QStringLiteral("event"), QVariantMap{
                       {QStringLiteral("x"), 1}
                   });
    QCOMPARE(bridge.eventCount(), 1);

    bridge.clear();
    QCOMPARE(bridge.eventCount(), 0);
    QVERIFY(bridge.lastDomain().isEmpty());
    QVERIFY(bridge.lastEventName().isEmpty());
    QVERIFY(bridge.lastPayload().isEmpty());
}

QTEST_APPLESS_MAIN(WhatSonBackendBridgeTest)

#include "test_backend_bridge.moc"
