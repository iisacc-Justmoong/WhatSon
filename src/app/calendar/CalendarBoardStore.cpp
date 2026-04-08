#include "CalendarBoardStore.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryNotePreviewText.hpp"
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

    QString noteDisplayTitle(const LibraryNoteRecord& note)
    {
        const QString previewHeadline = WhatSon::LibraryPreview::notePrimaryHeadline(note);
        if (!previewHeadline.isEmpty())
        {
            return previewHeadline;
        }

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        return QStringLiteral("Untitled note");
    }

    bool noteTimestampLessThan(const ParsedNoteTimestamp& left, const ParsedNoteTimestamp& right)
    {
        if (left.date != right.date)
        {
            return left.date < right.date;
        }

        const QTime leftTime = left.hasExplicitTime && left.time.isValid()
                                   ? left.time
                                   : QTime(0, 0);
        const QTime rightTime = right.hasExplicitTime && right.time.isValid()
                                    ? right.time
                                    : QTime(0, 0);
        if (leftTime != rightTime)
        {
            return leftTime < rightTime;
        }

        return !left.hasExplicitTime && right.hasExplicitTime;
    }

    CalendarBoardStore::CalendarEntry buildProjectedNoteEntryFromTimestamp(
        const LibraryNoteRecord& note,
        const ParsedNoteTimestamp& timestamp,
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
        entry.title = noteDisplayTitle(note);
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

    bool projectedEntryMapEqual(
        const QHash<QString, CalendarBoardStore::CalendarEntry>& left,
        const QHash<QString, CalendarBoardStore::CalendarEntry>& right)
    {
        if (left.size() != right.size())
        {
            return false;
        }

        for (auto it = left.constBegin(); it != left.constEnd(); ++it)
        {
            const auto rightIt = right.constFind(it.key());
            if (rightIt == right.constEnd() || !calendarEntryEqual(it.value(), rightIt.value()))
            {
                return false;
            }
        }

        return true;
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

    const QString resolvedDateKey = dateKey(parsedDate);
    const auto manualIt = m_entriesByDate.constFind(resolvedDateKey);
    const QVector<CalendarEntry> projectedEntries = projectedEntriesForDateQuery(parsedDate);
    const QVector<CalendarEntry> mergedEntries = mergeIndexedEntries(
        manualIt != m_entriesByDate.constEnd() ? manualIt.value() : QVector<CalendarEntry>{},
        projectedEntries);
    return toVariantList(mergedEntries);
}

QVariantMap CalendarBoardStore::countsForDate(const QString& dateIso) const
{
    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        return toCountsVariant({});
    }

    const QString resolvedDateKey = dateKey(parsedDate);
    EntryCounts mergedCounts;
    const auto manualIt = m_entryCountsByDate.constFind(resolvedDateKey);
    if (manualIt != m_entryCountsByDate.constEnd())
    {
        mergedCounts.eventCount += manualIt.value().eventCount;
        mergedCounts.taskCount += manualIt.value().taskCount;
    }

    const EntryCounts projectedCounts = projectedCountsForDateQuery(parsedDate);
    mergedCounts.eventCount += projectedCounts.eventCount;
    mergedCounts.taskCount += projectedCounts.taskCount;
    return toCountsVariant(mergedCounts);
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

        const CalendarEntry removedEntry = *it;
        m_entries.erase(it);
        removeIndexedEntry(removedEntry, &m_entriesByDate, &m_entryCountsByDate);
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
        updateIndexedEntry(entry, &m_entriesByDate);
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

void CalendarBoardStore::setProjectedNotesProvider(std::function<QVector<LibraryNoteRecord>()> provider)
{
    m_projectedNotesProvider = std::move(provider);
    if (m_projectedEntriesBySourceId.isEmpty())
    {
        m_projectedEntriesInitialized = false;
    }
}

void CalendarBoardStore::reloadProjectedNotesFromSnapshot(const QVector<LibraryNoteRecord>& notes)
{
    replaceProjectedEntries(buildProjectedNoteEntries(notes));
    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kCalendarBoardScope),
        QStringLiteral("reloadProjectedNotes.snapshot"),
        QStringLiteral("path=%1 noteCount=%2 projectedCount=%3")
            .arg(m_projectedNotesHubPath)
            .arg(notes.size())
            .arg(m_projectedEntriesBySourceId.size()));
}

bool CalendarBoardStore::upsertProjectedNote(const LibraryNoteRecord& note)
{
    const QString normalizedSourceId = note.noteId.trimmed();
    if (normalizedSourceId.isEmpty())
    {
        return false;
    }

    CalendarEntry nextEntry;
    const bool hasNextEntry = buildProjectedNoteEntry(note, &nextEntry);
    const auto existingIt = m_projectedEntriesBySourceId.constFind(normalizedSourceId);
    const bool hasExistingEntry = existingIt != m_projectedEntriesBySourceId.constEnd();
    const CalendarEntry existingEntry = hasExistingEntry ? existingIt.value() : CalendarEntry{};

    if (hasExistingEntry && hasNextEntry && calendarEntryEqual(existingEntry, nextEntry))
    {
        m_projectedEntriesInitialized = true;
        return false;
    }

    if (hasExistingEntry)
    {
        removeIndexedEntry(existingEntry, &m_projectedEntriesByDate, &m_projectedEntryCountsByDate);
        m_projectedEntriesBySourceId.remove(normalizedSourceId);
    }

    if (hasNextEntry)
    {
        m_projectedEntriesBySourceId.insert(normalizedSourceId, nextEntry);
        insertIndexedEntry(nextEntry, &m_projectedEntriesByDate, &m_projectedEntryCountsByDate);
    }

    m_projectedEntriesInitialized = true;
    emit entriesChanged();

    if (hasExistingEntry && !hasNextEntry)
    {
        emit entryRemoved(existingEntry.id);
    }
    else if (!hasExistingEntry && hasNextEntry)
    {
        emit entryAdded(
            nextEntry.id,
            entryTypeName(nextEntry.type),
            nextEntry.date.toString(Qt::ISODate),
            nextEntry.time.toString(QStringLiteral("HH:mm")));
    }
    else if (hasNextEntry)
    {
        if (existingEntry.id != nextEntry.id)
        {
            emit entryRemoved(existingEntry.id);
            emit entryAdded(
                nextEntry.id,
                entryTypeName(nextEntry.type),
                nextEntry.date.toString(Qt::ISODate),
                nextEntry.time.toString(QStringLiteral("HH:mm")));
        }
        else
        {
            emit entryUpdated(nextEntry.id);
        }
    }

    return true;
}

bool CalendarBoardStore::removeProjectedNoteBySourceId(const QString& noteId)
{
    const QString normalizedSourceId = noteId.trimmed();
    if (normalizedSourceId.isEmpty())
    {
        return false;
    }

    const auto existingIt = m_projectedEntriesBySourceId.constFind(normalizedSourceId);
    if (existingIt == m_projectedEntriesBySourceId.constEnd())
    {
        return false;
    }

    const CalendarEntry removedEntry = existingIt.value();
    removeIndexedEntry(removedEntry, &m_projectedEntriesByDate, &m_projectedEntryCountsByDate);
    m_projectedEntriesBySourceId.remove(normalizedSourceId);
    m_projectedEntriesInitialized = true;
    emit entriesChanged();
    emit entryRemoved(removedEntry.id);
    return true;
}

void CalendarBoardStore::requestProjectedNotesReload()
{
    if (m_projectedNotesHubPath.isEmpty() && !m_projectedNotesProvider)
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
        if (m_projectedNotesProvider)
        {
            const QVector<LibraryNoteRecord> notes = m_projectedNotesProvider();
            if (!notes.isEmpty())
            {
                replaceProjectedEntries(buildProjectedNoteEntries(notes));
                if (errorMessage != nullptr)
                {
                    errorMessage->clear();
                }
                WhatSon::Debug::traceSelf(
                    this,
                    QString::fromLatin1(kCalendarBoardScope),
                    QStringLiteral("reloadProjectedNotes.success"),
                    QStringLiteral("source=provider path=%1 noteCount=%2 projectedCount=%3")
                        .arg(m_projectedNotesHubPath)
                        .arg(notes.size())
                        .arg(m_projectedEntriesBySourceId.size()));
                return true;
            }
        }
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
            .arg(m_projectedEntriesBySourceId.size()));
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
    insertIndexedEntry(entry, &m_entriesByDate, &m_entryCountsByDate);

    emit entriesChanged();
    emit entryAdded(
        entry.id,
        entryTypeName(entry.type),
        entry.date.toString(Qt::ISODate),
        entry.time.toString(QStringLiteral("HH:mm")));
    return true;
}

void CalendarBoardStore::rebuildProjectedIndexes()
{
    QVector<CalendarEntry> entries;
    entries.reserve(m_projectedEntriesBySourceId.size());
    for (auto it = m_projectedEntriesBySourceId.constBegin(); it != m_projectedEntriesBySourceId.constEnd(); ++it)
    {
        entries.push_back(it.value());
    }
    indexEntries(entries, &m_projectedEntriesByDate, &m_projectedEntryCountsByDate);
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

QString CalendarBoardStore::dateKey(const QDate& date)
{
    return date.isValid() ? date.toString(Qt::ISODate) : QString();
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

QVariantMap CalendarBoardStore::toCountsVariant(const EntryCounts& counts)
{
    return {
        {QStringLiteral("eventCount"), counts.eventCount},
        {QStringLiteral("taskCount"), counts.taskCount},
        {QStringLiteral("entryCount"), counts.eventCount + counts.taskCount}
    };
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

void CalendarBoardStore::indexEntries(
    const QVector<CalendarEntry>& source,
    QHash<QString, QVector<CalendarEntry>>* entriesByDate,
    QHash<QString, EntryCounts>* countsByDate)
{
    if (entriesByDate == nullptr || countsByDate == nullptr)
    {
        return;
    }

    entriesByDate->clear();
    countsByDate->clear();

    for (const CalendarEntry& entry : source)
    {
        insertIndexedEntry(entry, entriesByDate, countsByDate);
    }
}

void CalendarBoardStore::insertIndexedEntry(
    const CalendarEntry& entry,
    QHash<QString, QVector<CalendarEntry>>* entriesByDate,
    QHash<QString, EntryCounts>* countsByDate)
{
    if (entriesByDate == nullptr || countsByDate == nullptr)
    {
        return;
    }

    const QString resolvedDateKey = dateKey(entry.date);
    if (resolvedDateKey.isEmpty())
    {
        return;
    }

    QVector<CalendarEntry>& dayEntries = (*entriesByDate)[resolvedDateKey];
    auto insertIt = std::lower_bound(dayEntries.begin(), dayEntries.end(), entry, calendarEntryLessThan);
    dayEntries.insert(insertIt, entry);

    EntryCounts& counts = (*countsByDate)[resolvedDateKey];
    if (entry.type == EntryType::Event)
    {
        counts.eventCount += 1;
    }
    else
    {
        counts.taskCount += 1;
    }
}

void CalendarBoardStore::removeIndexedEntry(
    const CalendarEntry& entry,
    QHash<QString, QVector<CalendarEntry>>* entriesByDate,
    QHash<QString, EntryCounts>* countsByDate)
{
    if (entriesByDate == nullptr || countsByDate == nullptr)
    {
        return;
    }

    const QString resolvedDateKey = dateKey(entry.date);
    if (resolvedDateKey.isEmpty())
    {
        return;
    }

    auto entriesIt = entriesByDate->find(resolvedDateKey);
    if (entriesIt == entriesByDate->end())
    {
        return;
    }

    QVector<CalendarEntry>& dayEntries = entriesIt.value();
    for (auto it = dayEntries.begin(); it != dayEntries.end(); ++it)
    {
        if (it->id != entry.id)
        {
            continue;
        }

        dayEntries.erase(it);
        break;
    }

    if (dayEntries.isEmpty())
    {
        entriesByDate->erase(entriesIt);
    }

    auto countsIt = countsByDate->find(resolvedDateKey);
    if (countsIt == countsByDate->end())
    {
        return;
    }

    if (entry.type == EntryType::Event)
    {
        countsIt->eventCount = qMax(0, countsIt->eventCount - 1);
    }
    else
    {
        countsIt->taskCount = qMax(0, countsIt->taskCount - 1);
    }

    if (countsIt->eventCount == 0 && countsIt->taskCount == 0)
    {
        countsByDate->erase(countsIt);
    }
}

void CalendarBoardStore::updateIndexedEntry(
    const CalendarEntry& entry,
    QHash<QString, QVector<CalendarEntry>>* entriesByDate)
{
    if (entriesByDate == nullptr)
    {
        return;
    }

    const QString resolvedDateKey = dateKey(entry.date);
    if (resolvedDateKey.isEmpty())
    {
        return;
    }

    auto entriesIt = entriesByDate->find(resolvedDateKey);
    if (entriesIt == entriesByDate->end())
    {
        return;
    }

    QVector<CalendarEntry>& dayEntries = entriesIt.value();
    for (CalendarEntry& existingEntry : dayEntries)
    {
        if (existingEntry.id == entry.id)
        {
            existingEntry = entry;
            return;
        }
    }
}

QVariantList CalendarBoardStore::toVariantList(const QVector<CalendarEntry>& entries)
{
    QVariantList values;
    values.reserve(entries.size());
    for (const CalendarEntry& entry : entries)
    {
        values.push_back(toVariantMap(entry));
    }
    return values;
}

QVector<CalendarBoardStore::CalendarEntry> CalendarBoardStore::mergeIndexedEntries(
    const QVector<CalendarEntry>& manualEntries,
    const QVector<CalendarEntry>& projectedEntries)
{
    QVector<CalendarEntry> mergedEntries;
    mergedEntries.reserve(manualEntries.size() + projectedEntries.size());

    int manualIndex = 0;
    int projectedIndex = 0;
    while (manualIndex < manualEntries.size() && projectedIndex < projectedEntries.size())
    {
        if (calendarEntryLessThan(projectedEntries.at(projectedIndex), manualEntries.at(manualIndex)))
        {
            mergedEntries.push_back(projectedEntries.at(projectedIndex));
            projectedIndex += 1;
            continue;
        }

        mergedEntries.push_back(manualEntries.at(manualIndex));
        manualIndex += 1;
    }

    while (manualIndex < manualEntries.size())
    {
        mergedEntries.push_back(manualEntries.at(manualIndex));
        manualIndex += 1;
    }

    while (projectedIndex < projectedEntries.size())
    {
        mergedEntries.push_back(projectedEntries.at(projectedIndex));
        projectedIndex += 1;
    }

    return mergedEntries;
}

void CalendarBoardStore::cacheProjectedEntriesForQueries(QVector<CalendarEntry> entries)
{
    sortEntries(&entries);

    QHash<QString, CalendarEntry> cachedEntriesBySourceId;
    cachedEntriesBySourceId.reserve(entries.size());
    for (const CalendarEntry& entry : entries)
    {
        const QString sourceId = entry.sourceId.trimmed();
        if (sourceId.isEmpty())
        {
            continue;
        }
        cachedEntriesBySourceId.insert(sourceId, entry);
    }

    m_projectedEntriesBySourceId = std::move(cachedEntriesBySourceId);
    rebuildProjectedIndexes();
    m_projectedEntriesInitialized = true;
}

void CalendarBoardStore::clearProjectedEntries()
{
    if (m_projectedEntriesBySourceId.isEmpty()
        && m_projectedEntriesByDate.isEmpty()
        && m_projectedEntryCountsByDate.isEmpty())
    {
        m_projectedEntriesInitialized = !m_projectedNotesProvider;
        return;
    }

    m_projectedEntriesBySourceId.clear();
    m_projectedEntriesByDate.clear();
    m_projectedEntryCountsByDate.clear();
    m_projectedEntriesInitialized = !m_projectedNotesProvider;
    emit entriesChanged();
}

void CalendarBoardStore::replaceProjectedEntries(QVector<CalendarEntry> entries)
{
    sortEntries(&entries);
    QHash<QString, CalendarEntry> nextEntriesBySourceId;
    nextEntriesBySourceId.reserve(entries.size());
    for (const CalendarEntry& entry : entries)
    {
        const QString sourceId = entry.sourceId.trimmed();
        if (sourceId.isEmpty())
        {
            continue;
        }
        nextEntriesBySourceId.insert(sourceId, entry);
    }

    if (projectedEntryMapEqual(m_projectedEntriesBySourceId, nextEntriesBySourceId))
    {
        m_projectedEntriesInitialized = true;
        return;
    }

    m_projectedEntriesBySourceId = std::move(nextEntriesBySourceId);
    rebuildProjectedIndexes();
    m_projectedEntriesInitialized = true;
    emit entriesChanged();
}

bool CalendarBoardStore::buildProjectedNoteEntry(const LibraryNoteRecord& note, CalendarEntry* outEntry) const
{
    if (outEntry == nullptr)
    {
        return false;
    }

    const QString noteId = note.noteId.trimmed();
    if (noteId.isEmpty())
    {
        return false;
    }

    const ParsedNoteTimestamp createdTimestamp = parseNoteTimestamp(note.createdAt);
    const ParsedNoteTimestamp modifiedTimestamp = parseNoteTimestamp(note.lastModifiedAt);
    if (!createdTimestamp.valid && !modifiedTimestamp.valid)
    {
        return false;
    }

    ParsedNoteTimestamp effectiveTimestamp = createdTimestamp;
    QString effectiveTimestampKey = QStringLiteral("created");
    if (modifiedTimestamp.valid
        && (!createdTimestamp.valid
            || !noteTimestampLessThan(modifiedTimestamp, createdTimestamp)))
    {
        effectiveTimestamp = modifiedTimestamp;
        effectiveTimestampKey = QStringLiteral("modified");
    }

    *outEntry = buildProjectedNoteEntryFromTimestamp(note, effectiveTimestamp, effectiveTimestampKey);
    return true;
}

QVector<CalendarBoardStore::CalendarEntry> CalendarBoardStore::projectedEntriesForDateQuery(const QDate& date) const
{
    const QString resolvedDateKey = dateKey(date);
    if (resolvedDateKey.isEmpty())
    {
        return {};
    }

    const auto projectedIt = m_projectedEntriesByDate.constFind(resolvedDateKey);
    if (projectedIt != m_projectedEntriesByDate.constEnd())
    {
        return projectedIt.value();
    }

    if (m_projectedEntriesInitialized || !m_projectedNotesProvider)
    {
        return {};
    }

    const QVector<CalendarEntry> fallbackEntries = buildProjectedNoteEntries(m_projectedNotesProvider());
    const_cast<CalendarBoardStore*>(this)->cacheProjectedEntriesForQueries(fallbackEntries);
    const auto refreshedIt = m_projectedEntriesByDate.constFind(resolvedDateKey);
    return refreshedIt != m_projectedEntriesByDate.constEnd() ? refreshedIt.value() : QVector<CalendarEntry>{};
}

CalendarBoardStore::EntryCounts CalendarBoardStore::projectedCountsForDateQuery(const QDate& date) const
{
    const QString resolvedDateKey = dateKey(date);
    if (resolvedDateKey.isEmpty())
    {
        return {};
    }

    const auto countsIt = m_projectedEntryCountsByDate.constFind(resolvedDateKey);
    if (countsIt != m_projectedEntryCountsByDate.constEnd())
    {
        return countsIt.value();
    }

    if (m_projectedEntriesInitialized || !m_projectedNotesProvider)
    {
        return {};
    }

    const QVector<CalendarEntry> fallbackEntries = buildProjectedNoteEntries(m_projectedNotesProvider());
    const_cast<CalendarBoardStore*>(this)->cacheProjectedEntriesForQueries(fallbackEntries);
    return m_projectedEntryCountsByDate.value(resolvedDateKey);
}

QVector<CalendarBoardStore::CalendarEntry> CalendarBoardStore::buildProjectedNoteEntries(
    const QVector<LibraryNoteRecord>& notes) const
{
    QVector<CalendarEntry> entries;
    entries.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        CalendarEntry entry;
        if (buildProjectedNoteEntry(note, &entry))
        {
            entries.push_back(entry);
        }
    }

    return entries;
}
