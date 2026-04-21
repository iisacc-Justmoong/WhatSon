#include "app/models/file/hierarchy/bookmarks/WhatSonBookmarksHierarchyStore.hpp"

#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

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

WhatSonBookmarksHierarchyStore::WhatSonBookmarksHierarchyStore()
{
    clear();
}

WhatSonBookmarksHierarchyStore::~WhatSonBookmarksHierarchyStore() = default;

void WhatSonBookmarksHierarchyStore::clear()
{
    m_hubPath.clear();
    m_bookmarkIds.clear();
    m_bookmarkColorCriteriaHex.clear();
    for (const WhatSon::Bookmarks::BookmarkColorDefinition& definition : WhatSon::Bookmarks::kBookmarkColorDefinitions)
    {
        m_bookmarkColorCriteriaHex.push_back(QString::fromLatin1(definition.hex));
    }
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.bookmarks.store"),
                              QStringLiteral("setBookmarkIds"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_bookmarkIds.size())
                              .arg(m_bookmarkIds.join(QStringLiteral(", "))));
}

QStringList WhatSonBookmarksHierarchyStore::bookmarkColorCriteriaHex() const
{
    return m_bookmarkColorCriteriaHex;
}

void WhatSonBookmarksHierarchyStore::setBookmarkColorCriteriaHex(QStringList values)
{
    const int rawCount = values.size();
    QStringList sanitized;
    sanitized.reserve(rawCount);

    for (const QString& value : values)
    {
        const QString colorHex = WhatSon::Bookmarks::bookmarkColorToHex(value);
        if (sanitized.contains(colorHex))
        {
            continue;
        }
        sanitized.push_back(colorHex);
    }

    if (sanitized.size() < static_cast<int>(WhatSon::Bookmarks::kBookmarkColorDefinitions.size()))
    {
        for (const WhatSon::Bookmarks::BookmarkColorDefinition& definition :
             WhatSon::Bookmarks::kBookmarkColorDefinitions)
        {
            const QString colorHex = QString::fromLatin1(definition.hex);
            if (sanitized.contains(colorHex))
            {
                continue;
            }
            sanitized.push_back(colorHex);
        }
    }

    m_bookmarkColorCriteriaHex = std::move(sanitized);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.bookmarks.store"),
                              QStringLiteral("setBookmarkColorCriteriaHex"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_bookmarkColorCriteriaHex.size())
                              .arg(m_bookmarkColorCriteriaHex.join(QStringLiteral(", "))));
}
