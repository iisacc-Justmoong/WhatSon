#include "WhatSonProjectsHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonProjectsHierarchyCreator.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>

#include <utility>

namespace
{
    QString sanitizeText(QString value)
    {
        return value.trimmed();
    }

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

    QVector<WhatSonFolderDepthEntry> buildFlatEntries(const QStringList& projectNames)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(projectNames.size());

        for (const QString& projectName : projectNames)
        {
            WhatSonFolderDepthEntry entry;
            entry.id = projectName;
            entry.label = projectName;
            entry.depth = 0;
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    QVector<WhatSonFolderDepthEntry> sanitizeFolderEntries(QVector<WhatSonFolderDepthEntry> entries)
    {
        QVector<WhatSonFolderDepthEntry> sanitized;
        sanitized.reserve(entries.size());

        for (WhatSonFolderDepthEntry& entry : entries)
        {
            entry.id = sanitizeText(std::move(entry.id));
            entry.label = sanitizeText(std::move(entry.label));
            if (entry.label.isEmpty() && !entry.id.isEmpty())
            {
                entry.label = entry.id;
            }
            if (entry.id.isEmpty() && !entry.label.isEmpty())
            {
                entry.id = entry.label;
            }
            if (entry.label.isEmpty() || entry.id.isEmpty())
            {
                continue;
            }
            if (entry.depth < 0)
            {
                entry.depth = 0;
            }
            sanitized.push_back(std::move(entry));
        }

        return sanitized;
    }

    QStringList extractProjectNames(const QVector<WhatSonFolderDepthEntry>& folderEntries)
    {
        QStringList names;
        names.reserve(folderEntries.size());

        QSet<QString> seen;
        for (const WhatSonFolderDepthEntry& entry : folderEntries)
        {
            const QString label = entry.label.trimmed();
            if (label.isEmpty() || seen.contains(label))
            {
                continue;
            }
            seen.insert(label);
            names.push_back(label);
        }

        return names;
    }
} // namespace

WhatSonProjectsHierarchyStore::WhatSonProjectsHierarchyStore() = default;

WhatSonProjectsHierarchyStore::~WhatSonProjectsHierarchyStore() = default;

void WhatSonProjectsHierarchyStore::clear()
{
    m_hubPath.clear();
    m_projectNames.clear();
    m_folderEntries.clear();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.store"),
                              QStringLiteral("clear"));
}

QString WhatSonProjectsHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonProjectsHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.store"),
                              QStringLiteral("setHubPath"),
                              QStringLiteral("value=%1").arg(m_hubPath));
}

QStringList WhatSonProjectsHierarchyStore::projectNames() const
{
    return m_projectNames;
}

void WhatSonProjectsHierarchyStore::setProjectNames(QStringList values)
{
    const int rawCount = values.size();
    m_projectNames = sanitizeValues(std::move(values));
    m_folderEntries = buildFlatEntries(m_projectNames);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.store"),
                              QStringLiteral("setProjectNames"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_projectNames.size())
                              .arg(m_projectNames.join(QStringLiteral(", "))));
}

QVector<WhatSonFolderDepthEntry> WhatSonProjectsHierarchyStore::folderEntries() const
{
    return m_folderEntries;
}

void WhatSonProjectsHierarchyStore::setFolderEntries(QVector<WhatSonFolderDepthEntry> entries)
{
    const int rawCount = entries.size();
    m_folderEntries = sanitizeFolderEntries(std::move(entries));
    m_projectNames = extractProjectNames(m_folderEntries);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.store"),
                              QStringLiteral("setFolderEntries"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 projectNames=%3")
                              .arg(rawCount)
                              .arg(m_folderEntries.size())
                              .arg(m_projectNames.size()));
}

bool WhatSonProjectsHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
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

    WhatSonProjectsHierarchyCreator creator;
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
                              QStringLiteral("hierarchy.projects.store"),
                              QStringLiteral("writeToFile"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
