#include "WhatSonNoteHeaderCreator.hpp"

#include "WhatSonBookmarkColorPalette.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QFileInfo>

#include <utility>

namespace
{
    QString escapeXmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString boolToText(bool value)
    {
        return value ? QStringLiteral("true") : QStringLiteral("false");
    }
} // namespace

WhatSonNoteHeaderCreator::WhatSonNoteHeaderCreator(QString workspaceRootPath, QString notesRootPath)
    : WhatSonNoteCreator(std::move(workspaceRootPath), std::move(notesRootPath))
{
}

WhatSonNoteHeaderCreator::~WhatSonNoteHeaderCreator() = default;

QString WhatSonNoteHeaderCreator::creatorName() const
{
    return QStringLiteral("WhatSonNoteHeaderCreator");
}

QString WhatSonNoteHeaderCreator::targetPathForNote(const QString& noteId) const
{
    const QString noteDirPath = noteDirectoryPath(noteId);
    const QString noteStem = QFileInfo(noteDirPath).completeBaseName();
    const QString fileName = noteStem + QStringLiteral(".wsnhead");
    const QString targetPath = joinPath(noteDirPath, fileName);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.creator.header"),
                              QStringLiteral("targetPathForNote"),
                              QStringLiteral("noteId=%1 path=%2").arg(noteId, targetPath));
    return targetPath;
}

QStringList WhatSonNoteHeaderCreator::requiredRelativePaths() const
{
    return {metadataDirectoryName()};
}

QString WhatSonNoteHeaderCreator::headerFileName() const
{
    return QStringLiteral("note.wsnhead");
}

QString WhatSonNoteHeaderCreator::metadataDirectoryName() const
{
    return QStringLiteral(".meta");
}

QString WhatSonNoteHeaderCreator::createHeaderText(const WhatSonNoteHeaderStore& store) const
{
    const QString noteId = store.noteId().trimmed().isEmpty()
                               ? QStringLiteral("note")
                               : escapeXmlText(store.noteId().trimmed());

    QString text;
    text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
    text += QStringLiteral("<contents id=\"") + noteId + QStringLiteral("\">\n");
    text += QStringLiteral("  <head>\n");
    text += QStringLiteral("    <meta charset=\"UTF-8\" />\n");
    text += QStringLiteral("    <meta name=\"wsn-type\" content=\"wsnhead\" />\n");
    text += QStringLiteral("    <created>") + escapeXmlText(store.createdAt()) + QStringLiteral("</created>\n");
    text += QStringLiteral("    <author>") + escapeXmlText(store.author()) + QStringLiteral("</author>\n");
    text += QStringLiteral("    <lastModified>") + escapeXmlText(store.lastModifiedAt())
        + QStringLiteral("</lastModified>\n");
    text += QStringLiteral("    <modifiedBy>") + escapeXmlText(store.modifiedBy())
        + QStringLiteral("</modifiedBy>\n");

    text += QStringLiteral("    <folders>\n");
    const QStringList folders = store.folders();
    const QStringList folderUuids = store.folderUuids();
    for (int index = 0; index < folders.size(); ++index)
    {
        const QString folder = folders.at(index);
        QString folderTag = QStringLiteral("      <folder");
        if (index < folderUuids.size() && !folderUuids.at(index).trimmed().isEmpty())
        {
            folderTag += QStringLiteral(" uuid=\"") + escapeXmlText(folderUuids.at(index).trimmed())
                + QStringLiteral("\"");
        }
        folderTag += QStringLiteral(">")
            + escapeXmlText(folder)
            + QStringLiteral("</folder>\n");
        text += folderTag;
    }
    text += QStringLiteral("    </folders>\n");

    text += QStringLiteral("    <project>") + escapeXmlText(store.project()) + QStringLiteral("</project>\n");
    QString bookmarkTag = QStringLiteral("    <bookmarks state=\"") + boolToText(store.isBookmarked()) +
        QStringLiteral("\"");
    const QString colorsAttribute = WhatSon::Bookmarks::serializeBookmarkColorsAttribute(store.bookmarkColors());
    if (!colorsAttribute.isEmpty())
    {
        bookmarkTag += QStringLiteral(" colors=\"") + escapeXmlText(colorsAttribute) + QStringLiteral("\"");
    }
    bookmarkTag += QStringLiteral(" />\n");
    text += bookmarkTag;

    text += QStringLiteral("    <tags>\n");
    for (const QString& tag : store.tags())
    {
        text += QStringLiteral("      <tag>") + escapeXmlText(tag) + QStringLiteral("</tag>\n");
    }
    text += QStringLiteral("    </tags>\n");

    text += QStringLiteral("    <progress enums=\"{Ready,Pending,InProgress,Done}\">")
        + QString::number(store.progress()) + QStringLiteral("</progress>\n");
    text += QStringLiteral("    <isPreset>") + boolToText(store.isPreset()) + QStringLiteral("</isPreset>\n");
    text += QStringLiteral("  </head>\n");
    text += QStringLiteral("</contents>\n");

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("note.creator.header"),
                              QStringLiteral("createHeaderText"),
                              QStringLiteral("id=%1 folderCount=%2 tagCount=%3 progress=%4 bytes=%5")
                              .arg(store.noteId())
                              .arg(store.folders().size())
                              .arg(store.tags().size())
                              .arg(store.progress())
                              .arg(text.toUtf8().size()));
    return text;
}
