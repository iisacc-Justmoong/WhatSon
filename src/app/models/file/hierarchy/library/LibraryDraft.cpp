#include "LibraryDraft.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "note/WhatSonNoteFolderSemantics.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
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

    QString resolveNoteHeaderPath(const LibraryNoteRecord& note)
    {
        const QString directPath = normalizePath(note.noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString noteDirectoryPath = normalizePath(note.noteDirectoryPath);
        if (noteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(noteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(noteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDir.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDir.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        return {};
    }

    bool readUtf8File(const QString& filePath, QString* outText)
    {
        if (outText == nullptr)
        {
            return false;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }

        *outText = QString::fromUtf8(file.readAll());
        return true;
    }

    bool headerDeclaresDraftMembership(const LibraryNoteRecord& record)
    {
        const QString headerPath = resolveNoteHeaderPath(record);
        if (headerPath.isEmpty())
        {
            return false;
        }

        QString rawHeaderText;
        if (!readUtf8File(headerPath, &rawHeaderText))
        {
            return false;
        }

        const WhatSon::NoteFolders::RawFoldersBlockState foldersState =
            WhatSon::NoteFolders::inspectRawFoldersBlock(rawHeaderText);
        return foldersState.blockPresent && !foldersState.hasConcreteEntry;
    }
} // namespace

LibraryDraft::LibraryDraft()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.draft"), QStringLiteral("ctor"));
}

LibraryDraft::~LibraryDraft()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.draft"), QStringLiteral("dtor"));
}

bool LibraryDraft::rebuild(const QVector<LibraryNoteRecord>& allNotes)
{
    m_notes.clear();
    m_notes.reserve(allNotes.size());

    for (const LibraryNoteRecord& record : allNotes)
    {
        if (matches(record))
        {
            m_notes.push_back(record);
        }
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.draft"),
                              QStringLiteral("rebuild"),
                              QStringLiteral("sourceCount=%1 draftCount=%2").arg(allNotes.size()).arg(m_notes.size()));

    if (WhatSon::Debug::isEnabled())
    {
        for (const LibraryNoteRecord& record : m_notes)
        {
            qWarning().noquote()
                << QStringLiteral(
                    "[wsnindex:draft] id=%1 firstLine=%2 folders=[%3]")
                .arg(record.noteId, record.bodyFirstLine, record.folders.join(QStringLiteral(", ")));
        }
    }

    return true;
}

bool LibraryDraft::matches(const LibraryNoteRecord& note)
{
    return headerDeclaresDraftMembership(note);
}

bool LibraryDraft::upsertNote(const LibraryNoteRecord& note)
{
    const QString normalizedNoteId = note.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    if (!matches(note))
    {
        return removeNoteById(normalizedNoteId);
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
        return true;
    }

    m_notes.push_back(normalizedNote);
    return true;
}

bool LibraryDraft::removeNoteById(const QString& noteId)
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
        return true;
    }

    return false;
}

void LibraryDraft::setNotes(QVector<LibraryNoteRecord> notes)
{
    m_notes = std::move(notes);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.draft"),
                              QStringLiteral("setNotes"),
                              QStringLiteral("count=%1").arg(m_notes.size()));
}

void LibraryDraft::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.draft"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_notes.size()));
    m_notes.clear();
}

const QVector<LibraryNoteRecord>& LibraryDraft::notes() const noexcept
{
    return m_notes;
}
