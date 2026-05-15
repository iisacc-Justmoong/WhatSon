#pragma once

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

class WhatSonNoteVersionDiffBuilder final
{
public:
    WhatSonNoteVersionDiffSegment diffSegment(
        const QString& before,
        const QString& after,
        const QString& label) const;
};
