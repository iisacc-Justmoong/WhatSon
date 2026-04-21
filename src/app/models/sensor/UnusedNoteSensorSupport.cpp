#include "UnusedNoteSensorSupport.hpp"

#include "models/file/hub/WhatSonHubPathUtils.hpp"
#include "models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "models/file/note/WhatSonNoteHeaderParser.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QTimeZone>
#include <QVariantMap>

#include <algorithm>

namespace
{
    QString validatedHubPath(const QString& hubPath, QString* errorMessage)
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        const QFileInfo hubInfo(normalizedHubPath);
        if (!hubInfo.exists() || !hubInfo.isDir()
            || !hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub path is not an unpacked .wshub directory: %1").arg(
                    normalizedHubPath);
            }
            return {};
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return normalizedHubPath;
    }

    bool isSystemHubRootSegment(const QString& segment)
    {
        const QString folded = segment.trimmed().toCaseFolded();
        return folded.endsWith(QStringLiteral(".wscontents"))
            || folded.endsWith(QStringLiteral(".wsresources"));
    }

    bool shouldIgnoreHubPath(const QString& absolutePath, const QString& hubRootPath)
    {
        if (absolutePath.trimmed().isEmpty() || hubRootPath.trimmed().isEmpty())
        {
            return false;
        }

        const QString relativePath = QDir(hubRootPath).relativeFilePath(absolutePath);
        const QStringList segments = relativePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (int segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex)
        {
            const QString segment = segments.at(segmentIndex).trimmed();
            if (!segment.startsWith(QLatin1Char('.')))
            {
                continue;
            }
            if (segmentIndex == 0 && isSystemHubRootSegment(segment))
            {
                continue;
            }
            return true;
        }

        return false;
    }

    QString readUtf8File(const QString& filePath, QString* errorMessage)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file: %1").arg(filePath);
            }
            return {};
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return QString::fromUtf8(file.readAll());
    }

    QDateTime parseNoteTimestamp(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QDateTime isoWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoWithMs.isValid())
        {
            return isoWithMs.toUTC();
        }

        const QDateTime iso = QDateTime::fromString(trimmed, Qt::ISODate);
        if (iso.isValid())
        {
            return iso.toUTC();
        }

        static const QStringList dateTimeFormats{
            QStringLiteral("yyyy-MM-dd-HH-mm-ss"),
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")
        };

        for (const QString& format : dateTimeFormats)
        {
            const QDateTime parsed = QDateTime::fromString(trimmed, format);
            if (parsed.isValid())
            {
                return parsed.toUTC();
            }
        }

        static const QStringList dateFormats{
            QStringLiteral("yyyy-MM-dd"),
            QStringLiteral("yyyy/MM/dd")
        };

        for (const QString& format : dateFormats)
        {
            const QDate parsedDate = QDate::fromString(trimmed, format);
            if (parsedDate.isValid())
            {
                return QDateTime(parsedDate, QTime(0, 0), QTimeZone::UTC);
            }
        }

        return {};
    }

    struct EffectiveActivity final
    {
        QDateTime timestampUtc;
        QString source;
    };

    EffectiveActivity resolveEffectiveActivity(const WhatSonNoteHeaderStore& headerStore)
    {
        const QDateTime lastOpenedAt = parseNoteTimestamp(headerStore.lastOpenedAt());
        if (lastOpenedAt.isValid())
        {
            return {lastOpenedAt, QStringLiteral("lastOpenedAt")};
        }

        const QDateTime createdAt = parseNoteTimestamp(headerStore.createdAt());
        if (createdAt.isValid())
        {
            return {createdAt, QStringLiteral("createdAt")};
        }

        const QDateTime lastModifiedAt = parseNoteTimestamp(headerStore.lastModifiedAt());
        if (lastModifiedAt.isValid())
        {
            return {lastModifiedAt, QStringLiteral("lastModifiedAt")};
        }

        return {};
    }

    QVariantMap buildUnusedNoteEntry(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& headerPath,
        const WhatSonNoteHeaderStore& headerStore,
        const EffectiveActivity& activity,
        const QDateTime& referenceUtc,
        const QDateTime& cutoffUtc)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("noteId"), noteId);
        entry.insert(QStringLiteral("noteDirectoryPath"), noteDirectoryPath);
        entry.insert(QStringLiteral("headerPath"), headerPath);
        entry.insert(QStringLiteral("createdAt"), headerStore.createdAt());
        entry.insert(QStringLiteral("lastModifiedAt"), headerStore.lastModifiedAt());
        entry.insert(QStringLiteral("lastOpenedAt"), headerStore.lastOpenedAt());
        entry.insert(QStringLiteral("activityTimestampUtc"), activity.timestampUtc.toString(Qt::ISODate));
        entry.insert(QStringLiteral("activitySource"), activity.source);
        entry.insert(QStringLiteral("openCount"), headerStore.openCount());
        entry.insert(QStringLiteral("inactiveDays"), activity.timestampUtc.daysTo(referenceUtc));
        entry.insert(QStringLiteral("cutoffUtc"), cutoffUtc.toString(Qt::ISODate));
        return entry;
    }
} // namespace

QVariantList WhatSon::UnusedNoteSensorSupport::collectUnusedNoteEntries(
    const QString& hubPath,
    const QDateTime& referenceUtc,
    const QDateTime& cutoffUtc,
    QString* errorMessage)
{
    const QString normalizedHubPath = validatedHubPath(hubPath, errorMessage);
    if (normalizedHubPath.isEmpty())
    {
        return {};
    }

    QVariantList unusedNotes;
    QDirIterator iterator(
        normalizedHubPath,
        QStringList{QStringLiteral("*.wsnote")},
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden,
        QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const QString noteDirectoryPath = WhatSon::HubPath::normalizeAbsolutePath(iterator.next());
        if (shouldIgnoreHubPath(noteDirectoryPath, normalizedHubPath))
        {
            continue;
        }

        const QString headerPath = WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath);
        const QFileInfo headerInfo(headerPath);
        if (headerPath.isEmpty() || !headerInfo.isFile())
        {
            continue;
        }

        QString readError;
        const QString headerText = readUtf8File(headerPath, &readError);
        if (!readError.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return {};
        }

        WhatSonNoteHeaderStore headerStore;
        WhatSonNoteHeaderParser parser;
        QString parseError;
        if (!parser.parse(headerText, &headerStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return {};
        }

        const EffectiveActivity activity = resolveEffectiveActivity(headerStore);
        if (!activity.timestampUtc.isValid() || activity.timestampUtc > cutoffUtc)
        {
            continue;
        }

        QString noteId = headerStore.noteId().trimmed();
        if (noteId.isEmpty())
        {
            noteId = QFileInfo(noteDirectoryPath).completeBaseName().trimmed();
        }
        if (noteId.isEmpty())
        {
            continue;
        }

        unusedNotes.push_back(buildUnusedNoteEntry(
            noteId,
            noteDirectoryPath,
            headerPath,
            headerStore,
            activity,
            referenceUtc,
            cutoffUtc));
    }

    std::sort(
        unusedNotes.begin(),
        unusedNotes.end(),
        [](const QVariant& leftValue, const QVariant& rightValue)
        {
            const QString leftNoteId = leftValue.toMap().value(QStringLiteral("noteId")).toString();
            const QString rightNoteId = rightValue.toMap().value(QStringLiteral("noteId")).toString();
            return QString::compare(leftNoteId, rightNoteId, Qt::CaseInsensitive) < 0;
        });

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return unusedNotes;
}

QStringList WhatSon::UnusedNoteSensorSupport::noteIdsFromEntries(const QVariantList& unusedNotes)
{
    QStringList noteIds;
    noteIds.reserve(unusedNotes.size());
    for (const QVariant& entryValue : unusedNotes)
    {
        const QString noteId = entryValue.toMap().value(QStringLiteral("noteId")).toString().trimmed();
        if (!noteId.isEmpty())
        {
            noteIds.push_back(noteId);
        }
    }
    return noteIds;
}
