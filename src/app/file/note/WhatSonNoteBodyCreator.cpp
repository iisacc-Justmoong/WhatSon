#include "WhatSonNoteBodyCreator.hpp"

#include <QFileInfo>

#include <utility>

WhatSonNoteBodyCreator::WhatSonNoteBodyCreator(QString workspaceRootPath, QString notesRootPath)
    : WhatSonNoteCreator(std::move(workspaceRootPath), std::move(notesRootPath))
{
}

WhatSonNoteBodyCreator::~WhatSonNoteBodyCreator() = default;

QString WhatSonNoteBodyCreator::creatorName() const
{
    return QStringLiteral("WhatSonNoteBodyCreator");
}

QString WhatSonNoteBodyCreator::targetPathForNote(const QString& noteId) const
{
    const QString noteDirPath = noteDirectoryPath(noteId);
    const QString noteStem = QFileInfo(noteDirPath).completeBaseName();
    const QString fileName = noteStem + QStringLiteral(".wsnbody");
    return joinPath(noteDirPath, fileName);
}

QStringList WhatSonNoteBodyCreator::requiredRelativePaths() const
{
    return {};
}

QString WhatSonNoteBodyCreator::bodyFileName() const
{
    return QStringLiteral("note.wsnbody");
}

QString WhatSonNoteBodyCreator::draftBodyFileName() const
{
    return QStringLiteral("note.draft.wsnbody");
}
