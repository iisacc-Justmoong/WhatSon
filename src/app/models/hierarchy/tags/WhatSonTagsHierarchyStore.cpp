#include "app/models/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyCreator.hpp"

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

    QString leafNameFromPath(const QString& path)
    {
        const QString normalized = path.trimmed();
        if (normalized.isEmpty())
        {
            return {};
        }

        const int slashIndex = normalized.lastIndexOf(QLatin1Char('/'));
        if (slashIndex < 0)
        {
            return normalized;
        }

        return normalized.mid(slashIndex + 1).trimmed();
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

    QVector<WhatSonTagDepthEntry> buildFlatEntries(const QStringList& tagNames)
    {
        QVector<WhatSonTagDepthEntry> entries;
        entries.reserve(tagNames.size());

        for (const QString& tagName : tagNames)
        {
            WhatSonTagDepthEntry entry;
            entry.id = tagName;
            entry.label = tagName;
            entry.depth = 0;
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    QVector<WhatSonTagDepthEntry> sanitizeTagEntries(QVector<WhatSonTagDepthEntry> entries)
    {
        QVector<WhatSonTagDepthEntry> sanitized;
        sanitized.reserve(entries.size());
        QStringList pathStack;

        for (WhatSonTagDepthEntry& entry : entries)
        {
            entry.id = sanitizeText(std::move(entry.id));
            entry.label = sanitizeText(std::move(entry.label));
            if (entry.label.isEmpty() && !entry.id.isEmpty())
            {
                entry.label = leafNameFromPath(entry.id);
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
            const int maxAllowedDepth = static_cast<int>(pathStack.size());
            if (entry.depth > maxAllowedDepth)
            {
                entry.depth = maxAllowedDepth;
            }
            while (pathStack.size() > entry.depth)
            {
                pathStack.removeLast();
            }

            const QString parentPath = (entry.depth > 0 && !pathStack.isEmpty())
                                           ? pathStack.constLast()
                                           : QString();
            if (!parentPath.isEmpty()
                && !entry.id.startsWith(parentPath + QLatin1Char('/'), Qt::CaseInsensitive))
            {
                const QString leafName = leafNameFromPath(entry.id);
                entry.id = parentPath + QLatin1Char('/') + (leafName.isEmpty() ? entry.label : leafName);
            }

            if (pathStack.size() <= entry.depth)
            {
                pathStack.push_back(entry.id);
            }
            else
            {
                pathStack[entry.depth] = entry.id;
                pathStack = pathStack.mid(0, entry.depth + 1);
            }
            sanitized.push_back(std::move(entry));
        }

        return sanitized;
    }

    QStringList extractTagNames(const QVector<WhatSonTagDepthEntry>& tagEntries)
    {
        QStringList names;
        names.reserve(tagEntries.size());

        QSet<QString> seen;
        for (const WhatSonTagDepthEntry& entry : tagEntries)
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

WhatSonTagsHierarchyStore::WhatSonTagsHierarchyStore() = default;

WhatSonTagsHierarchyStore::~WhatSonTagsHierarchyStore() = default;

void WhatSonTagsHierarchyStore::clear()
{
    m_hubPath.clear();
    m_tagNames.clear();
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

QStringList WhatSonTagsHierarchyStore::tagNames() const
{
    return m_tagNames;
}

void WhatSonTagsHierarchyStore::setTagNames(QStringList values)
{
    const int rawCount = values.size();
    m_tagNames = sanitizeValues(std::move(values));
    m_tagEntries = buildFlatEntries(m_tagNames);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.tags.store"),
                              QStringLiteral("setTagNames"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
                              .arg(rawCount)
                              .arg(m_tagNames.size())
                              .arg(m_tagNames.join(QStringLiteral(", "))));
}

QVector<WhatSonTagDepthEntry> WhatSonTagsHierarchyStore::tagEntries() const
{
    return m_tagEntries;
}

void WhatSonTagsHierarchyStore::setTagEntries(QVector<WhatSonTagDepthEntry> entries)
{
    const int rawCount = entries.size();
    m_tagEntries = sanitizeTagEntries(std::move(entries));
    m_tagNames = extractTagNames(m_tagEntries);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.tags.store"),
                              QStringLiteral("setTagEntries"),
                              QStringLiteral("rawCount=%1 sanitizedCount=%2 tagNames=%3")
                              .arg(rawCount)
                              .arg(m_tagEntries.size())
                              .arg(m_tagNames.size()));
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
