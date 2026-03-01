#include "WhatSonTagsHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"

#include <utility>

WhatSonTagsHierarchyStore::WhatSonTagsHierarchyStore() = default;

WhatSonTagsHierarchyStore::~WhatSonTagsHierarchyStore() = default;

void WhatSonTagsHierarchyStore::clear()
{
    m_hubPath.clear();
    m_tagEntries.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.tags.store"),
        QStringLiteral("clear"));
}

QString WhatSonTagsHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonTagsHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.tags.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QVector<WhatSonTagDepthEntry> WhatSonTagsHierarchyStore::tagEntries() const
{
    return m_tagEntries;
}

void WhatSonTagsHierarchyStore::setTagEntries(QVector<WhatSonTagDepthEntry> tagEntries)
{
    m_tagEntries = std::move(tagEntries);
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.tags.store"),
        QStringLiteral("setTagEntries"),
        QStringLiteral("count=%1").arg(m_tagEntries.size()));
}
