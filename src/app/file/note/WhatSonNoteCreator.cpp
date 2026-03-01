#include "WhatSonNoteCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <utility>

namespace
{
    QString normalizeNoteStem(QString noteId)
    {
        QString stem = QFileInfo(noteId.trimmed()).completeBaseName();
        if (stem.isEmpty())
        {
            stem = QFileInfo(noteId.trimmed()).fileName();
        }

        stem = stem.trimmed();
        stem.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
        stem.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral(""));

        if (stem.isEmpty())
        {
            return QStringLiteral("untitled-note");
        }

        return stem;
    }

    QString normalizeNoteDirectoryName(const QString& noteId)
    {
        return normalizeNoteStem(noteId) + QStringLiteral(".wsnote");
    }
} // namespace

WhatSonNoteCreator::WhatSonNoteCreator(QString workspaceRootPath, QString notesRootPath)
    : m_workspaceRootPath(std::move(workspaceRootPath)),
      m_notesRootPath(std::move(notesRootPath))
{
    WhatSon::Debug::trace(
        QStringLiteral("note.creator.base"),
        QStringLiteral("ctor"),
        QStringLiteral("workspace=%1 notesRoot=%2").arg(m_workspaceRootPath, m_notesRootPath));
}

WhatSonNoteCreator::~WhatSonNoteCreator() = default;

void WhatSonNoteCreator::setWorkspaceRootPath(QString workspaceRootPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("note.creator.base"),
        QStringLiteral("setWorkspaceRootPath"),
        QStringLiteral("value=%1").arg(workspaceRootPath));
    m_workspaceRootPath = std::move(workspaceRootPath);
}

const QString& WhatSonNoteCreator::workspaceRootPath() const noexcept
{
    return m_workspaceRootPath;
}

void WhatSonNoteCreator::setNotesRootPath(QString notesRootPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("note.creator.base"),
        QStringLiteral("setNotesRootPath"),
        QStringLiteral("value=%1").arg(notesRootPath));
    m_notesRootPath = std::move(notesRootPath);
}

const QString& WhatSonNoteCreator::notesRootPath() const noexcept
{
    return m_notesRootPath;
}

QString WhatSonNoteCreator::noteDirectoryPath(const QString& noteId) const
{
    const QString notesRootAbsolutePath = joinPath(workspaceRootPath(), notesRootPath());
    const QString result = joinPath(notesRootAbsolutePath, normalizeNoteDirectoryName(noteId));
    WhatSon::Debug::trace(
        QStringLiteral("note.creator.base"),
        QStringLiteral("noteDirectoryPath"),
        QStringLiteral("noteId=%1 result=%2").arg(noteId, result));
    return result;
}

QString WhatSonNoteCreator::joinPath(const QString& left, const QString& right) const
{
    if (left.isEmpty())
    {
        return QDir::cleanPath(right);
    }
    if (right.isEmpty())
    {
        return QDir::cleanPath(left);
    }

    return QDir::cleanPath(left + QLatin1Char('/') + right);
}
