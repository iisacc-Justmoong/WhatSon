#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace WhatSon::UnusedNoteSensorSupport
{
    QVariantList collectUnusedNoteEntries(
        const QString& hubPath,
        const QDateTime& referenceUtc,
        const QDateTime& cutoffUtc,
        QString* errorMessage);

    QStringList noteIdsFromEntries(const QVariantList& unusedNotes);
} // namespace WhatSon::UnusedNoteSensorSupport
