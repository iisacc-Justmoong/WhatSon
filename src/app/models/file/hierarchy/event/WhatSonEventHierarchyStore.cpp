#include "app/models/file/hierarchy/event/WhatSonEventHierarchyStore.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hierarchy/event/WhatSonEventHierarchyCreator.hpp"

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

WhatSonEventHierarchyStore::WhatSonEventHierarchyStore() = default;

WhatSonEventHierarchyStore::~WhatSonEventHierarchyStore() = default;

void WhatSonEventHierarchyStore::clear()
{
    m_hubPath.clear();
    m_eventNames.clear();
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.event.store"),
                              QStringLiteral("setEventNames"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_eventNames.size())
                              .arg(m_eventNames.join(QStringLiteral(", "))));
}

bool WhatSonEventHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Event.wsevent path is empty.");
        }
        return false;
    }

    WhatSonEventHierarchyCreator creator;
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
                              QStringLiteral("hierarchy.event.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
