#include "app/models/file/diff/VersionLimitManager.h"

namespace
{
    constexpr int kMaximumSnapshotCount = 100;

    bool containsSnapshotId(const WhatSonNoteVersionState& state, const QString& snapshotId)
    {
        if (snapshotId.trimmed().isEmpty())
        {
            return false;
        }

        for (const WhatSonNoteVersionSnapshot& snapshot : state.snapshots)
        {
            if (snapshot.snapshotId == snapshotId)
            {
                return true;
            }
        }
        return false;
    }

    QString latestSnapshotId(const WhatSonNoteVersionState& state)
    {
        return state.snapshots.isEmpty() ? QString() : state.snapshots.constLast().snapshotId;
    }
} // namespace

int VersionLimitManager::maximumSnapshotCount() noexcept
{
    return kMaximumSnapshotCount;
}

int VersionLimitManager::pruneOldestSnapshots(WhatSonNoteVersionState* state)
{
    if (state == nullptr)
    {
        return 0;
    }

    const int overflow = static_cast<int>(state->snapshots.size()) - kMaximumSnapshotCount;
    if (overflow <= 0)
    {
        return 0;
    }

    state->snapshots.erase(state->snapshots.begin(), state->snapshots.begin() + overflow);
    if (!state->snapshots.isEmpty())
    {
        state->snapshots.first().parentSnapshotId.clear();
    }

    const QString fallbackSnapshotId = latestSnapshotId(*state);
    if (!containsSnapshotId(*state, state->headSnapshotId))
    {
        state->headSnapshotId = fallbackSnapshotId;
    }
    if (!containsSnapshotId(*state, state->currentSnapshotId))
    {
        state->currentSnapshotId = state->headSnapshotId;
    }

    return overflow;
}
