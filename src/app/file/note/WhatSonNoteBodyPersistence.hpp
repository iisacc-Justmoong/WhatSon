#pragma once

#include <QString>
#include <QStringList>

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text);
    QString serializeBodyDocument(const QString& noteId, const QString& bodySourceText);
    QStringList extractedInlineTagValues(const QString& bodySourceText);
    QString plainTextFromBodyDocument(const QString& bodyDocumentText);
    QString sourceTextFromBodyDocument(const QString& bodyDocumentText);
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
        QString* outNormalizedBodySourceText = nullptr,
        QString* outLastModifiedAt = nullptr,
        QString* errorMessage = nullptr);
} // namespace WhatSon::NoteBodyPersistence
