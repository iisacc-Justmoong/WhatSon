#include "WhatSonNoteStorageValidator.hpp"

#include <QDir>
#include <QFileInfo>

WhatSonNoteStorageValidator::WhatSonNoteStorageValidator() = default;

WhatSonNoteStorageValidator::~WhatSonNoteStorageValidator() = default;

QString WhatSonNoteStorageValidator::normalizePath(const QString& path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }

    return QDir::cleanPath(trimmed);
}

QString WhatSonNoteStorageValidator::resolveExistingNoteHeaderPath(const LibraryNoteRecord& record) const
{
    const QString directPath = normalizePath(record.noteHeaderPath);
    if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
    {
        return directPath;
    }

    const QString noteDirectoryPath = normalizePath(record.noteDirectoryPath);
    if (noteDirectoryPath.isEmpty())
    {
        return {};
    }

    const QDir noteDir(noteDirectoryPath);
    if (!noteDir.exists())
    {
        return {};
    }

    const QString noteStem = QFileInfo(noteDirectoryPath).completeBaseName().trimmed();
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

    return {};
}

QString WhatSonNoteStorageValidator::resolveExistingNoteDirectoryPath(const LibraryNoteRecord& record) const
{
    const QString directPath = normalizePath(record.noteDirectoryPath);
    if (!directPath.isEmpty() && QFileInfo(directPath).isDir())
    {
        return directPath;
    }

    const QString headerPath = resolveExistingNoteHeaderPath(record);
    if (!headerPath.isEmpty())
    {
        return QFileInfo(headerPath).absolutePath();
    }

    return {};
}

bool WhatSonNoteStorageValidator::hasMaterializedStorage(const LibraryNoteRecord& record) const
{
    return !resolveExistingNoteDirectoryPath(record).isEmpty()
        || !resolveExistingNoteHeaderPath(record).isEmpty();
}
