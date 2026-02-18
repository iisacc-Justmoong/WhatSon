#pragma once

#include "WhatSonNoteCreator.hpp"

class WhatSonNoteBodyCreator : public WhatSonNoteCreator
{
public:
    explicit WhatSonNoteBodyCreator(QString workspaceRootPath, QString notesRootPath = QStringLiteral("notes"));
    ~WhatSonNoteBodyCreator() override;

    QString creatorName() const override;
    QString targetPathForNote(const QString& noteId) const override;
    QStringList requiredRelativePaths() const override;

    QString bodyFileName() const;
    QString draftBodyFileName() const;
};
