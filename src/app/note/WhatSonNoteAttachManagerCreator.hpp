#pragma once

#include "WhatSonNoteCreator.hpp"

class WhatSonNoteAttachManagerCreator : public WhatSonNoteCreator
{
public:
    explicit WhatSonNoteAttachManagerCreator(QString workspaceRootPath,
                                             QString notesRootPath = QStringLiteral("notes"));
    ~WhatSonNoteAttachManagerCreator() override;

    QString creatorName() const override;
    QString targetPathForNote(const QString& noteId) const override;
    QStringList requiredRelativePaths() const override;

    QString attachmentDirectoryName() const;
    QString attachmentManifestFileName() const;
};
