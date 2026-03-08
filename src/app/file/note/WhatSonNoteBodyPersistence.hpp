#pragma once

#include <QString>

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text);
    QString firstLineFromBodyPlainText(const QString& text);
    QString resolveBodyPath(const QString& noteDirectoryPath);
    QString resolveHeaderPath(const QString& noteHeaderPath, const QString& noteDirectoryPath);
    bool persistBodyPlainText(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& noteHeaderPath,
        const QString& bodyPlainText,
        QString* outNormalizedBodyText = nullptr,
        QString* outLastModifiedAt = nullptr,
        QString* errorMessage = nullptr);
} // namespace WhatSon::NoteBodyPersistence
