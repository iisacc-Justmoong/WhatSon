#include "WhatSonNoteBodyPersistence.hpp"

#include "WhatSonLocalNoteFileStore.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    QString normalizePath(QString path)
    {
        path = path.trimmed();
        if (path.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(path);
    }
} // namespace

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return text;
    }

    QString firstLineFromBodyPlainText(const QString& text)
    {
        const QString normalizedText = normalizeBodyPlainText(text);
        const QStringList lines = normalizedText.split(QLatin1Char('\n'));
        for (const QString& line : lines)
        {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
            {
                return trimmed;
            }
        }
        return {};
    }

    QString resolveBodyPath(const QString& noteDirectoryPath)
    {
        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemBodyPath = noteDir.filePath(noteStem + QStringLiteral(".wsnbody"));
            if (QFileInfo(stemBodyPath).isFile())
            {
                return QDir::cleanPath(stemBodyPath);
            }
        }

        const QString canonicalBodyPath = noteDir.filePath(QStringLiteral("note.wsnbody"));
        if (QFileInfo(canonicalBodyPath).isFile())
        {
            return QDir::cleanPath(canonicalBodyPath);
        }

        const QFileInfoList bodyCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnbody")},
            QDir::Files,
            QDir::Name);
        QString draftBodyPath;
        for (const QFileInfo& fileInfo : bodyCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftBodyPath.isEmpty())
                {
                    draftBodyPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftBodyPath.isEmpty())
        {
            return QDir::cleanPath(draftBodyPath);
        }

        if (!noteStem.isEmpty())
        {
            return QDir::cleanPath(noteDir.filePath(noteStem + QStringLiteral(".wsnbody")));
        }
        return QDir::cleanPath(noteDir.filePath(QStringLiteral("note.wsnbody")));
    }

    QString resolveHeaderPath(const QString& noteHeaderPath, const QString& noteDirectoryPath)
    {
        const QString directPath = normalizePath(noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDir.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDir.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        if (!noteStem.isEmpty())
        {
            return QDir::cleanPath(noteDir.filePath(noteStem + QStringLiteral(".wsnhead")));
        }
        return QDir::cleanPath(noteDir.filePath(QStringLiteral("note.wsnhead")));
    }

    bool persistBodyPlainText(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& noteHeaderPath,
        const QString& bodyPlainText,
        QString* outNormalizedBodyText,
        QString* outLastModifiedAt,
        QString* errorMessage)
    {
        WhatSonLocalNoteFileStore localNoteFileStore;
        WhatSonLocalNoteFileStore::ReadRequest readRequest;
        readRequest.noteId = noteId;
        readRequest.noteDirectoryPath = noteDirectoryPath;
        readRequest.noteHeaderPath = noteHeaderPath;

        WhatSonLocalNoteDocument document;
        QString readError;
        if (!localNoteFileStore.readNote(std::move(readRequest), &document, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        const QString normalizedBodyText = normalizeBodyPlainText(bodyPlainText);
        if (normalizedBodyText == document.bodyPlainText)
        {
            if (outNormalizedBodyText != nullptr)
            {
                *outNormalizedBodyText = normalizedBodyText;
            }
            if (outLastModifiedAt != nullptr)
            {
                *outLastModifiedAt = document.headerStore.lastModifiedAt();
            }
            return true;
        }

        document.bodyPlainText = normalizedBodyText;

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = document;
        updateRequest.persistHeader = true;
        updateRequest.persistBody = true;
        updateRequest.touchLastModified = true;

        QString updateError;
        if (!localNoteFileStore.updateNote(std::move(updateRequest), &document, &updateError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = updateError;
            }
            return false;
        }

        if (outNormalizedBodyText != nullptr)
        {
            *outNormalizedBodyText = document.bodyPlainText;
        }
        if (outLastModifiedAt != nullptr)
        {
            *outLastModifiedAt = document.headerStore.lastModifiedAt();
        }
        return true;
    }
} // namespace WhatSon::NoteBodyPersistence
