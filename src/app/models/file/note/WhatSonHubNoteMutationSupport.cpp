#include "app/models/file/note/WhatSonHubNoteMutationSupport.hpp"

#include "app/viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QSet>

namespace
{
    constexpr auto kNoteTimestampFormat = "yyyy-MM-dd-hh-mm-ss";

    QString randomAlphaNumericSegment(int length)
    {
        static const QString upperAlphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        static const QString lowerAlphabet = QStringLiteral("abcdefghijklmnopqrstuvwxyz");
        static const QString digits = QStringLiteral("0123456789");
        static const QString alphaNumericAlphabet = upperAlphabet + lowerAlphabet + digits;

        if (length <= 0)
        {
            return {};
        }

        QString segment;
        segment.reserve(length);
        segment.push_back(upperAlphabet.at(QRandomGenerator::global()->bounded(upperAlphabet.size())));
        if (length > 1)
        {
            segment.push_back(lowerAlphabet.at(QRandomGenerator::global()->bounded(lowerAlphabet.size())));
        }
        if (length > 2)
        {
            segment.push_back(digits.at(QRandomGenerator::global()->bounded(digits.size())));
        }
        for (int index = segment.size(); index < length; ++index)
        {
            segment.push_back(
                alphaNumericAlphabet.at(QRandomGenerator::global()->bounded(alphaNumericAlphabet.size())));
        }

        for (int index = segment.size() - 1; index > 0; --index)
        {
            const int swapIndex = QRandomGenerator::global()->bounded(index + 1);
            if (swapIndex != index)
            {
                const QChar currentValue = segment.at(index);
                segment[index] = segment.at(swapIndex);
                segment[swapIndex] = currentValue;
            }
        }

        return segment;
    }
} // namespace

QString WhatSon::NoteMutationSupport::currentNoteTimestamp()
{
    return QDateTime::currentDateTime().toString(QString::fromLatin1(kNoteTimestampFormat));
}

int WhatSon::NoteMutationSupport::indexOfNoteRecordById(
    const QVector<LibraryNoteRecord>& notes,
    const QString& noteId)
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

QString WhatSon::NoteMutationSupport::createUniqueNoteId(
    const QString& libraryPath,
    const QVector<LibraryNoteRecord>& existingNotes)
{
    QSet<QString> existingKeys;
    existingKeys.reserve(existingNotes.size());
    for (const LibraryNoteRecord& note : existingNotes)
    {
        const QString noteIdKey = note.noteId.trimmed().toCaseFolded();
        if (!noteIdKey.isEmpty())
        {
            existingKeys.insert(noteIdKey);
        }
    }

    const QDir libraryDir(libraryPath);
    for (int attempt = 0; attempt < 4096; ++attempt)
    {
        const QString candidate = randomAlphaNumericSegment(16) + QLatin1Char('-')
            + randomAlphaNumericSegment(16);
        const QString candidateKey = candidate.toCaseFolded();
        if (existingKeys.contains(candidateKey))
        {
            continue;
        }

        if (libraryDir.exists(candidate + QStringLiteral(".wsnote")))
        {
            continue;
        }

        return candidate;
    }

    return {};
}

bool WhatSon::NoteMutationSupport::ensureDirectoryPath(const QString& directoryPath, QString* errorMessage)
{
    if (directoryPath.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Directory path must not be empty.");
        }
        return false;
    }

    WhatSonSystemIoGateway ioGateway;
    return ioGateway.ensureDirectory(directoryPath, errorMessage);
}

bool WhatSon::NoteMutationSupport::writeUtf8File(
    const QString& filePath,
    const QString& text,
    QString* errorMessage)
{
    WhatSonSystemIoGateway ioGateway;
    return ioGateway.writeUtf8File(filePath, text, errorMessage);
}

bool WhatSon::NoteMutationSupport::removeFilePath(const QString& filePath, QString* errorMessage)
{
    WhatSonSystemIoGateway ioGateway;
    return ioGateway.removeFile(filePath, errorMessage);
}

bool WhatSon::NoteMutationSupport::removeDirectoryPath(const QString& directoryPath, QString* errorMessage)
{
    WhatSonSystemIoGateway ioGateway;
    return ioGateway.removeDirectoryRecursively(directoryPath, errorMessage);
}

QString WhatSon::NoteMutationSupport::resolveNoteHeaderPath(const LibraryNoteRecord& note)
{
    const QString directPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(note.noteHeaderPath);
    if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
    {
        return directPath;
    }

    const QString noteDirectoryPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(note.noteDirectoryPath);
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

void WhatSon::NoteMutationSupport::syncNoteRecordFromDocument(
    LibraryNoteRecord* note,
    const WhatSonLocalNoteDocument& document)
{
    if (note == nullptr)
    {
        return;
    }

    const LibraryNoteRecord updatedRecord = document.toLibraryNoteRecord();
    note->folders = updatedRecord.folders;
    note->folderUuids = updatedRecord.folderUuids;
    note->lastModifiedAt = updatedRecord.lastModifiedAt;
    note->noteHeaderPath = updatedRecord.noteHeaderPath;
    if (note->noteDirectoryPath.isEmpty())
    {
        note->noteDirectoryPath = updatedRecord.noteDirectoryPath;
    }
}
