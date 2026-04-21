#include "WhatSonResourcesHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonResourcePackageSupport.hpp"
#include "WhatSonResourcesHierarchyCreator.hpp"

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
            value = WhatSon::Resources::normalizePath(value.trimmed());
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.resources.store"),
                              QStringLiteral("setResourcePaths"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_resourcePaths.size())
                              .arg(m_resourcePaths.join(QStringLiteral(", "))));
}

bool WhatSonResourcesHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Resources.wsresources path is empty.");
        }
        return false;
    }

    WhatSonResourcesHierarchyCreator creator;
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
                              QStringLiteral("hierarchy.resources.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
