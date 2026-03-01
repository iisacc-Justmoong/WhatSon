#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class WhatSonBackendBridge final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastDomain READ lastDomain NOTIFY lastEventChanged)
    Q_PROPERTY(QString lastEventName READ lastEventName NOTIFY lastEventChanged)
    Q_PROPERTY(QVariantMap lastPayload READ lastPayload NOTIFY lastEventChanged)
    Q_PROPERTY(int eventCount READ eventCount NOTIFY eventCountChanged)
    Q_PROPERTY(int commandRequestCount READ commandRequestCount NOTIFY commandMetricsChanged)
    Q_PROPERTY(int commandHandledCount READ commandHandledCount NOTIFY commandMetricsChanged)
    Q_PROPERTY(QString lastCommand READ lastCommand NOTIFY commandMetricsChanged)
    Q_PROPERTY(int requestSequence READ requestSequence NOTIFY commandMetricsChanged)

public:
    explicit WhatSonBackendBridge(QObject* parent = nullptr);
    ~WhatSonBackendBridge() override;

    QString lastDomain() const;
    QString lastEventName() const;
    QVariantMap lastPayload() const;
    int eventCount() const noexcept;
    int commandRequestCount() const noexcept;
    int commandHandledCount() const noexcept;
    QString lastCommand() const;
    int requestSequence() const noexcept;

    Q_INVOKABLE void publish(QString domain, QString eventName, QVariantMap payload = {});
    Q_INVOKABLE void request(QString command, QVariantMap payload = {});
    Q_INVOKABLE void ackCommand(QString command, bool ok, QVariantMap result = {});
    Q_INVOKABLE void clear();

public
    slots  :




    void requestBridgeHook()
    {
        emit bridgeHookRequested();
    }

    signals  :


    void backendEvent(const QString& domain, const QString& eventName, const QVariantMap& payload);
    void commandRequested(const QString& command, const QVariantMap& payload);
    void commandFinished(const QString& command, bool ok, const QVariantMap& result);
    void lastEventChanged();
    void eventCountChanged();
    void commandMetricsChanged();
    void bridgeHookRequested();

private:
    QString sanitizeToken(QString value) const;

    QString m_lastDomain;
    QString m_lastEventName;
    QVariantMap m_lastPayload;
    int m_eventCount = 0;
    int m_commandRequestCount = 0;
    int m_commandHandledCount = 0;
    QString m_lastCommand;
    int m_requestSequence = 0;
};
