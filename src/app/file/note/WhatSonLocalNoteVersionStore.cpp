#include "WhatSonLocalNoteVersionStore.hpp"

#include "WhatSonNoteBodyPersistence.hpp"
#include "WhatSonNoteHeaderCreator.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUuid>

#include <algorithm>

namespace
{
    constexpr auto kNoteVersionSchema = "whatson.note.version.store";

    QString createEmptyVersionDocumentText(const QString& noteId)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kNoteVersionSchema));
        root.insert(QStringLiteral("noteId"), noteId.trimmed());
        root.insert(QStringLiteral("currentSnapshotId"), QString());
        root.insert(QStringLiteral("headSnapshotId"), QString());
        root.insert(QStringLiteral("snapshots"), QJsonArray{});
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    QString serializeBodyDocument(const QString& noteId, const QString& plainText)
    {
        QString normalizedText = plainText;
        normalizedText.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        normalizedText.replace(QLatin1Char('\r'), QLatin1Char('\n'));

        const QString normalizedId = noteId.trimmed().isEmpty() ? QStringLiteral("note") : noteId.trimmed();
        QString escapedId = normalizedId;
        escapedId.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        escapedId.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        escapedId.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        escapedId.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        escapedId.replace(QStringLiteral("'"), QStringLiteral("&apos;"));

        auto escapeXmlText = [](QString value)
        {
            value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
            value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
            value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
            value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
            value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
            return value;
        };

        const QStringList lines = normalizedText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"") + escapedId + QStringLiteral("\">\n");
        text += QStringLiteral("  <body>\n");
        if (normalizedText.isEmpty())
        {
            text += QStringLiteral("  </body>\n");
            text += QStringLiteral("</contents>\n");
            return text;
        }

        for (const QString& line : lines)
        {
            text += QStringLiteral("    <paragraph>") + escapeXmlText(line) + QStringLiteral("</paragraph>\n");
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QJsonObject snapshotToJson(const WhatSonNoteVersionSnapshot& snapshot)
    {
        QJsonObject root;
        root.insert(QStringLiteral("snapshotId"), snapshot.snapshotId);
        root.insert(QStringLiteral("noteId"), snapshot.noteId);
        root.insert(QStringLiteral("parentSnapshotId"), snapshot.parentSnapshotId);
        root.insert(QStringLiteral("sourceSnapshotId"), snapshot.sourceSnapshotId);
        root.insert(QStringLiteral("operation"), snapshot.operation);
        root.insert(QStringLiteral("label"), snapshot.label);
        root.insert(QStringLiteral("capturedAtUtc"), snapshot.capturedAtUtc);
        root.insert(QStringLiteral("headerText"), snapshot.headerText);
        root.insert(QStringLiteral("bodyDocumentText"), snapshot.bodyDocumentText);
        root.insert(QStringLiteral("bodyPlainText"), snapshot.bodyPlainText);
        return root;
    }

    WhatSonNoteVersionSnapshot snapshotFromJson(const QJsonObject& object)
    {
        WhatSonNoteVersionSnapshot snapshot;
        snapshot.snapshotId = object.value(QStringLiteral("snapshotId")).toString().trimmed();
        snapshot.noteId = object.value(QStringLiteral("noteId")).toString().trimmed();
        snapshot.parentSnapshotId = object.value(QStringLiteral("parentSnapshotId")).toString().trimmed();
        snapshot.sourceSnapshotId = object.value(QStringLiteral("sourceSnapshotId")).toString().trimmed();
        snapshot.operation = object.value(QStringLiteral("operation")).toString().trimmed();
        snapshot.label = object.value(QStringLiteral("label")).toString();
        snapshot.capturedAtUtc = object.value(QStringLiteral("capturedAtUtc")).toString().trimmed();
        snapshot.headerText = object.value(QStringLiteral("headerText")).toString();
        snapshot.bodyDocumentText = object.value(QStringLiteral("bodyDocumentText")).toString();
        snapshot.bodyPlainText = object.value(QStringLiteral("bodyPlainText")).toString();
        return snapshot;
    }
} // namespace

WhatSonLocalNoteVersionStore::WhatSonLocalNoteVersionStore() = default;

WhatSonLocalNoteVersionStore::~WhatSonLocalNoteVersionStore() = default;

QString WhatSonLocalNoteVersionStore::normalizePath(QString path) const
{
    path = path.trimmed();
    if (path.isEmpty())
    {
        return {};
    }
    return QDir::cleanPath(path);
}

QString WhatSonLocalNoteVersionStore::resolveNoteStem(const QString& noteId, const QString& noteDirectoryPath) const
{
    QString stem = QFileInfo(normalizePath(noteDirectoryPath)).completeBaseName().trimmed();
    if (stem.isEmpty())
    {
        stem = QFileInfo(noteId.trimmed()).completeBaseName().trimmed();
    }
    if (stem.isEmpty())
    {
        stem = QFileInfo(noteId.trimmed()).fileName().trimmed();
    }
    if (stem.isEmpty())
    {
        stem = QStringLiteral("note");
    }
    return stem;
}

QString WhatSonLocalNoteVersionStore::resolveNoteId(const WhatSonLocalNoteDocument& document) const
{
    QString noteId = document.headerStore.noteId().trimmed();
    if (noteId.isEmpty() && !document.noteHeaderPath.trimmed().isEmpty())
    {
        noteId = QFileInfo(document.noteHeaderPath).completeBaseName().trimmed();
    }
    if (noteId.isEmpty())
    {
        noteId = resolveNoteStem(QString(), document.noteDirectoryPath);
    }
    return noteId;
}

QString WhatSonLocalNoteVersionStore::resolveVersionPath(const WhatSonLocalNoteDocument& document) const
{
    const QString directPath = normalizePath(document.noteVersionPath);
    if (!directPath.isEmpty())
    {
        return directPath;
    }

    const QString noteDirectoryPath = normalizePath(document.noteDirectoryPath);
    if (noteDirectoryPath.isEmpty())
    {
        return {};
    }
    return QDir(noteDirectoryPath).filePath(
        resolveNoteStem(resolveNoteId(document), noteDirectoryPath) + QStringLiteral(".wsnversion"));
}

QString WhatSonLocalNoteVersionStore::currentTimestampUtc() const
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

bool WhatSonLocalNoteVersionStore::ensureState(
    const WhatSonLocalNoteDocument& document,
    WhatSonNoteVersionState* outState,
    QString* outVersionPath,
    QString* errorMessage) const
{
    if (outState == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outState must not be null.");
        }
        return false;
    }

    const QString noteId = resolveNoteId(document);
    const QString versionPath = resolveVersionPath(document);
    if (noteId.isEmpty() || versionPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve note version storage path.");
        }
        return false;
    }

    if (!m_ioGateway.exists(versionPath))
    {
        QString writeError;
        if (!m_ioGateway.writeUtf8File(versionPath, createEmptyVersionDocumentText(noteId), &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    QString rawText;
    QString readError;
    if (!m_ioGateway.readUtf8File(versionPath, &rawText, &readError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = readError;
        }
        return false;
    }

    WhatSonNoteVersionState state;
    if (rawText.trimmed().isEmpty())
    {
        state.noteId = noteId;
    }
    else
    {
        QJsonParseError parseError;
        const QJsonDocument documentJson = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !documentJson.isObject())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to parse .wsnversion: %1").arg(parseError.errorString());
            }
            return false;
        }

        const QJsonObject root = documentJson.object();
        state.noteId = root.value(QStringLiteral("noteId")).toString().trimmed();
        state.currentSnapshotId = root.value(QStringLiteral("currentSnapshotId")).toString().trimmed();
        state.headSnapshotId = root.value(QStringLiteral("headSnapshotId")).toString().trimmed();

        const QJsonArray snapshotsArray = root.value(QStringLiteral("snapshots")).toArray();
        state.snapshots.reserve(snapshotsArray.size());
        for (const QJsonValue& value : snapshotsArray)
        {
            if (!value.isObject())
            {
                continue;
            }
            state.snapshots.push_back(snapshotFromJson(value.toObject()));
        }
    }

    if (state.noteId.isEmpty())
    {
        state.noteId = noteId;
    }
    if (outVersionPath != nullptr)
    {
        *outVersionPath = versionPath;
    }
    *outState = std::move(state);
    return true;
}

bool WhatSonLocalNoteVersionStore::writeState(
    const QString& versionPath,
    const WhatSonNoteVersionState& state,
    QString* errorMessage) const
{
    QJsonArray snapshotsArray;
    for (const WhatSonNoteVersionSnapshot& snapshot : state.snapshots)
    {
        snapshotsArray.push_back(snapshotToJson(snapshot));
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QString::fromLatin1(kNoteVersionSchema));
    root.insert(QStringLiteral("noteId"), state.noteId.trimmed());
    root.insert(QStringLiteral("currentSnapshotId"), state.currentSnapshotId.trimmed());
    root.insert(QStringLiteral("headSnapshotId"), state.headSnapshotId.trimmed());
    root.insert(QStringLiteral("snapshots"), snapshotsArray);
    return m_ioGateway.writeUtf8File(
        versionPath,
        QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented)),
        errorMessage);
}

bool WhatSonLocalNoteVersionStore::readWorkingTreeSnapshot(
    const WhatSonLocalNoteDocument& document,
    WhatSonNoteVersionSnapshot* outSnapshot,
    QString* errorMessage) const
{
    if (outSnapshot == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outSnapshot must not be null.");
        }
        return false;
    }

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = resolveNoteId(document);
    readRequest.noteDirectoryPath = document.noteDirectoryPath;
    readRequest.noteHeaderPath = document.noteHeaderPath;
    readRequest.noteBodyPath = document.noteBodyPath;

    WhatSonLocalNoteDocument normalizedDocument;
    QString readError;
    if (!m_localNoteFileStore.readNote(std::move(readRequest), &normalizedDocument, &readError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = readError;
        }
        return false;
    }

    QString headerText;
    if (!normalizedDocument.noteHeaderPath.trimmed().isEmpty() && m_ioGateway.exists(normalizedDocument.noteHeaderPath))
    {
        if (!m_ioGateway.readUtf8File(normalizedDocument.noteHeaderPath, &headerText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }
    }
    else
    {
        WhatSonNoteHeaderCreator headerCreator(normalizedDocument.noteDirectoryPath, QString());
        headerText = headerCreator.createHeaderText(normalizedDocument.headerStore);
    }

    QString bodyDocumentText;
    if (!normalizedDocument.noteBodyPath.trimmed().isEmpty() && m_ioGateway.exists(normalizedDocument.noteBodyPath))
    {
        if (!m_ioGateway.readUtf8File(normalizedDocument.noteBodyPath, &bodyDocumentText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }
    }
    else
    {
        bodyDocumentText = serializeBodyDocument(normalizedDocument.headerStore.noteId(), normalizedDocument.bodyPlainText);
    }

    outSnapshot->noteId = normalizedDocument.headerStore.noteId().trimmed();
    outSnapshot->headerText = headerText;
    outSnapshot->bodyDocumentText = bodyDocumentText;
    outSnapshot->bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(normalizedDocument.bodyPlainText);
    return true;
}

bool WhatSonLocalNoteVersionStore::applySnapshotToWorkingTree(
    const WhatSonLocalNoteDocument& baseDocument,
    const WhatSonNoteVersionSnapshot& snapshot,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    const QString noteDirectoryPath = normalizePath(baseDocument.noteDirectoryPath);
    if (noteDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteDirectoryPath must not be empty for checkout/rollback.");
        }
        return false;
    }

    const QString noteStem = resolveNoteStem(snapshot.noteId, noteDirectoryPath);
    const QString headerPath = normalizePath(baseDocument.noteHeaderPath).isEmpty()
                                   ? QDir(noteDirectoryPath).filePath(noteStem + QStringLiteral(".wsnhead"))
                                   : normalizePath(baseDocument.noteHeaderPath);
    const QString bodyPath = normalizePath(baseDocument.noteBodyPath).isEmpty()
                                 ? QDir(noteDirectoryPath).filePath(noteStem + QStringLiteral(".wsnbody"))
                                 : normalizePath(baseDocument.noteBodyPath);

    QString ioError;
    if (!m_ioGateway.ensureDirectory(noteDirectoryPath, &ioError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }

    if (!m_ioGateway.writeUtf8File(headerPath, snapshot.headerText, &ioError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }
    if (!m_ioGateway.writeUtf8File(bodyPath, snapshot.bodyDocumentText, &ioError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }

    if (outDocument != nullptr)
    {
        WhatSonLocalNoteFileStore::ReadRequest readRequest;
        readRequest.noteId = snapshot.noteId;
        readRequest.noteDirectoryPath = noteDirectoryPath;
        readRequest.noteHeaderPath = headerPath;
        readRequest.noteBodyPath = bodyPath;
        if (!m_localNoteFileStore.readNote(std::move(readRequest), outDocument, &ioError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = ioError;
            }
            return false;
        }
    }

    return true;
}

int WhatSonLocalNoteVersionStore::indexOfSnapshot(const WhatSonNoteVersionState& state, const QString& snapshotId) const
{
    const QString normalizedSnapshotId = snapshotId.trimmed();
    if (normalizedSnapshotId.isEmpty())
    {
        return -1;
    }

    for (int index = 0; index < state.snapshots.size(); ++index)
    {
        if (state.snapshots.at(index).snapshotId.trimmed() == normalizedSnapshotId)
        {
            return index;
        }
    }
    return -1;
}

WhatSonNoteVersionDiffSegment WhatSonLocalNoteVersionStore::diffSegment(const QString& fromText, const QString& toText) const
{
    WhatSonNoteVersionDiffSegment segment;
    const int maxPrefixLength = std::min(fromText.size(), toText.size());
    while (segment.prefixLength < maxPrefixLength
           && fromText.at(segment.prefixLength) == toText.at(segment.prefixLength))
    {
        ++segment.prefixLength;
    }

    const int fromRemainingLength = fromText.size() - segment.prefixLength;
    const int toRemainingLength = toText.size() - segment.prefixLength;
    const int maxSuffixLength = std::min(fromRemainingLength, toRemainingLength);
    while (segment.suffixLength < maxSuffixLength
           && fromText.at(fromText.size() - segment.suffixLength - 1)
                  == toText.at(toText.size() - segment.suffixLength - 1))
    {
        ++segment.suffixLength;
    }

    segment.removedText = fromText.mid(segment.prefixLength, fromRemainingLength - segment.suffixLength);
    segment.insertedText = toText.mid(segment.prefixLength, toRemainingLength - segment.suffixLength);
    return segment;
}

bool WhatSonLocalNoteVersionStore::readState(
    ReadRequest request,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    QString versionPath;
    return ensureState(request.document, outState, &versionPath, errorMessage);
}

bool WhatSonLocalNoteVersionStore::captureSnapshot(
    CaptureRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    WhatSonNoteVersionState state;
    QString versionPath;
    if (!ensureState(request.document, &state, &versionPath, errorMessage))
    {
        return false;
    }

    WhatSonNoteVersionSnapshot snapshot;
    if (!readWorkingTreeSnapshot(request.document, &snapshot, errorMessage))
    {
        return false;
    }

    snapshot.snapshotId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    snapshot.parentSnapshotId = state.currentSnapshotId;
    snapshot.sourceSnapshotId.clear();
    snapshot.operation = QStringLiteral("capture");
    snapshot.label = request.label;
    snapshot.capturedAtUtc = currentTimestampUtc();

    state.noteId = snapshot.noteId;
    state.snapshots.push_back(snapshot);
    state.currentSnapshotId = snapshot.snapshotId;
    state.headSnapshotId = snapshot.snapshotId;

    if (!writeState(versionPath, state, errorMessage))
    {
        return false;
    }

    if (outSnapshot != nullptr)
    {
        *outSnapshot = snapshot;
    }
    if (outState != nullptr)
    {
        *outState = state;
    }
    return true;
}

bool WhatSonLocalNoteVersionStore::diffSnapshots(
    DiffRequest request,
    WhatSonNoteVersionDiff* outDiff,
    QString* errorMessage) const
{
    if (outDiff == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outDiff must not be null.");
        }
        return false;
    }

    WhatSonNoteVersionState state;
    QString versionPath;
    if (!ensureState(request.document, &state, &versionPath, errorMessage))
    {
        return false;
    }

    const int fromIndex = indexOfSnapshot(state, request.fromSnapshotId);
    const int toIndex = indexOfSnapshot(state, request.toSnapshotId);
    if (fromIndex < 0 || toIndex < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve requested snapshots for diff.");
        }
        return false;
    }

    const WhatSonNoteVersionSnapshot& fromSnapshot = state.snapshots.at(fromIndex);
    const WhatSonNoteVersionSnapshot& toSnapshot = state.snapshots.at(toIndex);

    WhatSonNoteVersionDiff diff;
    diff.noteId = state.noteId;
    diff.fromSnapshotId = fromSnapshot.snapshotId;
    diff.toSnapshotId = toSnapshot.snapshotId;
    diff.header = diffSegment(fromSnapshot.headerText, toSnapshot.headerText);
    diff.body = diffSegment(fromSnapshot.bodyPlainText, toSnapshot.bodyPlainText);
    *outDiff = diff;
    return true;
}

bool WhatSonLocalNoteVersionStore::checkoutSnapshot(
    CheckoutRequest request,
    WhatSonLocalNoteDocument* outDocument,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    WhatSonNoteVersionState state;
    QString versionPath;
    if (!ensureState(request.document, &state, &versionPath, errorMessage))
    {
        return false;
    }

    const int targetIndex = indexOfSnapshot(state, request.snapshotId);
    if (targetIndex < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve checkout snapshot.");
        }
        return false;
    }

    WhatSonLocalNoteDocument checkedOutDocument;
    if (!applySnapshotToWorkingTree(request.document, state.snapshots.at(targetIndex), &checkedOutDocument, errorMessage))
    {
        return false;
    }

    state.currentSnapshotId = state.snapshots.at(targetIndex).snapshotId;
    if (!writeState(versionPath, state, errorMessage))
    {
        return false;
    }

    if (outDocument != nullptr)
    {
        *outDocument = checkedOutDocument;
    }
    if (outState != nullptr)
    {
        *outState = state;
    }
    return true;
}

bool WhatSonLocalNoteVersionStore::rollbackToSnapshot(
    RollbackRequest request,
    WhatSonNoteVersionSnapshot* outSnapshot,
    WhatSonLocalNoteDocument* outDocument,
    WhatSonNoteVersionState* outState,
    QString* errorMessage) const
{
    WhatSonNoteVersionState state;
    QString versionPath;
    if (!ensureState(request.document, &state, &versionPath, errorMessage))
    {
        return false;
    }

    const int targetIndex = indexOfSnapshot(state, request.snapshotId);
    if (targetIndex < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve rollback snapshot.");
        }
        return false;
    }

    const QString previousCurrentSnapshotId = state.currentSnapshotId;
    WhatSonLocalNoteDocument rolledBackDocument;
    if (!applySnapshotToWorkingTree(request.document, state.snapshots.at(targetIndex), &rolledBackDocument, errorMessage))
    {
        return false;
    }

    WhatSonNoteVersionSnapshot rollbackSnapshot;
    if (!readWorkingTreeSnapshot(rolledBackDocument, &rollbackSnapshot, errorMessage))
    {
        return false;
    }

    rollbackSnapshot.snapshotId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    rollbackSnapshot.parentSnapshotId = previousCurrentSnapshotId;
    rollbackSnapshot.sourceSnapshotId = state.snapshots.at(targetIndex).snapshotId;
    rollbackSnapshot.operation = QStringLiteral("rollback");
    rollbackSnapshot.label = request.label.trimmed().isEmpty()
                                 ? QStringLiteral("rollback:%1").arg(state.snapshots.at(targetIndex).snapshotId)
                                 : request.label;
    rollbackSnapshot.capturedAtUtc = currentTimestampUtc();

    state.noteId = rollbackSnapshot.noteId;
    state.snapshots.push_back(rollbackSnapshot);
    state.currentSnapshotId = rollbackSnapshot.snapshotId;
    state.headSnapshotId = rollbackSnapshot.snapshotId;

    if (!writeState(versionPath, state, errorMessage))
    {
        return false;
    }

    if (outSnapshot != nullptr)
    {
        *outSnapshot = rollbackSnapshot;
    }
    if (outDocument != nullptr)
    {
        *outDocument = rolledBackDocument;
    }
    if (outState != nullptr)
    {
        *outState = state;
    }
    return true;
}
