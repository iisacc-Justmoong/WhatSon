#pragma once

#include <QString>

class WhatSonTimestampConflictResolver final
{
public:
    struct MergeRequest final
    {
        QString baseLastModifiedAt;
        QString filesystemLastModifiedAt;
        QString incomingLastModifiedAt;
        QString filesystemBodySourceText;
        QString incomingBodySourceText;
    };

    struct MergeResult final
    {
        QString mergedBodySourceText;
        QString winner;
        QString winningLastModifiedAt;
        bool conflictDetected = false;
    };

    MergeResult mergeBodyByTimestamp(const MergeRequest& request) const;
};
