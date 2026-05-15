#pragma once

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

class WhatSonNoteVersionStateCodec final
{
public:
    QString emptyStateText(const QString& noteId) const;
    QString serializeState(const WhatSonNoteVersionState& state) const;
    bool parseState(
        const QString& versionText,
        const QString& versionFilePath,
        WhatSonNoteVersionState* outState,
        QString* errorMessage = nullptr) const;
};
