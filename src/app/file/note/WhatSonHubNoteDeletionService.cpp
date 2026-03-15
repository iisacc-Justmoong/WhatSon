#include "WhatSonHubNoteDeletionService.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/WhatSonLibraryHierarchyCreator.hpp"
#include "file/hierarchy/library/WhatSonLibraryHierarchyStore.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <utility>

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

    bool resolveContentsDirectories(
        const QString& wshubPath,
        QStringList* outContentsDirectories,
        QString* errorMessage = nullptr)
    {
        if (outContentsDirectories == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outContentsDirectories must not be null.");
            }
            return false;
        }

        outContentsDirectories->clear();

        const QString hubRootPath = normalizePath(wshubPath);
        if (hubRootPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must not be empty.");
            }
            return false;
        }

        const QFileInfo hubInfo(hubRootPath);
        if (!hubInfo.exists())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(hubRootPath);
            }
            return false;
        }

        if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")) || !hubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(hubRootPath);
            }
            return false;
        }

        const QDir hubDir(hubRootPath);
        const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents"));
        if (QFileInfo(fixedInternalPath).isDir())
        {
            outContentsDirectories->push_back(QDir::cleanPath(fixedInternalPath));
        }

        const QStringList dynamicContentsDirectories = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicContentsDirectories)
        {
            outContentsDirectories->push_back(QDir::cleanPath(hubDir.filePath(directoryName)));
        }

        outContentsDirectories->removeDuplicates();
        if (!outContentsDirectories->isEmpty())
        {
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No *.wscontents directory was found inside .wshub: %1").arg(hubRootPath);
        }
        return false;
    }

    QString resolvePrimaryLibraryPathFromWshub(
        const QString& wshubPath,
        QString* errorMessage = nullptr)
    {
        QStringList contentsDirectories;
        if (!resolveContentsDirectories(wshubPath, &contentsDirectories, errorMessage))
        {
            return {};
        }

        for (const QString& contentsDirectory : std::as_const(contentsDirectories))
        {
            const QString fixedLibraryPath = QDir(contentsDirectory).filePath(QStringLiteral("Library.wslibrary"));
            if (QFileInfo(fixedLibraryPath).isDir())
            {
                return QDir::cleanPath(fixedLibraryPath);
            }

            const QDir contentsDir(contentsDirectory);
            const QStringList dynamicLibraries = contentsDir.entryList(
                QStringList{QStringLiteral("*.wslibrary")},
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::Name);
            if (!dynamicLibraries.isEmpty())
            {
                return QDir::cleanPath(contentsDir.filePath(dynamicLibraries.first()));
            }
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No Library.wslibrary directory found inside: %1").arg(wshubPath);
        }
        return {};
    }

    QString resolveHubStatPathFromWshub(const QString& wshubPath)
    {
        const QString normalizedWshubPath = normalizePath(wshubPath);
        if (normalizedWshubPath.isEmpty())
        {
            return {};
        }

        const QDir hubDir(normalizedWshubPath);
        const QStringList statFiles = hubDir.entryList(
            QStringList{QStringLiteral("*.wsstat")},
            QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name);
        if (statFiles.isEmpty())
        {
            return {};
        }

        return QDir::cleanPath(hubDir.filePath(statFiles.first()));
    }

    QString resolveNoteHeaderPath(const LibraryNoteRecord& note)
    {
        const QString directPath = normalizePath(note.noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString noteDirectoryPath = normalizePath(note.noteDirectoryPath);
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

    QString resolveNoteDirectoryPath(const LibraryNoteRecord& note)
    {
        const QString directPath = normalizePath(note.noteDirectoryPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isDir())
        {
            return directPath;
        }

        const QString headerPath = resolveNoteHeaderPath(note);
        if (!headerPath.isEmpty())
        {
            return QFileInfo(headerPath).absolutePath();
        }

        return {};
    }

    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < notes.size(); ++index)
        {
            if (notes.at(index).noteId.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }

        return -1;
    }

    QStringList noteIdsFromRecords(const QVector<LibraryNoteRecord>& notes)
    {
        QStringList noteIds;
        noteIds.reserve(notes.size());

        for (const LibraryNoteRecord& note : notes)
        {
            const QString noteId = note.noteId.trimmed();
            if (!noteId.isEmpty())
            {
                noteIds.push_back(noteId);
            }
        }

        return noteIds;
    }

    QString createStagedRemovalDirectoryName(const QDir& parentDir, QString baseName)
    {
        baseName = baseName.trimmed();
        if (baseName.isEmpty())
        {
            baseName = QStringLiteral("note.wsnote");
        }

        QString candidate = QStringLiteral(".%1.deleting").arg(baseName);
        for (int suffix = 1; parentDir.exists(candidate); ++suffix)
        {
            candidate = QStringLiteral(".%1.deleting-%2").arg(baseName).arg(suffix);
        }
        return candidate;
    }
} // namespace

WhatSonHubNoteDeletionService::WhatSonHubNoteDeletionService() = default;

WhatSonHubNoteDeletionService::~WhatSonHubNoteDeletionService() = default;

bool WhatSonHubNoteDeletionService::deleteNote(Request request, Result* outResult, QString* errorMessage) const
{
    const QString normalizedNoteId = request.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId must not be empty.");
        }
        return false;
    }

    const int noteIndex = indexOfNoteRecordById(request.notes, normalizedNoteId);
    if (noteIndex < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId was not found in indexed notes: %1").arg(normalizedNoteId);
        }
        return false;
    }

    const QString normalizedWshubPath = normalizePath(request.wshubPath);
    QString normalizedLibraryPath = normalizePath(request.libraryPath);
    QString resolveError;
    if (normalizedLibraryPath.isEmpty() || !QFileInfo(normalizedLibraryPath).isDir())
    {
        normalizedLibraryPath = resolvePrimaryLibraryPathFromWshub(normalizedWshubPath, &resolveError);
    }
    if (normalizedLibraryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError.isEmpty()
                                ? QStringLiteral("Failed to resolve library path for note deletion.")
                                : resolveError;
        }
        return false;
    }

    const LibraryNoteRecord removedNote = request.notes.at(noteIndex);
    const QString noteDirectoryPath = resolveNoteDirectoryPath(removedNote);
    if (noteDirectoryPath.isEmpty() || !QFileInfo(noteDirectoryPath).isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve note directory for noteId=%1.").arg(normalizedNoteId);
        }
        return false;
    }

    const QFileInfo noteDirectoryInfo(noteDirectoryPath);
    QDir noteParentDir(noteDirectoryInfo.absolutePath());
    const QString originalDirectoryName = noteDirectoryInfo.fileName();
    const QString stagedDirectoryName = createStagedRemovalDirectoryName(noteParentDir, originalDirectoryName);
    const QString stagedNoteDirectoryPath = noteParentDir.filePath(stagedDirectoryName);
    if (!noteParentDir.rename(originalDirectoryName, stagedDirectoryName))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to stage note directory for deletion: %1").arg(noteDirectoryPath);
        }
        return false;
    }

    auto restoreStagedDirectory = [&noteParentDir, &originalDirectoryName, &stagedDirectoryName]()
    {
        if (!noteParentDir.exists(stagedDirectoryName) || noteParentDir.exists(originalDirectoryName))
        {
            return true;
        }
        return noteParentDir.rename(stagedDirectoryName, originalDirectoryName);
    };

    request.notes.removeAt(noteIndex);

    const QString indexPath = QDir(normalizedLibraryPath).filePath(QStringLiteral("index.wsnindex"));
    QString previousIndexText;
    const bool hadIndexFile = QFileInfo(indexPath).isFile();
    QString ioError;
    if (hadIndexFile && !m_ioGateway.readUtf8File(indexPath, &previousIndexText, &ioError))
    {
        restoreStagedDirectory();
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }

    WhatSonLibraryHierarchyStore libraryStore;
    libraryStore.setHubPath(normalizedWshubPath);
    libraryStore.setNoteIds(noteIdsFromRecords(request.notes));

    WhatSonLibraryHierarchyCreator libraryCreator;
    if (!m_ioGateway.writeUtf8File(indexPath, libraryCreator.createText(libraryStore), &ioError))
    {
        restoreStagedDirectory();
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }

    auto restoreIndexFile = [&]()
    {
        if (hadIndexFile)
        {
            m_ioGateway.writeUtf8File(indexPath, previousIndexText, nullptr);
            return;
        }
        m_ioGateway.removeFile(indexPath, nullptr);
    };

    QString normalizedStatPath = normalizePath(request.statPath);
    if (normalizedStatPath.isEmpty())
    {
        normalizedStatPath = resolveHubStatPathFromWshub(normalizedWshubPath);
    }

    const QString nowUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QString statCreatedAtUtc = request.hubStat.createdAtUtc().trimmed();
    QString previousStatText;
    const bool hadStatFile = !normalizedStatPath.isEmpty() && QFileInfo(normalizedStatPath).isFile();
    bool wroteStatFile = false;

    auto restoreStatFile = [&]()
    {
        if (normalizedStatPath.isEmpty())
        {
            return;
        }
        if (hadStatFile)
        {
            m_ioGateway.writeUtf8File(normalizedStatPath, previousStatText, nullptr);
            return;
        }
        if (wroteStatFile)
        {
            m_ioGateway.removeFile(normalizedStatPath, nullptr);
        }
    };

    if (!normalizedStatPath.isEmpty())
    {
        QJsonObject statRoot;
        if (hadStatFile)
        {
            QString rawStatText;
            if (!m_ioGateway.readUtf8File(normalizedStatPath, &rawStatText, &ioError))
            {
                restoreIndexFile();
                restoreStagedDirectory();
                if (errorMessage != nullptr)
                {
                    *errorMessage = ioError;
                }
                return false;
            }

            previousStatText = rawStatText;
            QJsonParseError statParseError;
            const QJsonDocument statDocument = QJsonDocument::fromJson(rawStatText.toUtf8(), &statParseError);
            if (statParseError.error == QJsonParseError::NoError && statDocument.isObject())
            {
                statRoot = statDocument.object();
            }
        }

        if (statCreatedAtUtc.isEmpty())
        {
            statCreatedAtUtc = statRoot.value(QStringLiteral("createdAtUtc")).toString().trimmed();
        }
        if (statCreatedAtUtc.isEmpty())
        {
            statCreatedAtUtc = nowUtc;
        }

        if (!statRoot.contains(QStringLiteral("version")))
        {
            statRoot.insert(QStringLiteral("version"), 1);
        }
        if (!statRoot.contains(QStringLiteral("schema")))
        {
            statRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.stat"));
        }
        if (!statRoot.contains(QStringLiteral("hub")) && !request.hubName.trimmed().isEmpty())
        {
            statRoot.insert(QStringLiteral("hub"), request.hubName.trimmed());
        }
        if (!statRoot.contains(QStringLiteral("participants")))
        {
            QJsonArray participants;
            for (const QString& participant : request.hubStat.participants())
            {
                participants.push_back(participant);
            }
            statRoot.insert(QStringLiteral("participants"), participants);
        }
        if (!statRoot.contains(QStringLiteral("profileLastModifiedAtUtc")))
        {
            statRoot.insert(
                QStringLiteral("profileLastModifiedAtUtc"),
                QJsonObject::fromVariantMap(request.hubStat.profileLastModifiedAtUtc()));
        }

        statRoot.insert(QStringLiteral("noteCount"), request.notes.size());
        statRoot.insert(QStringLiteral("resourceCount"), request.hubStat.resourceCount());
        statRoot.insert(QStringLiteral("characterCount"), request.hubStat.characterCount());
        statRoot.insert(QStringLiteral("createdAtUtc"), statCreatedAtUtc);
        statRoot.insert(QStringLiteral("lastModifiedAtUtc"), nowUtc);

        if (!m_ioGateway.writeUtf8File(
            normalizedStatPath,
            QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented)),
            &ioError))
        {
            restoreIndexFile();
            restoreStagedDirectory();
            if (errorMessage != nullptr)
            {
                *errorMessage = ioError;
            }
            return false;
        }

        wroteStatFile = true;
    }

    QString removeError;
    if (!m_ioGateway.removeDirectoryRecursively(stagedNoteDirectoryPath, &removeError))
    {
        restoreStatFile();
        restoreIndexFile();
        const bool restoredDirectory = restoreStagedDirectory();
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("%1 (directoryRestored=%2)").arg(
                removeError,
                restoredDirectory ? QStringLiteral("true") : QStringLiteral("false"));
        }
        return false;
    }

    WhatSonHubStat updatedHubStat = request.hubStat;
    updatedHubStat.setNoteCount(request.notes.size());
    updatedHubStat.setResourceCount(updatedHubStat.resourceCount());
    updatedHubStat.setCharacterCount(updatedHubStat.characterCount());
    if (updatedHubStat.createdAtUtc().trimmed().isEmpty())
    {
        updatedHubStat.setCreatedAtUtc(statCreatedAtUtc.isEmpty() ? nowUtc : statCreatedAtUtc);
    }
    updatedHubStat.setLastModifiedAtUtc(nowUtc);

    if (outResult != nullptr)
    {
        const int remainingCount = request.notes.size();
        outResult->noteId = normalizedNoteId;
        outResult->wshubPath = normalizedWshubPath;
        outResult->libraryPath = normalizedLibraryPath;
        outResult->statPath = normalizedStatPath;
        outResult->hubStat = std::move(updatedHubStat);
        outResult->remainingNotes = std::move(request.notes);
        WhatSon::Debug::trace(
            QStringLiteral("note.delete.service"),
            QStringLiteral("deleteNote.success"),
            QStringLiteral("noteId=%1 library=%2 remaining=%3")
            .arg(normalizedNoteId, normalizedLibraryPath)
            .arg(remainingCount));
        return true;
    }

    WhatSon::Debug::trace(
        QStringLiteral("note.delete.service"),
        QStringLiteral("deleteNote.success"),
        QStringLiteral("noteId=%1 library=%2 remaining=%3")
        .arg(normalizedNoteId, normalizedLibraryPath)
        .arg(request.notes.size()));
    return true;
}
