#pragma once

#include <QString>

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text);
    QString plainTextFromBodyDocument(const QString& bodyDocumentText);
    QString richTextFromBodyDocument(const QString& bodyDocumentText);
    QString firstLineFromBodyDocument(const QString& bodyDocumentText);
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
