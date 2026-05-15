#pragma once

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

class WhatSonNoteVersionSnapshotBuilder final
{
public:
    const WhatSonNoteVersionSnapshot* findSnapshot(
        const WhatSonNoteVersionState& state,
        const QString& snapshotId) const;

    QString parentSnapshotIdForCapture(const WhatSonNoteVersionState& state) const;

    WhatSonNoteVersionSnapshot buildSnapshot(
        const WhatSonNoteVersionState& state,
        const QString& parentSnapshotId,
        const QString& sourceSnapshotId,
        const QString& label,
        int commitModifiedCount,
        const QString& headerText,
        const QString& bodyDocumentText,
        const QString& bodyPlainText) const;
};
