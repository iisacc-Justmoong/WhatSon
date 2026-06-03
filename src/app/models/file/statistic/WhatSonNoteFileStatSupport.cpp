#include "app/models/file/statistic/WhatSonNoteFileStatSupport.hpp"

#include "app/models/file/note/header/WhatSonNoteHeaderCreator.hpp"
#include "app/models/file/note/header/WhatSonNoteHeaderParser.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace
{
    void setError(QString* errorMessage, const QString& message)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = message;
        }
    }

    QString cleanExistingDirectoryPath(QString noteDirectoryPath)
    {
        noteDirectoryPath = QDir::cleanPath(noteDirectoryPath.trimmed());
        if (noteDirectoryPath == QStringLiteral(".") || !QFileInfo(noteDirectoryPath).isDir())
        {
            return {};
        }
        return noteDirectoryPath;
    }

    QString resolveHeaderPath(const QString& noteId, const QString& noteDirectoryPath)
    {
        const QString directoryPath = cleanExistingDirectoryPath(noteDirectoryPath);
        if (directoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDirectory(directoryPath);
        const QString normalizedNoteId = noteId.trimmed();
        if (!normalizedNoteId.isEmpty())
        {
            const QString noteIdHeaderPath = noteDirectory.filePath(normalizedNoteId + QStringLiteral(".wsnhead"));
            if (QFileInfo(noteIdHeaderPath).isFile())
            {
                return noteIdHeaderPath;
            }
        }

        const QString directoryStemHeaderPath =
            noteDirectory.filePath(QFileInfo(directoryPath).completeBaseName() + QStringLiteral(".wsnhead"));
        if (QFileInfo(directoryStemHeaderPath).isFile())
        {
            return directoryStemHeaderPath;
        }

        const QString canonicalHeaderPath = noteDirectory.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return canonicalHeaderPath;
        }

        const QStringList headerFiles = noteDirectory.entryList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files | QDir::Readable,
            QDir::Name);
        if (!headerFiles.isEmpty())
        {
            return noteDirectory.filePath(headerFiles.first());
        }

        return {};
    }

    bool readHeaderStore(
        const QString& noteId,
        const QString& noteDirectoryPath,
        WhatSonNoteHeaderStore* headerStore,
        QString* headerPath,
        QString* errorMessage)
    {
        if (headerStore == nullptr)
        {
            setError(errorMessage, QStringLiteral("A note header store is required."));
            return false;
        }

        const QString resolvedHeaderPath = resolveHeaderPath(noteId, noteDirectoryPath);
        if (resolvedHeaderPath.isEmpty())
        {
            setError(errorMessage, QStringLiteral("No .wsnhead file exists for the note."));
            return false;
        }

        QFile headerFile(resolvedHeaderPath);
        if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            setError(
                errorMessage,
                QStringLiteral("Failed to read note header: %1").arg(headerFile.errorString()));
            return false;
        }

        const QString headerText = QString::fromUtf8(headerFile.readAll());
        headerFile.close();

        WhatSonNoteHeaderParser parser;
        QString parseError;
        if (!parser.parse(headerText, headerStore, &parseError))
        {
            setError(errorMessage, parseError);
            return false;
        }

        if (headerPath != nullptr)
        {
            *headerPath = resolvedHeaderPath;
        }
        return true;
    }

    bool writeHeaderStore(
        const QString& headerPath,
        const WhatSonNoteHeaderStore& headerStore,
        QString* errorMessage)
    {
        if (headerPath.trimmed().isEmpty())
        {
            setError(errorMessage, QStringLiteral("A note header path is required."));
            return false;
        }

        WhatSonNoteHeaderCreator creator{QString(), QString()};
        QFile headerFile(headerPath);
        if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            setError(
                errorMessage,
                QStringLiteral("Failed to write note header: %1").arg(headerFile.errorString()));
            return false;
        }

        const QByteArray payload = creator.createHeaderText(headerStore).toUtf8();
        if (headerFile.write(payload) != payload.size())
        {
            setError(
                errorMessage,
                QStringLiteral("Failed to write complete note header: %1").arg(headerFile.errorString()));
            return false;
        }

        headerFile.close();
        return true;
    }
} // namespace

bool WhatSon::NoteFileStatSupport::incrementOpenCountForNoteHeader(
    const QString& noteId,
    const QString& noteDirectoryPath,
    QString* errorMessage)
{
    WhatSonNoteHeaderStore headerStore;
    QString headerPath;
    if (!readHeaderStore(noteId, noteDirectoryPath, &headerStore, &headerPath, errorMessage))
    {
        return false;
    }

    if (headerStore.noteId().trimmed().isEmpty())
    {
        headerStore.setNoteId(noteId.trimmed());
    }
    headerStore.incrementOpenCount();
    headerStore.setLastOpenedAt(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    return writeHeaderStore(headerPath, headerStore, errorMessage);
}

bool WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNote(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const bool incrementOpenCount,
    QString* errorMessage)
{
    if (incrementOpenCount)
    {
        return incrementOpenCountForNoteHeader(noteId, noteDirectoryPath, errorMessage);
    }

    WhatSonNoteHeaderStore headerStore;
    return readHeaderStore(noteId, noteDirectoryPath, &headerStore, nullptr, errorMessage);
}

bool WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNoteId(
    const QString& noteId,
    const QString& referenceNoteDirectoryPath,
    QString* errorMessage)
{
    return refreshTrackedStatisticsForNote(noteId, referenceNoteDirectoryPath, false, errorMessage);
}
