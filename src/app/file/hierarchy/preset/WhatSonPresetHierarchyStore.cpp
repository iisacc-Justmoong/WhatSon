#include "WhatSonPresetHierarchyStore.hpp"

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

WhatSonPresetHierarchyStore::WhatSonPresetHierarchyStore() = default;

WhatSonPresetHierarchyStore::~WhatSonPresetHierarchyStore() = default;

void WhatSonPresetHierarchyStore::clear()
{
    m_hubPath.clear();
    m_presetNames.clear();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.preset.store"),
        QStringLiteral("clear"));
}

QString WhatSonPresetHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonPresetHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.preset.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonPresetHierarchyStore::presetNames() const
{
    return m_presetNames;
}

void WhatSonPresetHierarchyStore::setPresetNames(QStringList values)
{
    const int rawCount = values.size();
    m_presetNames = sanitizeValues(std::move(values));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.preset.store"),
        QStringLiteral("setPresetNames"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_presetNames.size())
        .arg(m_presetNames.join(QStringLiteral(", "))));
}
