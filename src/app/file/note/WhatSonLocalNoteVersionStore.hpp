#pragma once

#include "WhatSonLocalNoteDocument.hpp"
#include "WhatSonLocalNoteFileStore.hpp"
#include "file/IO/WhatSonSystemIoGateway.hpp"

#include <QString>
#include <QVector>

struct WhatSonNoteVersionDiffSegment
{
    int prefixLength = 0;
    int suffixLength = 0;
    QString removedText;
    QString insertedText;
    QString unifiedPatch;
};

struct WhatSonNoteVersionSnapshot
{
    QString snapshotId;
    QString noteId;
    QString parentSnapshotId;
    QString sourceSnapshotId;
    QString operation;
    QString label;
    QString capturedAtUtc;
    int commitModifiedCount = -1;
    QString headerText;
    QString bodyDocumentText;
    QString bodyPlainText;
    WhatSonNoteVersionDiffSegment headerDiff;
    WhatSonNoteVersionDiffSegment bodyDiff;
};

struct WhatSonNoteVersionState
{
    QString noteId;
    QString currentSnapshotId;
    QString headSnapshotId;
    QVector<WhatSonNoteVersionSnapshot> snapshots;
};

struct WhatSonNoteVersionDiff
{
    QString noteId;
    QString fromSnapshotId;
    QString toSnapshotId;
    WhatSonNoteVersionDiffSegment header;
    WhatSonNoteVersionDiffSegment body;
};

class WhatSonLocalNoteVersionStore final
{
public:
    struct ReadRequest final
    {
        WhatSonLocalNoteDocument document;
    };

    struct CaptureRequest final
    {
        WhatSonLocalNoteDocument document;
        QString label;
        int commitModifiedCount = -1;
    };

    struct DiffRequest final
    {
        WhatSonLocalNoteDocument document;
        QString fromSnapshotId;
        QString toSnapshotId;
    };

    struct CheckoutRequest final
    {
        WhatSonLocalNoteDocument document;
        QString snapshotId;
    };

    struct RollbackRequest final
    {
        WhatSonLocalNoteDocument document;
        QString snapshotId;
        QString label;
    };

    WhatSonLocalNoteVersionStore();
    ~WhatSonLocalNoteVersionStore();

    bool readState(ReadRequest request, WhatSonNoteVersionState* outState, QString* errorMessage = nullptr) const;
    bool captureSnapshot(
        CaptureRequest request,
        WhatSonNoteVersionSnapshot* outSnapshot = nullptr,
        WhatSonNoteVersionState* outState = nullptr,
        QString* errorMessage = nullptr) const;
    bool diffSnapshots(DiffRequest request, WhatSonNoteVersionDiff* outDiff, QString* errorMessage = nullptr) const;
    bool checkoutSnapshot(
        CheckoutRequest request,
        WhatSonLocalNoteDocument* outDocument = nullptr,
        WhatSonNoteVersionState* outState = nullptr,
        QString* errorMessage = nullptr) const;
    bool rollbackToSnapshot(
        RollbackRequest request,
        WhatSonNoteVersionSnapshot* outSnapshot = nullptr,
        WhatSonLocalNoteDocument* outDocument = nullptr,
        WhatSonNoteVersionState* outState = nullptr,
        QString* errorMessage = nullptr) const;

private:
    QString normalizePath(QString path) const;
    QString resolveNoteStem(const QString& noteId, const QString& noteDirectoryPath) const;
    QString resolveNoteId(const WhatSonLocalNoteDocument& document) const;
    QString resolveVersionPath(const WhatSonLocalNoteDocument& document) const;
    QString currentTimestampUtc() const;

    bool ensureState(
        const WhatSonLocalNoteDocument& document,
        WhatSonNoteVersionState* outState,
        QString* outVersionPath,
        QString* errorMessage = nullptr) const;
    bool writeState(
        const QString& versionPath,
        const WhatSonNoteVersionState& state,
        QString* errorMessage = nullptr) const;
    bool readWorkingTreeSnapshot(
        const WhatSonLocalNoteDocument& document,
        WhatSonNoteVersionSnapshot* outSnapshot,
        QString* errorMessage = nullptr) const;
    bool applySnapshotToWorkingTree(
        const WhatSonLocalNoteDocument& baseDocument,
        const WhatSonNoteVersionSnapshot& snapshot,
        WhatSonLocalNoteDocument* outDocument = nullptr,
        QString* errorMessage = nullptr) const;
    int indexOfSnapshot(const WhatSonNoteVersionState& state, const QString& snapshotId) const;
    WhatSonNoteVersionDiffSegment diffSegment(
        const QString& fromText,
        const QString& toText,
        const QString& fromLabel = QString(),
        const QString& toLabel = QString()) const;

    WhatSonLocalNoteFileStore m_localNoteFileStore;
    WhatSonSystemIoGateway m_ioGateway;
};
