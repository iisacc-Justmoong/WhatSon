#include "LibraryDraft.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDebug>
#include <utility>

namespace
{
    bool hasFolderValue(const QStringList& folders)
    {
        for (const QString& folder : folders)
        {
            if (!folder.trimmed().isEmpty())
            {
                return true;
            }
        }
        return false;
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
        if (!hasFolderValue(record.folders))
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
