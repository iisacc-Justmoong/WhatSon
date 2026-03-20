#include "WhatSonHubNoteDeletionService.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "hub/WhatSonHubWriteLease.hpp"

#include <QDateTime>
#include <QDir>
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
        normalizedLibraryPath = m_hubStructureValidator.resolvePrimaryLibraryPath(normalizedWshubPath, &resolveError);
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

    QString leaseError;
    if (!WhatSon::HubWriteLease::ensureWriteLeaseForPath(normalizedWshubPath, &leaseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = leaseError;
        }
        return false;
    }

    const LibraryNoteRecord removedNote = request.notes.at(noteIndex);
    const QString noteDirectoryPath = m_noteStorageValidator.resolveExistingNoteDirectoryPath(removedNote);
    const bool missingMaterializedDirectory = noteDirectoryPath.isEmpty() || !QFileInfo(noteDirectoryPath).isDir();
    QFileInfo noteDirectoryInfo(noteDirectoryPath);
    QDir noteParentDir(noteDirectoryInfo.absolutePath());
    const QString originalDirectoryName = noteDirectoryInfo.fileName();
    QString stagedDirectoryName;
    QString stagedNoteDirectoryPath;
    if (!missingMaterializedDirectory)
    {
        stagedDirectoryName = createStagedRemovalDirectoryName(noteParentDir, originalDirectoryName);
        stagedNoteDirectoryPath = noteParentDir.filePath(stagedDirectoryName);
        if (!noteParentDir.rename(originalDirectoryName, stagedDirectoryName))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to stage note directory for deletion: %1").
                    arg(noteDirectoryPath);
            }
            return false;
        }
    }
    else
    {
        WhatSon::Debug::trace(
            QStringLiteral("note.delete.service"),
            QStringLiteral("deleteNote.orphanIndexEntry"),
            QStringLiteral("noteId=%1 action=indexOnlyPrune").arg(normalizedNoteId));
    }

    auto restoreStagedDirectory = [&noteParentDir, &originalDirectoryName, &stagedDirectoryName,
            missingMaterializedDirectory]()
    {
        if (missingMaterializedDirectory)
        {
            return true;
        }
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

    if (!m_libraryIndexIntegrityValidator.rewriteIndexesFromRecords(
        normalizedWshubPath,
        QStringList{normalizedLibraryPath},
        request.notes,
        &ioError))
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
        normalizedStatPath = m_hubStructureValidator.resolveHubStatPath(normalizedWshubPath);
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
    if (!missingMaterializedDirectory && !m_ioGateway.removeDirectoryRecursively(stagedNoteDirectoryPath, &removeError))
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
            QStringLiteral("noteId=%1 library=%2 remaining=%3 materialized=%4")
            .arg(normalizedNoteId, normalizedLibraryPath)
            .arg(remainingCount)
            .arg(missingMaterializedDirectory ? QStringLiteral("false") : QStringLiteral("true")));
        return true;
    }

    WhatSon::Debug::trace(
        QStringLiteral("note.delete.service"),
        QStringLiteral("deleteNote.success"),
        QStringLiteral("noteId=%1 library=%2 remaining=%3 materialized=%4")
        .arg(normalizedNoteId, normalizedLibraryPath)
        .arg(request.notes.size())
        .arg(missingMaterializedDirectory ? QStringLiteral("false") : QStringLiteral("true")));
    return true;
}
