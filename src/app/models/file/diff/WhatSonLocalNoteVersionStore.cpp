#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

#include "app/models/file/diff/VersionLimitManager.h"
#include "app/models/file/diff/WhatSonNoteVersionDiffBuilder.hpp"
#include "app/models/file/diff/WhatSonNoteVersionFileGateway.hpp"
#include "app/models/file/diff/WhatSonNoteVersionSnapshotBuilder.hpp"
#include "app/models/file/diff/WhatSonNoteVersionStateCodec.hpp"
#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"

#include <QDir>

#include <utility>

namespace
{
    bool readState(
        const WhatSonNoteVersionFileGateway& fileGateway,
        const WhatSonNoteVersionStateCodec& stateCodec,
        const QString& versionFilePath,
        WhatSonNoteVersionState* outState,
        QString* errorMessage)
    {
        QString versionText;
        if (!fileGateway.readUtf8File(versionFilePath, &versionText, errorMessage))
        {
            return false;
        }
        return stateCodec.parseState(versionText, versionFilePath, outState, errorMessage);
    }

    bool writeState(
        const WhatSonNoteVersionFileGateway& fileGateway,
        const WhatSonNoteVersionStateCodec& stateCodec,
        const QString& versionFilePath,
        const WhatSonNoteVersionState& state,
        QString* errorMessage)
    {
        return fileGateway.writeUtf8File(versionFilePath, stateCodec.serializeState(state), errorMessage);
    }
} // namespace

bool WhatSonLocalNoteVersionStore::loadState(
    const QString& versionFilePath,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    const WhatSonNoteVersionFileGateway fileGateway;
    const WhatSonNoteVersionStateCodec stateCodec;
    return readState(
        fileGateway,
        stateCodec,
        QDir::cleanPath(versionFilePath.trimmed()),
        outState,
        errorMessage);
}

bool WhatSonLocalNoteVersionStore::captureSnapshot(
    CaptureRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    const WhatSonNoteVersionFileGateway fileGateway;
    const WhatSonNoteVersionStateCodec stateCodec;
    const WhatSonNoteVersionSnapshotBuilder snapshotBuilder;

    const QString noteId = fileGateway.noteIdFromDocument(request.document);
    const QString versionFilePath = fileGateway.versionPathFromDocument(request.document);
    if (!fileGateway.ensureVersionDocument(
            versionFilePath,
            stateCodec.emptyStateText(noteId),
            errorMessage))
    {
        return false;
    }

    WhatSonNoteVersionState state;
    if (!readState(fileGateway, stateCodec, versionFilePath, &state, errorMessage))
    {
        return false;
    }
    if (state.noteId.trimmed().isEmpty())
    {
        state.noteId = noteId;
    }

    QString headerText;
    if (!fileGateway.readUtf8File(fileGateway.headerPathFromDocument(request.document), &headerText, errorMessage))
    {
        return false;
    }

    QString bodyDocumentText;
    if (!fileGateway.readUtf8File(fileGateway.bodyPathFromDocument(request.document), &bodyDocumentText, errorMessage))
    {
        return false;
    }

    QString bodyPlainText = request.document.bodyPlainText.trimmed().isEmpty()
                                ? WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocumentText)
                                : request.document.bodyPlainText;
    bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyPlainText);
    const QString label = request.label.trimmed().isEmpty()
                              ? QStringLiteral("commit:%1").arg(request.commitModifiedCount)
                              : request.label.trimmed();
    const QString parentSnapshotId = snapshotBuilder.parentSnapshotIdForCapture(state);
    WhatSonNoteVersionSnapshot snapshot = snapshotBuilder.buildSnapshot(
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

    if (!writeState(fileGateway, stateCodec, versionFilePath, state, errorMessage))
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

    const WhatSonNoteVersionFileGateway fileGateway;
    const WhatSonNoteVersionStateCodec stateCodec;
    const WhatSonNoteVersionSnapshotBuilder snapshotBuilder;
    const WhatSonNoteVersionDiffBuilder diffBuilder;

    WhatSonNoteVersionState state;
    if (!readState(
            fileGateway,
            stateCodec,
            QDir::cleanPath(request.versionFilePath.trimmed()),
            &state,
            errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* baseSnapshot =
        snapshotBuilder.findSnapshot(state, request.baseSnapshotId);
    const WhatSonNoteVersionSnapshot* targetSnapshot =
        snapshotBuilder.findSnapshot(state, request.targetSnapshotId);
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
    result.headerDiff = diffBuilder.diffSegment(
        baseSnapshot->headerText,
        targetSnapshot->headerText,
        QStringLiteral("header.wsnhead"));
    result.bodyDiff = diffBuilder.diffSegment(
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
    const WhatSonNoteVersionFileGateway fileGateway;
    const WhatSonNoteVersionStateCodec stateCodec;
    const WhatSonNoteVersionSnapshotBuilder snapshotBuilder;
    const QString versionFilePath = QDir::cleanPath(request.versionFilePath.trimmed());

    WhatSonNoteVersionState state;
    if (!readState(fileGateway, stateCodec, versionFilePath, &state, errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* snapshot = snapshotBuilder.findSnapshot(state, request.snapshotId);
    if (snapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve checkout snapshot: %1").arg(request.snapshotId);
        }
        return false;
    }

    if (!fileGateway.writeUtf8File(
            QDir::cleanPath(request.noteHeaderPath.trimmed()),
            snapshot->headerText,
            errorMessage)
        || !fileGateway.writeUtf8File(
            QDir::cleanPath(request.noteBodyPath.trimmed()),
            snapshot->bodyDocumentText,
            errorMessage))
    {
        return false;
    }

    state.currentSnapshotId = snapshot->snapshotId;
    VersionLimitManager::pruneOldestSnapshots(&state);
    return writeState(fileGateway, stateCodec, versionFilePath, state, errorMessage);
}

bool WhatSonLocalNoteVersionStore::rollbackToSnapshot(
    RollbackRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    QString* errorMessage) const
{
    const WhatSonNoteVersionFileGateway fileGateway;
    const WhatSonNoteVersionStateCodec stateCodec;
    const WhatSonNoteVersionSnapshotBuilder snapshotBuilder;
    const QString versionFilePath = QDir::cleanPath(request.versionFilePath.trimmed());

    WhatSonNoteVersionState state;
    if (!readState(fileGateway, stateCodec, versionFilePath, &state, errorMessage))
    {
        return false;
    }

    const WhatSonNoteVersionSnapshot* targetSnapshot =
        snapshotBuilder.findSnapshot(state, request.snapshotId);
    if (targetSnapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve rollback snapshot: %1").arg(request.snapshotId);
        }
        return false;
    }

    if (!fileGateway.writeUtf8File(
            QDir::cleanPath(request.noteHeaderPath.trimmed()),
            targetSnapshot->headerText,
            errorMessage)
        || !fileGateway.writeUtf8File(
            QDir::cleanPath(request.noteBodyPath.trimmed()),
            targetSnapshot->bodyDocumentText,
            errorMessage))
    {
        return false;
    }

    const QString label = request.label.trimmed().isEmpty()
                              ? QStringLiteral("rollback:%1").arg(targetSnapshot->snapshotId)
                              : request.label.trimmed();
    const QString parentSnapshotId = snapshotBuilder.parentSnapshotIdForCapture(state);
    WhatSonNoteVersionSnapshot rollbackSnapshot = snapshotBuilder.buildSnapshot(
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

    if (!writeState(fileGateway, stateCodec, versionFilePath, state, errorMessage))
    {
        return false;
    }

    if (outSnapshot != nullptr)
    {
        *outSnapshot = std::move(rollbackSnapshot);
    }
    return true;
}
