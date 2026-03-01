#include "WhatSonBackendBridge.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <utility>

namespace
{
    constexpr auto kBridgeRequestId = "requestId";
    constexpr auto kBridgeCommand = "command";
    constexpr auto kBridgeOk = "ok";
}

WhatSonBackendBridge::WhatSonBackendBridge(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::trace(
        QStringLiteral("backend.bridge"),
        QStringLiteral("ctor"));
}

WhatSonBackendBridge::~WhatSonBackendBridge() = default;

QString WhatSonBackendBridge::lastDomain() const
{
    return m_lastDomain;
}

QString WhatSonBackendBridge::lastEventName() const
{
    return m_lastEventName;
}

QVariantMap WhatSonBackendBridge::lastPayload() const
{
    return m_lastPayload;
}

int WhatSonBackendBridge::eventCount() const noexcept
{
    return m_eventCount;
}

int WhatSonBackendBridge::commandRequestCount() const noexcept
{
    return m_commandRequestCount;
}

int WhatSonBackendBridge::commandHandledCount() const noexcept
{
    return m_commandHandledCount;
}

QString WhatSonBackendBridge::lastCommand() const
{
    return m_lastCommand;
}

int WhatSonBackendBridge::requestSequence() const noexcept
{
    return m_requestSequence;
}

void WhatSonBackendBridge::publish(QString domain, QString eventName, QVariantMap payload)
{
    domain = sanitizeToken(std::move(domain));
    eventName = sanitizeToken(std::move(eventName));
    if (domain.isEmpty())
    {
        domain = QStringLiteral("unknown");
    }
    if (eventName.isEmpty())
    {
        eventName = QStringLiteral("unknown");
    }

    m_lastDomain = domain;
    m_lastEventName = eventName;
    m_lastPayload = std::move(payload);
    ++m_eventCount;

    WhatSon::Debug::trace(
        QStringLiteral("backend.bridge"),
        QStringLiteral("publish"),
        QStringLiteral("domain=%1 event=%2 count=%3")
        .arg(m_lastDomain, m_lastEventName)
        .arg(m_eventCount));

    emit backendEvent(m_lastDomain, m_lastEventName, m_lastPayload);
    emit lastEventChanged();
    emit eventCountChanged();
}

void WhatSonBackendBridge::request(QString command, QVariantMap payload)
{
    command = sanitizeToken(std::move(command));
    if (command.isEmpty())
    {
        command = QStringLiteral("unknown");
    }
    ++m_requestSequence;
    ++m_commandRequestCount;
    m_lastCommand = command;
    payload.insert(QLatin1StringView(kBridgeRequestId), m_requestSequence);
    payload.insert(QLatin1StringView(kBridgeCommand), command);

    WhatSon::Debug::trace(
        QStringLiteral("backend.bridge"),
        QStringLiteral("request"),
        QStringLiteral("command=%1 requestId=%2").arg(command).arg(m_requestSequence));
    emit commandRequested(command, payload);
    emit commandMetricsChanged();
}

void WhatSonBackendBridge::ackCommand(QString command, bool ok, QVariantMap result)
{
    command = sanitizeToken(std::move(command));
    if (command.isEmpty())
    {
        command = QStringLiteral("unknown");
    }
    ++m_commandHandledCount;
    m_lastCommand = command;
    result.insert(QLatin1StringView(kBridgeCommand), command);
    result.insert(QLatin1StringView(kBridgeOk), ok);
    emit commandFinished(command, ok, result);
    emit commandMetricsChanged();

    publish(
        QStringLiteral("bridge"),
        ok ? QStringLiteral("command.handled") : QStringLiteral("command.failed"),
        result);
}

void WhatSonBackendBridge::clear()
{
    m_lastDomain.clear();
    m_lastEventName.clear();
    m_lastPayload.clear();
    m_eventCount = 0;
    m_commandRequestCount = 0;
    m_commandHandledCount = 0;
    m_lastCommand.clear();
    m_requestSequence = 0;
    emit lastEventChanged();
    emit eventCountChanged();
    emit commandMetricsChanged();
}

QString WhatSonBackendBridge::sanitizeToken(QString value) const
{
    return value.trimmed();
}
