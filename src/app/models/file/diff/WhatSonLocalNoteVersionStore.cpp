#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

#include "app/models/file/diff/VersionLimitManager.h"
#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>

#include <utility>

namespace
{
    constexpr auto kVersionSchema = "whatson.note.version.store";

    QString readUtf8File(const QString& path, QString* errorMessage)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file for reading: %1").arg(path);
            }
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    bool writeUtf8File(const QString& path, const QString& text, QString* errorMessage)
    {
        const QFileInfo fileInfo(path);
        if (fileInfo.absolutePath().trimmed().isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to resolve parent directory for: %1").arg(path);
            }
            return false;
        }

        QDir parentDirectory(fileInfo.absolutePath());
        if (!parentDirectory.exists() && !QDir().mkpath(fileInfo.absolutePath()))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to create parent directory for: %1").arg(path);
            }
            return false;
        }

        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file for writing: %1").arg(path);
            }
            return false;
        }
        file.write(text.toUtf8());
        if (!file.commit())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to commit file write: %1").arg(path);
            }
            return false;
        }
        return true;
    }

    QString noteIdFromDocument(const WhatSonLocalNoteDocument& document)
    {
        QString noteId = document.headerStore.noteId().trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        noteId = QFileInfo(document.noteDirectoryPath).completeBaseName().trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        return QStringLiteral("note");
    }

    QString noteStemFromDocument(const WhatSonLocalNoteDocument& document)
    {
        QString stem = QFileInfo(document.noteDirectoryPath).completeBaseName().trimmed();
        if (stem.isEmpty())
        {
            stem = QFileInfo(noteIdFromDocument(document)).completeBaseName().trimmed();
        }
        return stem.isEmpty() ? QStringLiteral("note") : stem;
    }

    QString versionPathFromDocument(const WhatSonLocalNoteDocument& document)
    {
        const QString directPath = QDir::cleanPath(document.noteVersionPath.trimmed());
        if (!directPath.isEmpty())
        {
            return directPath;
        }
        const QString noteDirectoryPath = QDir::cleanPath(document.noteDirectoryPath.trimmed());
        if (noteDirectoryPath.isEmpty())
        {
            return {};
        }
        return QDir(noteDirectoryPath).filePath(noteStemFromDocument(document) + QStringLiteral(".wsnversion"));
    }

    QString headerPathFromDocument(const WhatSonLocalNoteDocument& document)
    {
        const QString directPath = QDir::cleanPath(document.noteHeaderPath.trimmed());
        if (!directPath.isEmpty())
        {
            return directPath;
        }
        const QString noteDirectoryPath = QDir::cleanPath(document.noteDirectoryPath.trimmed());
        if (noteDirectoryPath.isEmpty())
        {
            return {};
        }
        return QDir(noteDirectoryPath).filePath(noteStemFromDocument(document) + QStringLiteral(".wsnhead"));
    }

    QString bodyPathFromDocument(const WhatSonLocalNoteDocument& document)
    {
        const QString directPath = QDir::cleanPath(document.noteBodyPath.trimmed());
        if (!directPath.isEmpty())
        {
            return directPath;
        }
        const QString noteDirectoryPath = QDir::cleanPath(document.noteDirectoryPath.trimmed());
        if (noteDirectoryPath.isEmpty())
        {
            return {};
        }
        return QDir(noteDirectoryPath).filePath(noteStemFromDocument(document) + QStringLiteral(".wsnbody"));
    }

    QJsonObject emptyVersionRoot(const QString& noteId)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kVersionSchema));
        root.insert(QStringLiteral("noteId"), noteId.trimmed());
        root.insert(QStringLiteral("currentSnapshotId"), QString());
        root.insert(QStringLiteral("headSnapshotId"), QString());
        root.insert(QStringLiteral("snapshots"), QJsonArray{});
        return root;
    }

    bool ensureVersionDocument(const QString& versionFilePath, const QString& noteId, QString* errorMessage)
    {
        if (versionFilePath.trimmed().isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("versionFilePath must not be empty.");
            }
            return false;
        }
        if (QFileInfo(versionFilePath).isFile())
        {
            return true;
        }
        return writeUtf8File(
            versionFilePath,
            QString::fromUtf8(QJsonDocument(emptyVersionRoot(noteId)).toJson(QJsonDocument::Indented)),
            errorMessage);
    }

    QStringList linesForPatch(const QString& text)
    {
        if (text.isEmpty())
        {
            return {};
        }
        QString normalized = text;
        if (normalized.endsWith(QLatin1Char('\n')))
        {
            normalized.chop(1);
        }
        return normalized.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    }

    QString unifiedPatch(const QString& label, const QString& before, const QString& after)
    {
        const QStringList beforeLines = linesForPatch(before);
        const QStringList afterLines = linesForPatch(after);

        QString patch;
        patch += QStringLiteral("--- a/%1\n").arg(label);
        patch += QStringLiteral("+++ b/%1\n").arg(label);
        patch += QStringLiteral("@@ -1,%1 +1,%2 @@\n")
                     .arg(QString::number(beforeLines.size()), QString::number(afterLines.size()));
        for (const QString& line : beforeLines)
        {
            patch += QLatin1Char('-') + line + QLatin1Char('\n');
        }
        for (const QString& line : afterLines)
        {
            patch += QLatin1Char('+') + line + QLatin1Char('\n');
        }
        return patch;
    }

    WhatSonNoteVersionDiffSegment diffSegment(
        const QString& before,
        const QString& after,
        const QString& label)
    {
        WhatSonNoteVersionDiffSegment segment;
        const int prefixLimit = qMin(before.size(), after.size());
        while (segment.prefixLength < prefixLimit
               && before.at(segment.prefixLength) == after.at(segment.prefixLength))
        {
            ++segment.prefixLength;
        }

        const int suffixLimit = qMin(before.size(), after.size()) - segment.prefixLength;
        while (segment.suffixLength < suffixLimit
               && before.at(before.size() - 1 - segment.suffixLength)
                      == after.at(after.size() - 1 - segment.suffixLength))
        {
            ++segment.suffixLength;
        }

        const int removedLength = qMax(0, before.size() - segment.prefixLength - segment.suffixLength);
        const int insertedLength = qMax(0, after.size() - segment.prefixLength - segment.suffixLength);
        segment.removedText = before.mid(segment.prefixLength, removedLength);
        segment.insertedText = after.mid(segment.prefixLength, insertedLength);
        segment.unifiedPatch = unifiedPatch(label, before, after);
        return segment;
    }

    QJsonObject diffSegmentToJson(const WhatSonNoteVersionDiffSegment& segment)
    {
        QJsonObject object;
        object.insert(QStringLiteral("prefixLength"), segment.prefixLength);
        object.insert(QStringLiteral("suffixLength"), segment.suffixLength);
        object.insert(QStringLiteral("removedText"), segment.removedText);
        object.insert(QStringLiteral("insertedText"), segment.insertedText);
        object.insert(QStringLiteral("unifiedPatch"), segment.unifiedPatch);
        return object;
    }

    WhatSonNoteVersionDiffSegment diffSegmentFromJson(const QJsonObject& object)
    {
        WhatSonNoteVersionDiffSegment segment;
        segment.prefixLength = object.value(QStringLiteral("prefixLength")).toInt();
        segment.suffixLength = object.value(QStringLiteral("suffixLength")).toInt();
        segment.removedText = object.value(QStringLiteral("removedText")).toString();
        segment.insertedText = object.value(QStringLiteral("insertedText")).toString();
        segment.unifiedPatch = object.value(QStringLiteral("unifiedPatch")).toString();
        return segment;
    }

    QJsonObject snapshotToJson(const WhatSonNoteVersionSnapshot& snapshot)
    {
        QJsonObject object;
        object.insert(QStringLiteral("snapshotId"), snapshot.snapshotId);
        object.insert(QStringLiteral("parentSnapshotId"), snapshot.parentSnapshotId);
        object.insert(QStringLiteral("sourceSnapshotId"), snapshot.sourceSnapshotId);
        object.insert(QStringLiteral("label"), snapshot.label);
        object.insert(QStringLiteral("createdAtUtc"), snapshot.createdAtUtc);
        object.insert(QStringLiteral("commitModifiedCount"), snapshot.commitModifiedCount);
        object.insert(QStringLiteral("headerText"), snapshot.headerText);
        object.insert(QStringLiteral("bodyDocumentText"), snapshot.bodyDocumentText);
        object.insert(QStringLiteral("bodyPlainText"), snapshot.bodyPlainText);
        object.insert(QStringLiteral("headerDiff"), diffSegmentToJson(snapshot.headerDiff));
        object.insert(QStringLiteral("bodyDiff"), diffSegmentToJson(snapshot.bodyDiff));
        return object;
    }

    WhatSonNoteVersionSnapshot snapshotFromJson(const QJsonObject& object)
    {
        WhatSonNoteVersionSnapshot snapshot;
        snapshot.snapshotId = object.value(QStringLiteral("snapshotId")).toString();
        snapshot.parentSnapshotId = object.value(QStringLiteral("parentSnapshotId")).toString();
        snapshot.sourceSnapshotId = object.value(QStringLiteral("sourceSnapshotId")).toString();
        snapshot.label = object.value(QStringLiteral("label")).toString();
        snapshot.createdAtUtc = object.value(QStringLiteral("createdAtUtc")).toString();
        snapshot.commitModifiedCount = object.value(QStringLiteral("commitModifiedCount")).toInt();
        snapshot.headerText = object.value(QStringLiteral("headerText")).toString();
        snapshot.bodyDocumentText = object.value(QStringLiteral("bodyDocumentText")).toString();
        snapshot.bodyPlainText = object.value(QStringLiteral("bodyPlainText")).toString();
        snapshot.headerDiff = diffSegmentFromJson(object.value(QStringLiteral("headerDiff")).toObject());
        snapshot.bodyDiff = diffSegmentFromJson(object.value(QStringLiteral("bodyDiff")).toObject());
        return snapshot;
    }

    QJsonObject stateToJson(const WhatSonNoteVersionState& state)
    {
        QJsonArray snapshots;
        for (const WhatSonNoteVersionSnapshot& snapshot : state.snapshots)
        {
            snapshots.push_back(snapshotToJson(snapshot));
        }

        QJsonObject root = emptyVersionRoot(state.noteId);
        root.insert(QStringLiteral("currentSnapshotId"), state.currentSnapshotId);
        root.insert(QStringLiteral("headSnapshotId"), state.headSnapshotId);
        root.insert(QStringLiteral("snapshots"), snapshots);
        return root;
    }

    bool stateFromJson(const QJsonObject& root, WhatSonNoteVersionState* outState, QString* errorMessage)
    {
        if (outState == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outState must not be null.");
            }
            return false;
        }

        WhatSonNoteVersionState state;
        state.noteId = root.value(QStringLiteral("noteId")).toString();
        state.currentSnapshotId = root.value(QStringLiteral("currentSnapshotId")).toString();
        state.headSnapshotId = root.value(QStringLiteral("headSnapshotId")).toString();

        const QJsonArray snapshots = root.value(QStringLiteral("snapshots")).toArray();
        state.snapshots.reserve(snapshots.size());
        for (const QJsonValue& value : snapshots)
        {
            const WhatSonNoteVersionSnapshot snapshot = snapshotFromJson(value.toObject());
            if (!snapshot.snapshotId.trimmed().isEmpty())
            {
                state.snapshots.push_back(snapshot);
            }
        }

        *outState = std::move(state);
        return true;
    }

    bool readState(
        const QString& versionFilePath,
        WhatSonNoteVersionState* outState,
        QString* errorMessage)
    {
        const QString versionText = readUtf8File(versionFilePath, errorMessage);
        if (versionText.isEmpty() && errorMessage != nullptr && !errorMessage->isEmpty())
        {
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(versionText.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to parse .wsnversion JSON: %1").arg(versionFilePath);
            }
            return false;
        }

        return stateFromJson(document.object(), outState, errorMessage);
    }

    bool writeState(
        const QString& versionFilePath,
        const WhatSonNoteVersionState& state,
        QString* errorMessage)
    {
        return writeUtf8File(
            versionFilePath,
            QString::fromUtf8(QJsonDocument(stateToJson(state)).toJson(QJsonDocument::Indented)),
            errorMessage);
    }

    const WhatSonNoteVersionSnapshot* findSnapshot(
        const WhatSonNoteVersionState& state,
        const QString& snapshotId)
    {
        for (const WhatSonNoteVersionSnapshot& snapshot : state.snapshots)
        {
            if (snapshot.snapshotId == snapshotId)
            {
                return &snapshot;
            }
        }
        return nullptr;
    }

    QString idSafeLabel(QString label)
    {
        label = label.trimmed().toLower();
        label.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
        label.replace(QRegularExpression(QStringLiteral("^-+|-+$")), QString());
        return label.isEmpty() ? QStringLiteral("snapshot") : label;
    }

    QString uniqueSnapshotId(
        const WhatSonNoteVersionState& state,
        const QString& label,
        const QString& headerText,
        const QString& bodyDocumentText)
    {
        QByteArray seed;
        seed += label.toUtf8();
        seed += '\0';
        seed += headerText.toUtf8();
        seed += '\0';
        seed += bodyDocumentText.toUtf8();
        const QString hash =
            QString::fromLatin1(QCryptographicHash::hash(seed, QCryptographicHash::Sha256).toHex().left(16));
        const QString baseId = QStringLiteral("%1-%2").arg(idSafeLabel(label), hash);

        QString candidate = baseId;
        int suffix = 2;
        while (findSnapshot(state, candidate) != nullptr)
        {
            candidate = QStringLiteral("%1-%2").arg(baseId, QString::number(suffix));
            ++suffix;
        }
        return candidate;
    }

    QString parentSnapshotIdForCapture(const WhatSonNoteVersionState& state)
    {
        if (findSnapshot(state, state.currentSnapshotId) != nullptr)
        {
            return state.currentSnapshotId;
        }
        if (findSnapshot(state, state.headSnapshotId) != nullptr)
        {
            return state.headSnapshotId;
        }
        return state.snapshots.isEmpty() ? QString() : state.snapshots.constLast().snapshotId;
    }

    WhatSonNoteVersionSnapshot buildSnapshot(
        const WhatSonNoteVersionState& state,
        const QString& parentSnapshotId,
        const QString& sourceSnapshotId,
        const QString& label,
        const int commitModifiedCount,
        const QString& headerText,
        const QString& bodyDocumentText,
        const QString& bodyPlainText)
    {
        const WhatSonNoteVersionSnapshot* parentSnapshot = findSnapshot(state, parentSnapshotId);
        const QString previousHeader = parentSnapshot == nullptr ? QString() : parentSnapshot->headerText;
        const QString previousBody = parentSnapshot == nullptr ? QString() : parentSnapshot->bodyDocumentText;

        WhatSonNoteVersionSnapshot snapshot;
        snapshot.snapshotId = uniqueSnapshotId(state, label, headerText, bodyDocumentText);
        snapshot.parentSnapshotId = parentSnapshotId;
        snapshot.sourceSnapshotId = sourceSnapshotId;
        snapshot.label = label;
        snapshot.createdAtUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        snapshot.commitModifiedCount = commitModifiedCount;
        snapshot.headerText = headerText;
        snapshot.bodyDocumentText = bodyDocumentText;
        snapshot.bodyPlainText = bodyPlainText;
        snapshot.headerDiff = diffSegment(previousHeader, headerText, QStringLiteral("header.wsnhead"));
        snapshot.bodyDiff = diffSegment(previousBody, bodyDocumentText, QStringLiteral("body.wsnbody"));
        return snapshot;
    }
} // namespace

bool WhatSonLocalNoteVersionStore::loadState(
    const QString& versionFilePath,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    return readState(QDir::cleanPath(versionFilePath.trimmed()), outState, errorMessage);
}

bool WhatSonLocalNoteVersionStore::captureSnapshot(
    CaptureRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    const QString noteId = noteIdFromDocument(request.document);
    const QString versionFilePath = versionPathFromDocument(request.document);
    if (!ensureVersionDocument(versionFilePath, noteId, errorMessage))
    {
        return false;
    }

    WhatSonNoteVersionState state;
    if (!readState(versionFilePath, &state, errorMessage))
    {
        return false;
    }
    if (state.noteId.trimmed().isEmpty())
    {
        state.noteId = noteId;
    }

    const QString headerPath = headerPathFromDocument(request.document);
    const QString bodyPath = bodyPathFromDocument(request.document);
    QString readError;
    const QString headerText = readUtf8File(headerPath, &readError);
    if (!readError.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = readError;
        }
        return false;
    }
    const QString bodyDocumentText = readUtf8File(bodyPath, &readError);
    if (!readError.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = readError;
        }
        return false;
    }

    QString bodyPlainText = request.document.bodyPlainText.trimmed().isEmpty()
                                ? WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocumentText)
                                : request.document.bodyPlainText;
    bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyPlainText);
    const QString label = request.label.trimmed().isEmpty()
                              ? QStringLiteral("commit:%1").arg(request.commitModifiedCount)
                              : request.label.trimmed();
    const QString parentSnapshotId = parentSnapshotIdForCapture(state);
    WhatSonNoteVersionSnapshot snapshot = buildSnapshot(
        state,
        parentSnapshotId,
        QString(),
        label,
        request.commitModifiedCount,
        headerText,
        bodyDocumentText,
        bodyPlainText);

    state.snapshots.push_back(snapshot);
    state.currentSnapshotId = snapshot.snapshotId;
    state.headSnapshotId = snapshot.snapshotId;
    VersionLimitManager::pruneOldestSnapshots(&state);

    if (!writeState(versionFilePath, state, errorMessage))
    {
        return false;
    }

    if (outSnapshot != nullptr)
    {
        *outSnapshot = snapshot;
    }
    if (outState != nullptr)
    {
        *outState = std::move(state);
    }
    return true;
}

bool WhatSonLocalNoteVersionStore::diffSnapshots(
    DiffRequest request,
    WhatSonNoteVersionDiffResult* outResult,
    QString* errorMessage) const
{
    if (outResult == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outResult must not be null.");
        }
        return false;
    }

    WhatSonNoteVersionState state;
    if (!readState(QDir::cleanPath(request.versionFilePath.trimmed()), &state, errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* baseSnapshot = findSnapshot(state, request.baseSnapshotId);
    const WhatSonNoteVersionSnapshot* targetSnapshot = findSnapshot(state, request.targetSnapshotId);
    if (baseSnapshot == nullptr || targetSnapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve requested snapshots.");
        }
        return false;
    }

    WhatSonNoteVersionDiffResult result;
    result.baseSnapshot = *baseSnapshot;
    result.targetSnapshot = *targetSnapshot;
    result.headerDiff = diffSegment(
        baseSnapshot->headerText,
        targetSnapshot->headerText,
        QStringLiteral("header.wsnhead"));
    result.bodyDiff = diffSegment(
        baseSnapshot->bodyDocumentText,
        targetSnapshot->bodyDocumentText,
        QStringLiteral("body.wsnbody"));
    *outResult = std::move(result);
    return true;
}

bool WhatSonLocalNoteVersionStore::checkoutSnapshot(
    CheckoutRequest request,
    QString* errorMessage) const
{
    const QString versionFilePath = QDir::cleanPath(request.versionFilePath.trimmed());
    WhatSonNoteVersionState state;
    if (!readState(versionFilePath, &state, errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* snapshot = findSnapshot(state, request.snapshotId);
    if (snapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve checkout snapshot: %1").arg(request.snapshotId);
        }
        return false;
    }

    if (!writeUtf8File(QDir::cleanPath(request.noteHeaderPath.trimmed()), snapshot->headerText, errorMessage)
        || !writeUtf8File(QDir::cleanPath(request.noteBodyPath.trimmed()), snapshot->bodyDocumentText, errorMessage))
    {
        return false;
    }

    state.currentSnapshotId = snapshot->snapshotId;
    VersionLimitManager::pruneOldestSnapshots(&state);
    return writeState(versionFilePath, state, errorMessage);
}

bool WhatSonLocalNoteVersionStore::rollbackToSnapshot(
    RollbackRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    QString* errorMessage) const
{
    const QString versionFilePath = QDir::cleanPath(request.versionFilePath.trimmed());
    WhatSonNoteVersionState state;
    if (!readState(versionFilePath, &state, errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* targetSnapshot = findSnapshot(state, request.snapshotId);
    if (targetSnapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve rollback snapshot: %1").arg(request.snapshotId);
        }
        return false;
    }

    if (!writeUtf8File(QDir::cleanPath(request.noteHeaderPath.trimmed()), targetSnapshot->headerText, errorMessage)
        || !writeUtf8File(QDir::cleanPath(request.noteBodyPath.trimmed()), targetSnapshot->bodyDocumentText, errorMessage))
    {
        return false;
    }

    const QString label = request.label.trimmed().isEmpty()
                              ? QStringLiteral("rollback:%1").arg(targetSnapshot->snapshotId)
                              : request.label.trimmed();
    const QString parentSnapshotId = parentSnapshotIdForCapture(state);
    WhatSonNoteVersionSnapshot rollbackSnapshot = buildSnapshot(
        state,
        parentSnapshotId,
        targetSnapshot->snapshotId,
        label,
        request.commitModifiedCount,
        targetSnapshot->headerText,
        targetSnapshot->bodyDocumentText,
        targetSnapshot->bodyPlainText);

    state.snapshots.push_back(rollbackSnapshot);
    state.currentSnapshotId = rollbackSnapshot.snapshotId;
    state.headSnapshotId = rollbackSnapshot.snapshotId;
    VersionLimitManager::pruneOldestSnapshots(&state);

    if (!writeState(versionFilePath, state, errorMessage))
    {
        return false;
    }

    if (outSnapshot != nullptr)
    {
        *outSnapshot = std::move(rollbackSnapshot);
    }
    return true;
}
