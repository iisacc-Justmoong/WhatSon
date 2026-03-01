#include "WhatSonLibraryHierarchyStore.hpp"

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

WhatSonLibraryHierarchyStore::WhatSonLibraryHierarchyStore() = default;

WhatSonLibraryHierarchyStore::~WhatSonLibraryHierarchyStore() = default;

void WhatSonLibraryHierarchyStore::clear()
{
    m_hubPath.clear();
    m_noteIds.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.library.store"),
        QStringLiteral("clear"));
}

QString WhatSonLibraryHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonLibraryHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.library.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonLibraryHierarchyStore::noteIds() const
{
    return m_noteIds;
}

void WhatSonLibraryHierarchyStore::setNoteIds(QStringList values)
{
    const int rawCount = values.size();
    m_noteIds = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.library.store"),
        QStringLiteral("setNoteIds"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_noteIds.size())
        .arg(m_noteIds.join(QStringLiteral(", "))));
}
