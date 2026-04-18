#include "WhatSonFoldersHierarchyStore.hpp"

#include "../WhatSonFolderIdentity.hpp"
#include "WhatSonDebugTrace.hpp"
#include "WhatSonFoldersHierarchyCreator.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <utility>

namespace
{
    QString sanitizeText(QString value)
    {
        return value.trimmed();
    }

    QVector<WhatSonFolderDepthEntry> sanitizeFolderEntries(QVector<WhatSonFolderDepthEntry> entries)
    {
        QVector<WhatSonFolderDepthEntry> sanitized;
        sanitized.reserve(entries.size());

        for (WhatSonFolderDepthEntry& entry : entries)
        {
            entry.id = WhatSon::NoteFolders::normalizeFolderPath(std::move(entry.id));
            entry.label = sanitizeText(std::move(entry.label));
            entry.uuid = WhatSon::FolderIdentity::normalizeFolderUuid(std::move(entry.uuid));
            if (entry.label.isEmpty() && !entry.id.isEmpty())
            {
                entry.label = WhatSon::NoteFolders::leafFolderName(entry.id);
            }
            if (entry.id.isEmpty() && !entry.label.isEmpty())
            {
                entry.id = WhatSon::NoteFolders::appendFolderPathSegment({}, entry.label);
            }
            if (entry.label.isEmpty() || entry.id.isEmpty())
            {
                continue;
            }
            if (entry.depth < 0)
            {
                entry.depth = 0;
            }
            if (entry.uuid.isEmpty())
            {
                entry.uuid = WhatSon::FolderIdentity::createFolderUuid();
            }
            sanitized.push_back(std::move(entry));
        }

        return sanitized;
    }
} // namespace

WhatSonFoldersHierarchyStore::WhatSonFoldersHierarchyStore() = default;

WhatSonFoldersHierarchyStore::~WhatSonFoldersHierarchyStore() = default;

void WhatSonFoldersHierarchyStore::clear()
{
    m_hubPath.clear();
    m_folderEntries.clear();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.store"),
                              QStringLiteral("clear"));
}

QString WhatSonFoldersHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonFoldersHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.store"),
                              QStringLiteral("setHubPath"),
                              QStringLiteral("value=%1").arg(m_hubPath));
}

QVector<WhatSonFolderDepthEntry> WhatSonFoldersHierarchyStore::folderEntries() const
{
    return m_folderEntries;
}

void WhatSonFoldersHierarchyStore::setFolderEntries(QVector<WhatSonFolderDepthEntry> entries)
{
    const int rawCount = entries.size();
    m_folderEntries = sanitizeFolderEntries(std::move(entries));

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.store"),
                              QStringLiteral("setFolderEntries"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2")
                              .arg(rawCount)
                              .arg(m_folderEntries.size()));
}

bool WhatSonFoldersHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Folders.wsfolders path is empty.");
        }
        return false;
    }

    WhatSonFoldersHierarchyCreator creator;
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
                              QStringLiteral("hierarchy.folders.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
