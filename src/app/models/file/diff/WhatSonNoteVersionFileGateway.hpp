#pragma once

#include "app/models/file/note/local/WhatSonLocalNoteDocument.hpp"

#include <QString>

class WhatSonNoteVersionFileGateway final
{
public:
    QString noteIdFromDocument(const WhatSonLocalNoteDocument& document) const;
    QString versionPathFromDocument(const WhatSonLocalNoteDocument& document) const;
    QString headerPathFromDocument(const WhatSonLocalNoteDocument& document) const;
    QString bodyPathFromDocument(const WhatSonLocalNoteDocument& document) const;

    bool readUtf8File(
        const QString& path,
        QString* outText,
        QString* errorMessage = nullptr) const;
    bool writeUtf8File(
        const QString& path,
        const QString& text,
        QString* errorMessage = nullptr) const;
    bool ensureVersionDocument(
        const QString& versionFilePath,
        const QString& emptyVersionText,
        QString* errorMessage = nullptr) const;
};
