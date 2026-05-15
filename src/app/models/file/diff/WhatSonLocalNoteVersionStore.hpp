#pragma once

#include "app/models/file/note/local/WhatSonLocalNoteDocument.hpp"

#include <QString>
#include <QVector>

struct WhatSonNoteVersionDiffSegment final
{
    int prefixLength = 0;
    int suffixLength = 0;
    QString removedText;
    QString insertedText;
    QString unifiedPatch;
    QString generatedAtUtc;
};

struct WhatSonNoteVersionSnapshot final
{
    QString snapshotId;
    QString parentSnapshotId;
    QString sourceSnapshotId;
    QString label;
    QString createdAtUtc;
    int commitModifiedCount = 0;
    QString headerText;
    QString bodyDocumentText;
    QString bodyPlainText;
    WhatSonNoteVersionDiffSegment headerDiff;
    WhatSonNoteVersionDiffSegment bodyDiff;
};

struct WhatSonNoteVersionState final
{
    QString noteId;
    QString currentSnapshotId;
    QString headSnapshotId;
    QVector<WhatSonNoteVersionSnapshot> snapshots;
};

struct WhatSonNoteVersionDiffResult final
{
    WhatSonNoteVersionSnapshot baseSnapshot;
    WhatSonNoteVersionSnapshot targetSnapshot;
    WhatSonNoteVersionDiffSegment headerDiff;
    WhatSonNoteVersionDiffSegment bodyDiff;
};

class WhatSonLocalNoteVersionStore final
{
public:
    struct CaptureRequest final
    {
        WhatSonLocalNoteDocument document;
        QString label;
        int commitModifiedCount = 0;
    };

    struct DiffRequest final
    {
        QString versionFilePath;
        QString baseSnapshotId;
        QString targetSnapshotId;
    };

    struct CheckoutRequest final
    {
        QString versionFilePath;
        QString snapshotId;
        QString noteHeaderPath;
        QString noteBodyPath;
    };

    struct RollbackRequest final
    {
        QString versionFilePath;
        QString snapshotId;
        QString noteHeaderPath;
        QString noteBodyPath;
        QString label;
        int commitModifiedCount = 0;
    };

    bool loadState(
        const QString& versionFilePath,
        WhatSonNoteVersionState* outState,
        QString* errorMessage = nullptr) const;

    bool captureSnapshot(
        CaptureRequest request,
        WhatSonNoteVersionSnapshot* outSnapshot = nullptr,
        WhatSonNoteVersionState* outState = nullptr,
        QString* errorMessage = nullptr) const;

    bool diffSnapshots(
        DiffRequest request,
        WhatSonNoteVersionDiffResult* outResult,
        QString* errorMessage = nullptr) const;

    bool checkoutSnapshot(
        CheckoutRequest request,
        QString* errorMessage = nullptr) const;

    bool rollbackToSnapshot(
        RollbackRequest request,
        WhatSonNoteVersionSnapshot* outSnapshot = nullptr,
        QString* errorMessage = nullptr) const;
};
