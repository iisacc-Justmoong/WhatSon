#pragma once

#include <QString>

class WhatSonTimestampConflictResolver final
{
public:
    bool isTimestampNewer(
        const QString& candidateLastModifiedAt,
        const QString& baselineLastModifiedAt) const;
};
