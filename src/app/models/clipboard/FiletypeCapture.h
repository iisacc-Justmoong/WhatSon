#pragma once

#include <QString>

namespace WhatSon::Clipboard::FiletypeCapture
{
    QString normalizeMimeType(QString value);
    bool mimeTypeLooksLikeImagePayload(const QString& mimeType);
    QString formatFromMimeType(const QString& mimeType);
    QString defaultResourceFileName(const QString& format);
    QString normalizedFileNameOrDefault(const QString& fileName, const QString& format);
    QString normalizedFormatForFileType(const QString& fileName, const QString& mimeType);
} // namespace WhatSon::Clipboard::FiletypeCapture
