#include "app/models/file/hierarchy/library/LibraryToday.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDateTime>
#include <QDebug>
#include <utility>

namespace
{
    QDate parseDate(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QList<QString> formats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-dd")
        };

        for (const QString& format : formats)
        {
            const QDateTime dateTime = QDateTime::fromString(trimmed, format);
            if (dateTime.isValid())
            {
                return dateTime.date();
            }

            const QDate dateOnly = QDate::fromString(trimmed, format);
            if (dateOnly.isValid())
            {
                return dateOnly;
            }
        }

        const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
        if (isoDateTime.isValid())
        {
            return isoDateTime.date();
        }

        const QDateTime isoDateTimeWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoDateTimeWithMs.isValid())
        {
            return isoDateTimeWithMs.date();
        }

        return {};
    }
} // namespace

LibraryToday::LibraryToday()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.today"), QStringLiteral("ctor"));
}

LibraryToday::~LibraryToday()
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.today"), QStringLiteral("dtor"));
}

bool LibraryToday::rebuild(const QVector<LibraryNoteRecord>& allNotes, const QDate& today)
{
    m_notes.clear();
    m_notes.reserve(allNotes.size());

    for (const LibraryNoteRecord& record : allNotes)
    {
        if (matches(record, today))
        {
            m_notes.push_back(record);
        }
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.today"),
                              QStringLiteral("rebuild"),
                              QStringLiteral("sourceCount=%1 todayCount=%2 date=%3")
                              .arg(allNotes.size())
                              .arg(m_notes.size())
                              .arg(today.toString(QStringLiteral("yyyy-MM-dd"))));

    if (WhatSon::Debug::isEnabled())
    {
        for (const LibraryNoteRecord& record : m_notes)
        {
            qWarning().noquote()
                << QStringLiteral(
                    "[wsnindex:today] id=%1 firstLine=%2 created=%3 modified=%4")
                .arg(record.noteId, record.bodyFirstLine, record.createdAt, record.lastModifiedAt);
        }
    }

    return true;
}

bool LibraryToday::matches(const LibraryNoteRecord& note, const QDate& today)
{
    const QDate createdDate = parseDate(note.createdAt);
    const QDate modifiedDate = parseDate(note.lastModifiedAt);
    return (createdDate.isValid() && createdDate == today)
        || (modifiedDate.isValid() && modifiedDate == today);
}

bool LibraryToday::upsertNote(const LibraryNoteRecord& note, const QDate& today)
{
    const QString normalizedNoteId = note.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    if (!matches(note, today))
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

bool LibraryToday::removeNoteById(const QString& noteId)
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

void LibraryToday::setNotes(QVector<LibraryNoteRecord> notes)
{
    m_notes = std::move(notes);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.today"),
                              QStringLiteral("setNotes"),
                              QStringLiteral("count=%1").arg(m_notes.size()));
}

void LibraryToday::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.today"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_notes.size()));
    m_notes.clear();
}

const QVector<LibraryNoteRecord>& LibraryToday::notes() const noexcept
{
    return m_notes;
}
