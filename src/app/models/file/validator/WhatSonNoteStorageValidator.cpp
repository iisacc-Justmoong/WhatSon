#include "app/models/file/validator/WhatSonNoteStorageValidator.hpp"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace
{
    constexpr auto kVersionSchema = "whatson.note.version.store";
    constexpr auto kDefaultProgressEnums = "{Ready,Pending,InProgress,Done}";

    QString createEmptyHeaderDocumentText(const QString& noteId)
    {
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<!DOCTYPE WHATSONNOTE>\n"
                   "<contents id=\"%1\">\n"
                   "  <head>\n"
                   "    <meta charset=\"UTF-8\" />\n"
                   "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                   "    <created></created>\n"
                   "    <author></author>\n"
                   "    <lastModified></lastModified>\n"
                   "    <lastOpened></lastOpened>\n"
                   "    <modifiedBy></modifiedBy>\n"
                   "    <folders>\n"
                   "    </folders>\n"
                   "    <project></project>\n"
                   "    <bookmarks state=\"false\" />\n"
                   "    <tags>\n"
                   "    </tags>\n"
                   "    <fileStat>\n"
                   "      <totalFolders>0</totalFolders>\n"
                   "      <totalTags>0</totalTags>\n"
                   "      <letterCount>0</letterCount>\n"
                   "      <wordCount>0</wordCount>\n"
                   "      <sentenceCount>0</sentenceCount>\n"
                   "      <paragraphCount>0</paragraphCount>\n"
                   "      <spaceCount>0</spaceCount>\n"
                   "      <indentCount>0</indentCount>\n"
                   "      <lineCount>0</lineCount>\n"
                   "      <openCount>0</openCount>\n"
                   "      <modifiedCount>0</modifiedCount>\n"
                   "      <backlinkToCount>0</backlinkToCount>\n"
                   "      <backlinkByCount>0</backlinkByCount>\n"
                   "      <includedResourceCount>0</includedResourceCount>\n"
                   "    </fileStat>\n"
                   "    <progress enums=\"%2\"></progress>\n"
                   "    <isPreset>false</isPreset>\n"
                   "  </head>\n"
                   "</contents>\n")
            .arg(noteId.trimmed(), QString::fromLatin1(kDefaultProgressEnums));
    }

    QString createEmptyBodyDocumentText(const QString& noteId)
    {
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<!DOCTYPE WHATSONNOTE>\n"
                   "<contents id=\"%1\">\n"
                   "  <body>\n"
                   "    <paragraph></paragraph>\n"
                   "  </body>\n"
                   "</contents>\n")
            .arg(noteId.trimmed());
    }

    QString createEmptyVersionDocumentText(const QString& noteId)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kVersionSchema));
        root.insert(QStringLiteral("noteId"), noteId.trimmed());
        root.insert(QStringLiteral("currentSnapshotId"), QString());
        root.insert(QStringLiteral("headSnapshotId"), QString());
        root.insert(QStringLiteral("snapshots"), QJsonArray{});
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    QString createEmptyPaintDocumentText(const QString& noteId)
    {
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<!DOCTYPE WHATSONNOTEPAINT>\n"
                   "<contents id=\"%1\">\n"
                   "  <body>\n"
                   "  </body>\n"
                   "</contents>\n")
            .arg(noteId.trimmed());
    }
}

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

bool WhatSonNoteStorageValidator::normalizeWsnotePackage(
    const LibraryNoteRecord& record,
    QString* errorMessage) const
{
    const QString noteDirectoryPath = resolveExistingNoteDirectoryPath(record);
    if (noteDirectoryPath.isEmpty())
    {
        return true;
    }

    return normalizeWsnotePackageByDirectoryPath(noteDirectoryPath, record.noteId, errorMessage);
}

bool WhatSonNoteStorageValidator::normalizeWsnotePackageByDirectoryPath(
    const QString& noteDirectoryPath,
    const QString& noteId,
    QString* errorMessage) const
{
    const QString normalizedDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return true;
    }

    const QFileInfo noteDirectoryInfo(normalizedDirectoryPath);
    if (!noteDirectoryInfo.isDir() || !noteDirectoryInfo.fileName().endsWith(QStringLiteral(".wsnote"), Qt::CaseInsensitive))
    {
        return true;
    }

    const QDir noteDir(normalizedDirectoryPath);
    const QString noteStem = noteDirectoryInfo.completeBaseName().trimmed();
    if (noteStem.isEmpty())
    {
        return true;
    }

    const QString targetHeaderName = noteStem + QStringLiteral(".wsnhead");
    const QString targetBodyName = noteStem + QStringLiteral(".wsnbody");
    const QString targetVersionName = noteStem + QStringLiteral(".wsnversion");
    const QString targetPaintName = noteStem + QStringLiteral(".wsnpaint");
    const QString targetHeaderPath = noteDir.filePath(targetHeaderName);
    const QString targetBodyPath = noteDir.filePath(targetBodyName);
    const QString targetVersionPath = noteDir.filePath(targetVersionName);
    const QString targetPaintPath = noteDir.filePath(targetPaintName);
    const QString normalizedNoteId = noteId.trimmed().isEmpty() ? noteStem : noteId.trimmed();

    const auto materializeBySuffix = [&](const QString& suffix, const QString& targetPath, const QString& fallbackText) -> bool
    {
        if (QFileInfo(targetPath).isFile())
        {
            return true;
        }

        const QStringList fileNames = m_ioGateway.listFileNames(normalizedDirectoryPath);
        for (const QString& fileName : fileNames)
        {
            if (!fileName.endsWith(suffix, Qt::CaseInsensitive))
            {
                continue;
            }

            const QString sourcePath = noteDir.filePath(fileName);
            if (QDir::cleanPath(sourcePath) == QDir::cleanPath(targetPath))
            {
                return true;
            }

            QString sourceText;
            if (!m_ioGateway.readUtf8File(sourcePath, &sourceText, errorMessage))
            {
                return false;
            }
            if (!m_ioGateway.writeUtf8File(targetPath, sourceText, errorMessage))
            {
                return false;
            }
            if (!m_ioGateway.removeFile(sourcePath, errorMessage))
            {
                return false;
            }
            return true;
        }

        if (fallbackText.isNull())
        {
            return true;
        }
        return m_ioGateway.writeUtf8File(targetPath, fallbackText, errorMessage);
    };

    if (!materializeBySuffix(QStringLiteral(".wsnhead"), targetHeaderPath, createEmptyHeaderDocumentText(normalizedNoteId)))
    {
        return false;
    }
    if (!materializeBySuffix(QStringLiteral(".wsnbody"), targetBodyPath, createEmptyBodyDocumentText(normalizedNoteId)))
    {
        return false;
    }
    if (!materializeBySuffix(QStringLiteral(".wsnversion"), targetVersionPath, createEmptyVersionDocumentText(normalizedNoteId)))
    {
        return false;
    }
    if (!materializeBySuffix(QStringLiteral(".wsnpaint"), targetPaintPath, createEmptyPaintDocumentText(normalizedNoteId)))
    {
        return false;
    }

    const QSet<QString> allowedFileNames{
        targetHeaderName.toCaseFolded(),
        targetBodyName.toCaseFolded(),
        targetVersionName.toCaseFolded(),
        targetPaintName.toCaseFolded()
    };

    const QFileInfoList allEntries = noteDir.entryInfoList(
        QStringList{},
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
        QDir::Name);
    for (const QFileInfo& entryInfo : allEntries)
    {
        const QString entryPath = entryInfo.absoluteFilePath();
        if (entryInfo.isSymLink())
        {
            if (!m_ioGateway.removeFile(entryPath, errorMessage))
            {
                return false;
            }
            continue;
        }

        if (entryInfo.isDir())
        {
            if (!m_ioGateway.removeDirectoryRecursively(entryPath, errorMessage))
            {
                return false;
            }
            continue;
        }

        if (allowedFileNames.contains(entryInfo.fileName().toCaseFolded()))
        {
            continue;
        }

        if (!m_ioGateway.removeFile(entryPath, errorMessage))
        {
            return false;
        }
    }

    return true;
}
