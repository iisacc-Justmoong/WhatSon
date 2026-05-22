#pragma once

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

class WhatSonNoteVersionDiffBuilder final
{
public:
    WhatSonNoteVersionDiffSegment diffSegment(
        const QString& before,
        const QString& after,
        const QString& label) const;
    QString applyDiffSegmentOntoCurrent(
        const QString& base,
        const QString& current,
        const WhatSonNoteVersionDiffSegment& segment,
        bool* applied = nullptr,
        QString* errorMessage = nullptr) const;
};
