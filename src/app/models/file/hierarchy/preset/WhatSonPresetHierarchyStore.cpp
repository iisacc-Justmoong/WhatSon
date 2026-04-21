#include "app/models/file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hierarchy/preset/WhatSonPresetHierarchyCreator.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.preset.store"),
                              QStringLiteral("setPresetNames"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_presetNames.size())
                              .arg(m_presetNames.join(QStringLiteral(", "))));
}

bool WhatSonPresetHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Preset.wspreset path is empty.");
        }
        return false;
    }

    WhatSonPresetHierarchyCreator creator;
    const QString text = creator.createText(*this);

    const QFileInfo info(normalizedPath);
    if (!QDir().mkpath(info.absolutePath()))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create hierarchy directory: %1").arg(info.absolutePath());
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open hierarchy file for write: %1").arg(normalizedPath);
        }
        return false;
    }

    file.write(text.toUtf8());
    file.close();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.preset.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
