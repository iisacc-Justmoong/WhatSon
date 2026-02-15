#pragma once

#include "WhatSonNoteCreator.hpp"

class WhatSonNoteLinkManagerCreator : public WhatSonNoteCreator
{
public:
    explicit WhatSonNoteLinkManagerCreator(QString workspaceRootPath, QString notesRootPath = QStringLiteral("notes"));
    ~WhatSonNoteLinkManagerCreator() override;

    QString creatorName() const override;
    QString targetPathForNote(const QString& noteId) const override;
    QStringList requiredRelativePaths() const override;

    QString linksFileName() const;
    QString backlinksFileName() const;
};
