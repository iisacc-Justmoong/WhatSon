#include "WhatSonNoteLinkManagerCreator.hpp"

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
    return joinPath(noteDirPath, linksFileName());
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
