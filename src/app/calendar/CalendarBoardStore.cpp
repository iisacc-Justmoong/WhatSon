#include "CalendarBoardStore.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/WhatSonLibraryIndexedState.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"

#include <QDateTime>
#include <QUuid>
#include <QStringList>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kCalendarBoardScope = "calendar.board";

    struct ParsedNoteTimestamp final
    {
        QDate date;
        QTime time;
        bool valid = false;
        bool hasExplicitTime = false;
    };

    ParsedNoteTimestamp parseNoteTimestamp(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QStringList dateTimeFormats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd-HH-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")
        };

        for (const QString& format : dateTimeFormats)
        {
            const QDateTime parsed = QDateTime::fromString(trimmed, format);
            if (!parsed.isValid())
            {
                continue;
            }

            ParsedNoteTimestamp timestamp;
            timestamp.date = parsed.date();
            timestamp.time = parsed.time();
            timestamp.valid = timestamp.date.isValid();
            timestamp.hasExplicitTime = timestamp.time.isValid();
            return timestamp;
        }

        const QDateTime isoWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoWithMs.isValid())
        {
            return {
                isoWithMs.date(),
                isoWithMs.time(),
                isoWithMs.date().isValid(),
                isoWithMs.time().isValid()
            };
        }

        const QDateTime iso = QDateTime::fromString(trimmed, Qt::ISODate);
        if (iso.isValid())
        {
            return {
                iso.date(),
                iso.time(),
                iso.date().isValid(),
                iso.time().isValid()
            };
        }

        const QStringList dateOnlyFormats{
            QStringLiteral("yyyy-MM-dd"),
            QStringLiteral("yyyy/MM/dd")
        };

        for (const QString& format : dateOnlyFormats)
        {
            const QDate parsedDate = QDate::fromString(trimmed, format);
            if (!parsedDate.isValid())
            {
                continue;
            }

            ParsedNoteTimestamp timestamp;
            timestamp.date = parsedDate;
            timestamp.valid = true;
            return timestamp;
        }

        return {};
    }

    QString firstLineFromBody(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        const int newlineIndex = value.indexOf(QLatin1Char('\n'));
        if (newlineIndex >= 0)
        {
            value = value.left(newlineIndex);
        }
        return value.trimmed();
    }

    QString noteDisplayTitle(const LibraryNoteRecord& note)
    {
        const QString firstLine = note.bodyFirstLine.trimmed();
        if (!firstLine.isEmpty())
        {
            return firstLine;
        }

        const QString bodyLine = firstLineFromBody(note.bodyPlainText);
        if (!bodyLine.isEmpty())
        {
            return bodyLine;
        }

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        return QStringLiteral("Untitled note");
    }

    QString noteLifecycleTitle(const QString& prefix, const QString& title)
    {
        return QStringLiteral("%1: %2").arg(prefix, title);
    }

    CalendarBoardStore::CalendarEntry buildProjectedNoteEntry(
        const LibraryNoteRecord& note,
        const ParsedNoteTimestamp& timestamp,
        QString lifecycleLabel,
        QString lifecycleKey)
    {
        CalendarBoardStore::CalendarEntry entry;
        entry.id = QStringLiteral("note:%1:%2:%3")
                       .arg(note.noteId.trimmed(),
                            lifecycleKey,
                            timestamp.date.toString(Qt::ISODate));
        entry.type = CalendarBoardStore::EntryType::Event;
        entry.date = timestamp.date;
        entry.time = timestamp.hasExplicitTime ? timestamp.time : QTime(0, 0);
        entry.title = noteLifecycleTitle(lifecycleLabel, noteDisplayTitle(note));
        entry.detail = note.noteId.trimmed();
        entry.allDay = !timestamp.hasExplicitTime;
        entry.readOnly = true;
        entry.projected = true;
        entry.sourceKind = QStringLiteral("note");
        entry.sourceId = note.noteId.trimmed();
        entry.dateRole = std::move(lifecycleKey);
        return entry;
    }

    bool calendarEntryEqual(
        const CalendarBoardStore::CalendarEntry& lhs,
        const CalendarBoardStore::CalendarEntry& rhs)
    {
        return lhs.id == rhs.id
            && lhs.type == rhs.type
            && lhs.date == rhs.date
            && lhs.time == rhs.time
            && lhs.title == rhs.title
            && lhs.detail == rhs.detail
            && lhs.completed == rhs.completed
            && lhs.allDay == rhs.allDay
            && lhs.readOnly == rhs.readOnly
            && lhs.projected == rhs.projected
            && lhs.sourceKind == rhs.sourceKind
            && lhs.sourceId == rhs.sourceId
            && lhs.dateRole == rhs.dateRole;
    }

    bool calendarEntryLessThan(
        const CalendarBoardStore::CalendarEntry& left,
        const CalendarBoardStore::CalendarEntry& right)
    {
        if (left.date != right.date)
        {
            return left.date < right.date;
        }
        if (left.allDay != right.allDay)
        {
            return left.allDay;
        }
        if (left.time.isValid() != right.time.isValid())
        {
            return !left.time.isValid();
        }
        if (left.time != right.time)
        {
            return left.time < right.time;
        }
        if (left.type != right.type)
        {
            return left.type < right.type;
        }
        if (left.projected != right.projected)
        {
            return !left.projected;
        }
        return left.title < right.title;
    }
}

CalendarBoardStore::CalendarBoardStore(QObject* parent)
    : ICalendarBoardStore(parent)
{
    m_projectedNotesReloadTimer.setSingleShot(true);
    m_projectedNotesReloadTimer.setInterval(250);
    connect(&m_projectedNotesReloadTimer, &QTimer::timeout, this, [this]()
    {
        reloadProjectedNotes();
    });
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

    QVector<CalendarEntry> matches;
    matches.reserve(m_entries.size() + m_projectedEntries.size());

    const auto appendMatches = [&parsedDate, &matches](const QVector<CalendarEntry>& source)
    {
        for (const CalendarEntry& entry : source)
        {
            if (entry.date == parsedDate)
            {
                matches.push_back(entry);
            }
        }
    };

    appendMatches(m_entries);
    appendMatches(m_projectedEntries);
    sortEntries(&matches);

    QVariantList entries;
    entries.reserve(matches.size());
    for (const CalendarEntry& entry : std::as_const(matches))
    {
        entries.push_back(toVariantMap(entry));
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
    const auto accumulateCounts = [&parsedDate, &eventCount, &taskCount](const QVector<CalendarEntry>& source)
    {
        for (const CalendarEntry& entry : source)
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
    };

    accumulateCounts(m_entries);
    accumulateCounts(m_projectedEntries);

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

void CalendarBoardStore::setProjectedNotesHubPath(const QString& wshubPath)
{
    const QString normalizedPath = WhatSon::HubPath::normalizePath(wshubPath);
    if (m_projectedNotesHubPath == normalizedPath)
    {
        return;
    }

    m_projectedNotesHubPath = normalizedPath;
    if (m_projectedNotesHubPath.isEmpty())
    {
        clearProjectedEntries();
    }
}

QString CalendarBoardStore::projectedNotesHubPath() const
{
    return m_projectedNotesHubPath;
}

void CalendarBoardStore::requestProjectedNotesReload()
{
    if (m_projectedNotesHubPath.isEmpty())
    {
        clearProjectedEntries();
        return;
    }

    m_projectedNotesReloadTimer.start();
}

bool CalendarBoardStore::reloadProjectedNotes(QString* errorMessage)
{
    if (m_projectedNotesHubPath.isEmpty())
    {
        clearProjectedEntries();
        return true;
    }

    WhatSonLibraryIndexedState indexedState;
    QString indexError;
    if (!indexedState.indexFromWshub(m_projectedNotesHubPath, &indexError))
    {
        WhatSon::Debug::traceSelf(
            this,
            QString::fromLatin1(kCalendarBoardScope),
            QStringLiteral("reloadProjectedNotes.failed"),
            QStringLiteral("path=%1 error=%2").arg(m_projectedNotesHubPath, indexError));
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        clearProjectedEntries();
        return false;
    }

    replaceProjectedEntries(buildProjectedNoteEntries(indexedState.allNotes()));
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kCalendarBoardScope),
        QStringLiteral("reloadProjectedNotes.success"),
        QStringLiteral("path=%1 noteCount=%2 projectedCount=%3")
            .arg(m_projectedNotesHubPath)
            .arg(indexedState.allNotes().size())
            .arg(m_projectedEntries.size()));
    return true;
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
    entry.allDay = (parsedTime.hour() == 0 && parsedTime.minute() == 0 && parsedTime.second() == 0);
    entry.readOnly = false;
    entry.projected = false;
    entry.sourceKind = QStringLiteral("board");
    m_entries.push_back(entry);
    sortEntries(&m_entries);

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
        {QStringLiteral("completed"), entry.completed},
        {QStringLiteral("allDay"), entry.allDay},
        {QStringLiteral("readOnly"), entry.readOnly},
        {QStringLiteral("sourceKind"), entry.sourceKind},
        {QStringLiteral("sourceId"), entry.sourceId},
        {QStringLiteral("dateRole"), entry.dateRole}
    };
}

void CalendarBoardStore::sortEntries(QVector<CalendarEntry>* entries)
{
    if (entries == nullptr)
    {
        return;
    }

    std::sort(entries->begin(), entries->end(), calendarEntryLessThan);
}

void CalendarBoardStore::clearProjectedEntries()
{
    if (m_projectedEntries.isEmpty())
    {
        return;
    }

    m_projectedEntries.clear();
    emit entriesChanged();
}

void CalendarBoardStore::replaceProjectedEntries(QVector<CalendarEntry> entries)
{
    sortEntries(&entries);
    if (entries.size() == m_projectedEntries.size())
    {
        bool unchanged = true;
        for (int index = 0; index < entries.size(); ++index)
        {
            if (!calendarEntryEqual(entries.at(index), m_projectedEntries.at(index)))
            {
                unchanged = false;
                break;
            }
        }
        if (unchanged)
        {
            return;
        }
    }

    m_projectedEntries = std::move(entries);
    emit entriesChanged();
}

QVector<CalendarBoardStore::CalendarEntry> CalendarBoardStore::buildProjectedNoteEntries(
    const QVector<LibraryNoteRecord>& notes) const
{
    QVector<CalendarEntry> entries;
    entries.reserve(notes.size() * 2);

    for (const LibraryNoteRecord& note : notes)
    {
        const QString noteId = note.noteId.trimmed();
        if (noteId.isEmpty())
        {
            continue;
        }

        const ParsedNoteTimestamp createdTimestamp = parseNoteTimestamp(note.createdAt);
        const ParsedNoteTimestamp modifiedTimestamp = parseNoteTimestamp(note.lastModifiedAt);

        if (!createdTimestamp.valid && !modifiedTimestamp.valid)
        {
            continue;
        }

        if (createdTimestamp.valid && modifiedTimestamp.valid && createdTimestamp.date == modifiedTimestamp.date)
        {
            const ParsedNoteTimestamp effectiveTimestamp =
                modifiedTimestamp.hasExplicitTime ? modifiedTimestamp : createdTimestamp;
            const QString combinedLabel =
                note.createdAt.trimmed() == note.lastModifiedAt.trimmed()
                    ? QStringLiteral("Created note")
                    : QStringLiteral("Created/modified note");
            entries.push_back(
                buildProjectedNoteEntry(
                    note,
                    effectiveTimestamp,
                    combinedLabel,
                    QStringLiteral("created-modified")));
            continue;
        }

        if (createdTimestamp.valid)
        {
            entries.push_back(
                buildProjectedNoteEntry(
                    note,
                    createdTimestamp,
                    QStringLiteral("Created note"),
                    QStringLiteral("created")));
        }
        if (modifiedTimestamp.valid)
        {
            entries.push_back(
                buildProjectedNoteEntry(
                    note,
                    modifiedTimestamp,
                    QStringLiteral("Modified note"),
                    QStringLiteral("modified")));
        }
    }

    return entries;
}
