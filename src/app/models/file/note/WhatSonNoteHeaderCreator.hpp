#pragma once

#include "app/models/file/note/WhatSonNoteCreator.hpp"
#include "app/models/file/note/WhatSonNoteHeaderStore.hpp"

class WhatSonNoteHeaderCreator : public WhatSonNoteCreator
{
public:
    explicit WhatSonNoteHeaderCreator(QString workspaceRootPath, QString notesRootPath = QStringLiteral("notes"));
    ~WhatSonNoteHeaderCreator() override;

    QString creatorName() const override;
    QString targetPathForNote(const QString& noteId) const override;
    QStringList requiredRelativePaths() const override;

    QString headerFileName() const;
    QString createHeaderText(const WhatSonNoteHeaderStore& store) const;
};
