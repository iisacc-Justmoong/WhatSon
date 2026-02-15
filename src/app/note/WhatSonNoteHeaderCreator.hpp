#pragma once

#include "WhatSonNoteCreator.hpp"

class WhatSonNoteHeaderCreator : public WhatSonNoteCreator
{
public:
    explicit WhatSonNoteHeaderCreator(QString workspaceRootPath, QString notesRootPath = QStringLiteral("notes"));
    ~WhatSonNoteHeaderCreator() override;

    QString creatorName() const override;
    QString targetPathForNote(const QString& noteId) const override;
    QStringList requiredRelativePaths() const override;

    QString headerFileName() const;
    QString metadataDirectoryName() const;
};
