#include "WhatSonNoteAttachManagerCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <utility>

WhatSonNoteAttachManagerCreator::WhatSonNoteAttachManagerCreator(
    QString workspaceRootPath,
    QString notesRootPath)
    : WhatSonNoteCreator(std::move(workspaceRootPath), std::move(notesRootPath))
{
}

WhatSonNoteAttachManagerCreator::~WhatSonNoteAttachManagerCreator() = default;

QString WhatSonNoteAttachManagerCreator::creatorName() const
{
    return QStringLiteral("WhatSonNoteAttachManagerCreator");
}

QString WhatSonNoteAttachManagerCreator::targetPathForNote(const QString& noteId) const
{
    const QString noteDirPath = noteDirectoryPath(noteId);
    const QString targetPath = joinPath(noteDirPath, attachmentManifestFileName());
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.creator.attach"),
                              QStringLiteral("targetPathForNote"),
                              QStringLiteral("noteId=%1 path=%2").arg(noteId, targetPath));
    return targetPath;
}

QStringList WhatSonNoteAttachManagerCreator::requiredRelativePaths() const
{
    return {attachmentDirectoryName()};
}

QString WhatSonNoteAttachManagerCreator::attachmentDirectoryName() const
{
    return QStringLiteral("attachments");
}

QString WhatSonNoteAttachManagerCreator::attachmentManifestFileName() const
{
    return QStringLiteral("attachments.wsnpaint");
}
