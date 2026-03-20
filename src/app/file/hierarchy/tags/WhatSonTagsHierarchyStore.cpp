#include "WhatSonTagsHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "hub/WhatSonHubWriteLease.hpp"
#include "WhatSonTagsHierarchyCreator.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <utility>

WhatSonTagsHierarchyStore::WhatSonTagsHierarchyStore() = default;

WhatSonTagsHierarchyStore::~WhatSonTagsHierarchyStore() = default;

void WhatSonTagsHierarchyStore::clear()
{
    m_hubPath.clear();
    m_tagEntries.clear();
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.tags.store"),
                              QStringLiteral("setTagEntries"),
                              QStringLiteral("count=%1").arg(m_tagEntries.size()));
}

bool WhatSonTagsHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Tags.wstags path is empty.");
        }
        return false;
    }

    WhatSonTagsHierarchyCreator creator;
    const QString text = creator.createText(*this);

    QString leaseError;
    if (!WhatSon::HubWriteLease::ensureWriteLeaseForPath(normalizedPath, &leaseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = leaseError;
        }
        return false;
    }

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
                              QStringLiteral("hierarchy.tags.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
