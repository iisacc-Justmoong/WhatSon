#include "app/models/file/note/WhatSonNoteBodyCreator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

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
    const QString targetPath = joinPath(noteDirPath, fileName);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.creator.body"),
                              QStringLiteral("targetPathForNote"),
                              QStringLiteral("noteId=%1 path=%2").arg(noteId, targetPath));
    return targetPath;
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
