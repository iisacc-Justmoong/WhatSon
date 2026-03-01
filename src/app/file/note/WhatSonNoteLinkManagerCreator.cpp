#include "WhatSonNoteLinkManagerCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <utility>

WhatSonNoteLinkManagerCreator::WhatSonNoteLinkManagerCreator(
    QString workspaceRootPath,
    QString notesRootPath)
    : WhatSonNoteCreator(std::move(workspaceRootPath), std::move(notesRootPath))
{
}

WhatSonNoteLinkManagerCreator::~WhatSonNoteLinkManagerCreator() = default;

QString WhatSonNoteLinkManagerCreator::creatorName() const
{
    return QStringLiteral("WhatSonNoteLinkManagerCreator");
}

QString WhatSonNoteLinkManagerCreator::targetPathForNote(const QString& noteId) const
{
    const QString noteDirPath = noteDirectoryPath(noteId);
    const QString targetPath = joinPath(noteDirPath, linksFileName());
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.creator.link"),
                              QStringLiteral("targetPathForNote"),
                              QStringLiteral("noteId=%1 path=%2").arg(noteId, targetPath));
    return targetPath;
}

QStringList WhatSonNoteLinkManagerCreator::requiredRelativePaths() const
{
    return {};
}

QString WhatSonNoteLinkManagerCreator::linksFileName() const
{
    return QStringLiteral("links.wsnlink");
}

QString WhatSonNoteLinkManagerCreator::backlinksFileName() const
{
    return QStringLiteral("backlinks.wsnlink");
}
