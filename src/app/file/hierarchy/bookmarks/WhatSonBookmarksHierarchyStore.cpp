#include "WhatSonBookmarksHierarchyStore.hpp"

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

WhatSonBookmarksHierarchyStore::WhatSonBookmarksHierarchyStore() = default;

WhatSonBookmarksHierarchyStore::~WhatSonBookmarksHierarchyStore() = default;

void WhatSonBookmarksHierarchyStore::clear()
{
    m_hubPath.clear();
    m_bookmarkIds.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.bookmarks.store"),
        QStringLiteral("clear"));
}

QString WhatSonBookmarksHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonBookmarksHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.bookmarks.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonBookmarksHierarchyStore::bookmarkIds() const
{
    return m_bookmarkIds;
}

void WhatSonBookmarksHierarchyStore::setBookmarkIds(QStringList values)
{
    const int rawCount = values.size();
    m_bookmarkIds = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.bookmarks.store"),
        QStringLiteral("setBookmarkIds"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_bookmarkIds.size())
        .arg(m_bookmarkIds.join(QStringLiteral(", "))));
}
