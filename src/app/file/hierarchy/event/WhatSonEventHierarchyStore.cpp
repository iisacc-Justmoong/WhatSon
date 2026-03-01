#include "WhatSonEventHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"

#include <utility>

namespace
{
    QStringList sanitizeValues(QStringList values)
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
            sanitized.push_back(value);
        }

        return sanitized;
    }
} // namespace

WhatSonEventHierarchyStore::WhatSonEventHierarchyStore() = default;

WhatSonEventHierarchyStore::~WhatSonEventHierarchyStore() = default;

void WhatSonEventHierarchyStore::clear()
{
    m_hubPath.clear();
    m_eventNames.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.event.store"),
        QStringLiteral("clear"));
}

QString WhatSonEventHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonEventHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.event.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonEventHierarchyStore::eventNames() const
{
    return m_eventNames;
}

void WhatSonEventHierarchyStore::setEventNames(QStringList values)
{
    const int rawCount = values.size();
    m_eventNames = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.event.store"),
        QStringLiteral("setEventNames"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_eventNames.size())
        .arg(m_eventNames.join(QStringLiteral(", "))));
}
