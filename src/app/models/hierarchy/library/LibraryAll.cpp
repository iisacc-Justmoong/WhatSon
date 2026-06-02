#include "app/models/hierarchy/library/LibraryAll.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFileInfo>

#include <utility>

namespace
{
    QString normalizePath(const QString& input)
    {
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }
} // namespace

LibraryAll::LibraryAll()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.all"), QStringLiteral("ctor"));
}

LibraryAll::~LibraryAll()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.all"), QStringLiteral("dtor"));
}

bool LibraryAll::indexFromWshub(const QString& wshubPath, QString* errorMessage)
{
    clear();

    const QString normalizedHubPath = normalizePath(wshubPath);
    if (normalizedHubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must not be empty.");
        }
        return false;
    }

    const QFileInfo hubInfo(normalizedHubPath);
    if (!hubInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(normalizedHubPath);
        }
        return false;
    }
    if (!hubInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(
                normalizedHubPath);
        }
        return false;
    }
    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must end with .wshub: %1").arg(normalizedHubPath);
        }
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("index.begin"),
                              QStringLiteral("path=%1").arg(normalizedHubPath));

    const QStringList libraryRoots = m_hubStructureValidator.resolveLibraryRoots(normalizedHubPath);
    if (libraryRoots.isEmpty())
    {
        m_sourceWshubPath = normalizedHubPath;
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No Library.wslibrary directory found inside: %1").arg(
                normalizedHubPath);
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.all"),
                                  QStringLiteral("index.noLibraryRoot"),
                                  QStringLiteral("path=%1").arg(normalizedHubPath));
        return false;
    }

    m_sourceWshubPath = normalizedHubPath;
    m_notes.clear();

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("index.notePackagesDisabled"),
                              QStringLiteral("path=%1 noteCount=%2").arg(m_sourceWshubPath).arg(m_notes.size()));
    return true;
}

void LibraryAll::setIndexedNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes)
{
    m_sourceWshubPath = normalizePath(sourceWshubPath);
    m_notes = std::move(notes);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("setIndexedNotes"),
                              QStringLiteral("path=%1 noteCount=%2").arg(m_sourceWshubPath).arg(m_notes.size()));
}

void LibraryAll::setSourceWshubPath(QString sourceWshubPath)
{
    m_sourceWshubPath = normalizePath(sourceWshubPath);
}

bool LibraryAll::upsertNote(const LibraryNoteRecord& note)
{
    const QString normalizedNoteId = note.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    LibraryNoteRecord normalizedNote = note;
    normalizedNote.noteId = normalizedNoteId;

    for (int index = 0; index < m_notes.size(); ++index)
    {
        if (m_notes.at(index).noteId.trimmed() != normalizedNoteId)
        {
            continue;
        }

        if (m_notes.at(index) == normalizedNote)
        {
            return false;
        }

        m_notes[index] = normalizedNote;
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.all"),
                                  QStringLiteral("upsertNote.update"),
                                  QStringLiteral("noteId=%1 count=%2").arg(normalizedNoteId).arg(m_notes.size()));
        return true;
    }

    m_notes.push_back(normalizedNote);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("upsertNote.insert"),
                              QStringLiteral("noteId=%1 count=%2").arg(normalizedNoteId).arg(m_notes.size()));
    return true;
}

bool LibraryAll::removeNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    for (auto it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        if (it->noteId.trimmed() != normalizedNoteId)
        {
            continue;
        }

        m_notes.erase(it);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.all"),
                                  QStringLiteral("removeNoteById"),
                                  QStringLiteral("noteId=%1 count=%2").arg(normalizedNoteId).arg(m_notes.size()));
        return true;
    }

    return false;
}

bool LibraryAll::noteById(const QString& noteId, LibraryNoteRecord* outNote) const
{
    if (outNote == nullptr)
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    for (const LibraryNoteRecord& note : m_notes)
    {
        if (note.noteId.trimmed() != normalizedNoteId)
        {
            continue;
        }

        *outNote = note;
        return true;
    }

    return false;
}

void LibraryAll::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.all"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_notes.size()));
    m_sourceWshubPath.clear();
    m_notes.clear();
}

QString LibraryAll::sourceWshubPath() const
{
    return m_sourceWshubPath;
}

const QVector<LibraryNoteRecord>& LibraryAll::notes() const noexcept
{
    return m_notes;
}
