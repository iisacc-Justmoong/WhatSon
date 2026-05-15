#pragma once

#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"

class VersionLimitManager final
{
public:
    static int maximumSnapshotCount() noexcept;
    static int pruneOldestSnapshots(WhatSonNoteVersionState* state);
};
