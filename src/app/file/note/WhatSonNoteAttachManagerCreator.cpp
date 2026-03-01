#include "WhatSonNoteAttachManagerCreator.hpp"

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
    return joinPath(noteDirPath, attachmentManifestFileName());
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
