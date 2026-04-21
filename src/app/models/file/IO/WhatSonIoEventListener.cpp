#include "app/models/file/IO/WhatSonIoEventListener.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDateTime>

#include <utility>

namespace
{
    QStringList sanitizePrefixes(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());

        for (QString& value : values)
        {
            value = value.trimmed();
            if (value.isEmpty())
            {
                continue;
            }

            if (!sanitized.contains(value, Qt::CaseInsensitive))
            {
                sanitized.push_back(value);
            }
        }

        return sanitized;
    }
} // namespace

WhatSonIoEventListener::WhatSonIoEventListener()
{
    setAcceptedEventPrefixes({QStringLiteral("io."), QStringLiteral("lvrs.io.")});
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.event.listener"),
                              QStringLiteral("ctor"),
                              QStringLiteral("prefixCount=%1").arg(m_acceptedEventPrefixes.size()));
}

WhatSonIoEventListener::~WhatSonIoEventListener() = default;

void WhatSonIoEventListener::clear()
{
    m_pendingEvents.clear();
    m_pendingHead = 0;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.event.listener"),
                              QStringLiteral("clear"));
}

bool WhatSonIoEventListener::enabled() const noexcept
{
    return m_enabled;
}

void WhatSonIoEventListener::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.event.listener"),
                              QStringLiteral("setEnabled"),
                              QStringLiteral("value=%1").arg(
                                  m_enabled ? QStringLiteral("true") : QStringLiteral("false")));
}

QStringList WhatSonIoEventListener::acceptedEventPrefixes() const
{
    return m_acceptedEventPrefixes;
}

void WhatSonIoEventListener::setAcceptedEventPrefixes(QStringList prefixes)
{
    m_acceptedEventPrefixes = sanitizePrefixes(std::move(prefixes));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.event.listener"),
                              QStringLiteral("setAcceptedEventPrefixes"),
                              QStringLiteral("count=%1 values=[%2]")
                              .arg(m_acceptedEventPrefixes.size())
                              .arg(m_acceptedEventPrefixes.join(QStringLiteral(", "))));
}

void WhatSonIoEventListener::pushLvrsEvent(QString eventName, QVariantMap payload)
{
    eventName = eventName.trimmed();
    if (eventName.isEmpty())
    {
        return;
    }

    if (!m_enabled)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.event.listener"),
                                  QStringLiteral("pushLvrsEvent.ignored"),
                                  QStringLiteral("reason=listenerDisabled name=%1").arg(eventName));
        return;
    }

    if (!acceptsEventName(eventName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.event.listener"),
                                  QStringLiteral("pushLvrsEvent.rejected"),
                                  QStringLiteral("name=%1").arg(eventName));
        return;
    }

    WhatSonIoEvent event;
    event.name = std::move(eventName);
    event.payload = std::move(payload);
    event.createdAtUtc = QDateTime::currentDateTimeUtc();

    m_pendingEvents.push_back(std::move(event));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.event.listener"),
                              QStringLiteral("pushLvrsEvent.accepted"),
                              QStringLiteral("pendingCount=%1").arg(pendingCount()));
}

bool WhatSonIoEventListener::hasPendingEvents() const noexcept
{
    return pendingCount() > 0;
}

int WhatSonIoEventListener::pendingCount() const noexcept
{
    return m_pendingEvents.size() - m_pendingHead;
}

WhatSonIoEvent WhatSonIoEventListener::takeNextEvent()
{
    if (!hasPendingEvents())
    {
        return {};
    }

    WhatSonIoEvent next = std::move(m_pendingEvents[m_pendingHead]);
    ++m_pendingHead;
    compactQueueIfNeeded();
    return next;
}

bool WhatSonIoEventListener::acceptsEventName(const QString& eventName) const
{
    if (m_acceptedEventPrefixes.isEmpty())
    {
        return true;
    }

    const QString normalized = eventName.trimmed().toCaseFolded();
    for (const QString& prefix : m_acceptedEventPrefixes)
    {
        const QString normalizedPrefix = prefix.trimmed().toCaseFolded();
        if (!normalizedPrefix.isEmpty() && normalized.startsWith(normalizedPrefix))
        {
            return true;
        }
    }

    return false;
}

void WhatSonIoEventListener::compactQueueIfNeeded()
{
    if (m_pendingHead <= 0)
    {
        return;
    }

    if (m_pendingHead >= m_pendingEvents.size())
    {
        m_pendingEvents.clear();
        m_pendingHead = 0;
        return;
    }

    constexpr int kCompactHeadThreshold = 256;
    if (m_pendingHead < kCompactHeadThreshold && m_pendingHead * 2 < m_pendingEvents.size())
    {
        return;
    }

    m_pendingEvents = m_pendingEvents.mid(m_pendingHead);
    m_pendingHead = 0;
}
