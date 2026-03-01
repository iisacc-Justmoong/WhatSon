#include "WhatSonProjectsHierarchyStore.hpp"

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

WhatSonProjectsHierarchyStore::WhatSonProjectsHierarchyStore() = default;

WhatSonProjectsHierarchyStore::~WhatSonProjectsHierarchyStore() = default;

void WhatSonProjectsHierarchyStore::clear()
{
    m_hubPath.clear();
    m_projectNames.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.projects.store"),
        QStringLiteral("clear"));
}

QString WhatSonProjectsHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonProjectsHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.projects.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonProjectsHierarchyStore::projectNames() const
{
    return m_projectNames;
}

void WhatSonProjectsHierarchyStore::setProjectNames(QStringList values)
{
    const int rawCount = values.size();
    m_projectNames = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.projects.store"),
        QStringLiteral("setProjectNames"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_projectNames.size())
        .arg(m_projectNames.join(QStringLiteral(", "))));
}
