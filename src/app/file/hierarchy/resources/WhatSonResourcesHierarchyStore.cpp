#include "WhatSonResourcesHierarchyStore.hpp"

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

WhatSonResourcesHierarchyStore::WhatSonResourcesHierarchyStore() = default;

WhatSonResourcesHierarchyStore::~WhatSonResourcesHierarchyStore() = default;

void WhatSonResourcesHierarchyStore::clear()
{
    m_hubPath.clear();
    m_resourcePaths.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.resources.store"),
        QStringLiteral("clear"));
}

QString WhatSonResourcesHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonResourcesHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.resources.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonResourcesHierarchyStore::resourcePaths() const
{
    return m_resourcePaths;
}

void WhatSonResourcesHierarchyStore::setResourcePaths(QStringList values)
{
    const int rawCount = values.size();
    m_resourcePaths = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.resources.store"),
        QStringLiteral("setResourcePaths"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_resourcePaths.size())
        .arg(m_resourcePaths.join(QStringLiteral(", "))));
}
