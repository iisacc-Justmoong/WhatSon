#include "WhatSonHubStat.hpp"

#include "WhatSonDebugTrace.hpp"

#include <algorithm>
#include <utility>

namespace
{
    QStringList sanitizeParticipants(QStringList values)
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
            if (!sanitized.contains(value))
            {
                sanitized.push_back(value);
            }
        }
        return sanitized;
    }

    QVariantMap sanitizeProfileMap(QVariantMap values)
    {
        QVariantMap sanitized;
        for (auto it = values.begin(); it != values.end(); ++it)
        {
            const QString key = it.key().trimmed();
            const QString value = it.value().toString().trimmed();
            if (key.isEmpty() || value.isEmpty())
            {
                continue;
            }
            sanitized.insert(key, value);
        }
        return sanitized;
    }
} // namespace

WhatSonHubStat::WhatSonHubStat() = default;

WhatSonHubStat::~WhatSonHubStat() = default;

void WhatSonHubStat::clear()
{
    m_noteCount = 0;
    m_resourceCount = 0;
    m_characterCount = 0;
    m_createdAtUtc.clear();
    m_lastModifiedAtUtc.clear();
    m_participants.clear();
    m_profileLastModifiedAtUtc.clear();

    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("clear"));
}

int WhatSonHubStat::noteCount() const noexcept
{
    return m_noteCount;
}

void WhatSonHubStat::setNoteCount(int value)
{
    m_noteCount = std::max(0, value);
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setNoteCount"),
        QStringLiteral("value=%1").arg(m_noteCount));
}

int WhatSonHubStat::resourceCount() const noexcept
{
    return m_resourceCount;
}

void WhatSonHubStat::setResourceCount(int value)
{
    m_resourceCount = std::max(0, value);
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setResourceCount"),
        QStringLiteral("value=%1").arg(m_resourceCount));
}

int WhatSonHubStat::characterCount() const noexcept
{
    return m_characterCount;
}

void WhatSonHubStat::setCharacterCount(int value)
{
    m_characterCount = std::max(0, value);
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setCharacterCount"),
        QStringLiteral("value=%1").arg(m_characterCount));
}

QString WhatSonHubStat::createdAtUtc() const
{
    return m_createdAtUtc;
}

void WhatSonHubStat::setCreatedAtUtc(QString value)
{
    m_createdAtUtc = value.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setCreatedAtUtc"),
        QStringLiteral("value=%1").arg(m_createdAtUtc));
}

QString WhatSonHubStat::lastModifiedAtUtc() const
{
    return m_lastModifiedAtUtc;
}

void WhatSonHubStat::setLastModifiedAtUtc(QString value)
{
    m_lastModifiedAtUtc = value.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setLastModifiedAtUtc"),
        QStringLiteral("value=%1").arg(m_lastModifiedAtUtc));
}

QStringList WhatSonHubStat::participants() const
{
    return m_participants;
}

void WhatSonHubStat::setParticipants(QStringList values)
{
    m_participants = sanitizeParticipants(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setParticipants"),
        QStringLiteral("count=%1 values=[%2]")
        .arg(m_participants.size())
        .arg(m_participants.join(QStringLiteral(", "))));
}

QVariantMap WhatSonHubStat::profileLastModifiedAtUtc() const
{
    return m_profileLastModifiedAtUtc;
}

void WhatSonHubStat::setProfileLastModifiedAtUtc(QVariantMap values)
{
    m_profileLastModifiedAtUtc = sanitizeProfileMap(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hub.stat"),
        QStringLiteral("setProfileLastModifiedAtUtc"),
        QStringLiteral("count=%1").arg(m_profileLastModifiedAtUtc.size()));
}
