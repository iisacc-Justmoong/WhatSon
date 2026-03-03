#include "LibraryToday.hpp"

#include "WhatSonDebugTrace.hpp"

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
        const QDate createdDate = parseDate(record.createdAt);
        const QDate modifiedDate = parseDate(record.lastModifiedAt);
        if ((createdDate.isValid() && createdDate == today)
            || (modifiedDate.isValid() && modifiedDate == today))
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
                    "[wsnindex:today] id=%1 title=%2 created=%3 modified=%4")
                .arg(record.noteId, record.title, record.createdAt, record.lastModifiedAt);
        }
    }

    return true;
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
