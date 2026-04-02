#include "CalendarBoardStore.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <QUuid>
#include <QStringList>

#include <algorithm>

namespace
{
    constexpr auto kCalendarBoardScope = "calendar.board";
}

CalendarBoardStore::CalendarBoardStore(QObject* parent)
    : ICalendarBoardStore(parent)
{
}

CalendarBoardStore::~CalendarBoardStore() = default;

bool CalendarBoardStore::addEvent(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    return addEntry(EntryType::Event, dateIso, timeText, title, detail);
}

bool CalendarBoardStore::addTask(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    return addEntry(EntryType::Task, dateIso, timeText, title, detail);
}

QVariantList CalendarBoardStore::entriesForDate(const QString& dateIso) const
{
    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        return {};
    }

    QVariantList entries;
    entries.reserve(m_entries.size());
    for (const CalendarEntry& entry : m_entries)
    {
        if (entry.date == parsedDate)
        {
            entries.push_back(toVariantMap(entry));
        }
    }

    return entries;
}

QVariantMap CalendarBoardStore::countsForDate(const QString& dateIso) const
{
    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        return {
            {QStringLiteral("eventCount"), 0},
            {QStringLiteral("taskCount"), 0},
            {QStringLiteral("entryCount"), 0}
        };
    }

    int eventCount = 0;
    int taskCount = 0;
    for (const CalendarEntry& entry : m_entries)
    {
        if (entry.date != parsedDate)
        {
            continue;
        }

        if (entry.type == EntryType::Event)
        {
            eventCount += 1;
            continue;
        }

        taskCount += 1;
    }

    return {
        {QStringLiteral("eventCount"), eventCount},
        {QStringLiteral("taskCount"), taskCount},
        {QStringLiteral("entryCount"), eventCount + taskCount}
    };
}

bool CalendarBoardStore::removeEntry(const QString& entryId)
{
    const QString normalizedEntryId = entryId.trimmed();
    if (normalizedEntryId.isEmpty())
    {
        return false;
    }

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->id != normalizedEntryId)
        {
            continue;
        }

        m_entries.erase(it);
        emit entriesChanged();
        emit entryRemoved(normalizedEntryId);
        return true;
    }

    return false;
}

bool CalendarBoardStore::setTaskCompleted(const QString& entryId, bool completed)
{
    const QString normalizedEntryId = entryId.trimmed();
    if (normalizedEntryId.isEmpty())
    {
        return false;
    }

    for (CalendarEntry& entry : m_entries)
    {
        if (entry.id != normalizedEntryId || entry.type != EntryType::Task)
        {
            continue;
        }

        if (entry.completed == completed)
        {
            return true;
        }

        entry.completed = completed;
        emit entriesChanged();
        emit entryUpdated(normalizedEntryId);
        return true;
    }

    return false;
}

bool CalendarBoardStore::addEntry(
    EntryType type,
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    const QString normalizedTitle = title.trimmed();
    if (normalizedTitle.isEmpty())
    {
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kCalendarBoardScope),
            QStringLiteral("addEntry.rejected"),
            QStringLiteral("reason=empty-title type=%1").arg(entryTypeName(type)));
        return false;
    }

    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kCalendarBoardScope),
            QStringLiteral("addEntry.rejected"),
            QStringLiteral("reason=invalid-date type=%1 date=%2")
            .arg(entryTypeName(type), dateIso));
        return false;
    }

    QTime parsedTime;
    if (!parseTimeText(timeText, &parsedTime))
    {
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kCalendarBoardScope),
            QStringLiteral("addEntry.rejected"),
            QStringLiteral("reason=invalid-time type=%1 time=%2")
            .arg(entryTypeName(type), timeText));
        return false;
    }

    CalendarEntry entry;
    entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    entry.type = type;
    entry.date = parsedDate;
    entry.time = parsedTime;
    entry.title = normalizedTitle;
    entry.detail = detail.trimmed();
    entry.completed = false;
    m_entries.push_back(entry);
    sortEntries();

    emit entriesChanged();
    emit entryAdded(
        entry.id,
        entryTypeName(entry.type),
        entry.date.toString(Qt::ISODate),
        entry.time.toString(QStringLiteral("HH:mm")));
    return true;
}

bool CalendarBoardStore::parseIsoDate(const QString& dateIso, QDate* outDate)
{
    if (!outDate)
    {
        return false;
    }

    const QDate date = QDate::fromString(dateIso.trimmed(), Qt::ISODate);
    if (!date.isValid())
    {
        return false;
    }

    *outDate = date;
    return true;
}

bool CalendarBoardStore::parseTimeText(const QString& timeText, QTime* outTime)
{
    if (!outTime)
    {
        return false;
    }

    const QString trimmedTime = timeText.trimmed();
    if (trimmedTime.isEmpty())
    {
        return false;
    }

    const QStringList formats{
        QStringLiteral("HH:mm"),
        QStringLiteral("HH:mm:ss"),
        QStringLiteral("hh:mm AP"),
        QStringLiteral("h:mm AP")
    };

    for (const QString& format : formats)
    {
        const QTime parsedTime = QTime::fromString(trimmedTime, format);
        if (!parsedTime.isValid())
        {
            continue;
        }

        *outTime = parsedTime;
        return true;
    }

    const QTime isoTime = QTime::fromString(trimmedTime, Qt::ISODate);
    if (!isoTime.isValid())
    {
        return false;
    }

    *outTime = isoTime;
    return true;
}

QString CalendarBoardStore::entryTypeName(EntryType type)
{
    switch (type)
    {
    case EntryType::Event:
        return QStringLiteral("event");
    case EntryType::Task:
        return QStringLiteral("task");
    }

    return QStringLiteral("event");
}

QVariantMap CalendarBoardStore::toVariantMap(const CalendarEntry& entry)
{
    return {
        {QStringLiteral("id"), entry.id},
        {QStringLiteral("type"), entryTypeName(entry.type)},
        {QStringLiteral("date"), entry.date.toString(Qt::ISODate)},
        {QStringLiteral("time"), entry.time.toString(QStringLiteral("HH:mm"))},
        {QStringLiteral("title"), entry.title},
        {QStringLiteral("detail"), entry.detail},
        {QStringLiteral("completed"), entry.completed}
    };
}

void CalendarBoardStore::sortEntries()
{
    std::sort(
        m_entries.begin(),
        m_entries.end(),
        [](const CalendarEntry& left, const CalendarEntry& right)
        {
            if (left.date != right.date)
            {
                return left.date < right.date;
            }
            if (left.time != right.time)
            {
                return left.time < right.time;
            }
            if (left.type != right.type)
            {
                return left.type < right.type;
            }
            return left.title < right.title;
        });
}
