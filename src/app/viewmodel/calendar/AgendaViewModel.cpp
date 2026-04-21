#include "AgendaViewModel.hpp"

#include "calendar/ICalendarBoardStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QVariantMap>
#include <QtGlobal>

AgendaViewModel::AgendaViewModel(QObject* parent)
    : QObject(parent)
      , m_displayedDateIso(QDate::currentDate().toString(Qt::ISODate))
{
    m_locationModel.cityName = QStringLiteral("Seoul");
    m_locationModel.regionName = QStringLiteral("KR");
    m_locationModel.countryCode = QStringLiteral("KR");
    m_locationModel.timeZoneId = QStringLiteral("Asia/Seoul");

    rebuildAgenda();
}

QString AgendaViewModel::displayedDateIso() const
{
    return m_displayedDateIso;
}

QString AgendaViewModel::dateLabel() const
{
    return m_dateLabel;
}

QVariantMap AgendaViewModel::location() const
{
    return toLocationVariant(m_locationModel);
}

QVariantList AgendaViewModel::allDayEvents() const
{
    return m_allDayEvents;
}

QVariantList AgendaViewModel::timedEvents() const
{
    return m_timedEvents;
}

QVariantList AgendaViewModel::agendaItems() const
{
    return m_agendaItems;
}

QVariantMap AgendaViewModel::summary() const
{
    return m_summary;
}

void AgendaViewModel::setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore)
{
    if (m_calendarBoardStore == calendarBoardStore)
    {
        return;
    }

    if (m_calendarBoardStore)
    {
        disconnect(m_calendarBoardStore, nullptr, this, nullptr);
    }

    m_calendarBoardStore = calendarBoardStore;
    if (m_calendarBoardStore)
    {
        connect(m_calendarBoardStore, &ICalendarBoardStore::entriesChanged, this, [this]()
        {
            rebuildAgenda();
        });
    }

    rebuildAgenda();
}

void AgendaViewModel::setDisplayedDateIso(const QString& dateIso)
{
    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        return;
    }

    const QString normalizedDateIso = parsedDate.toString(Qt::ISODate);
    if (m_displayedDateIso == normalizedDateIso)
    {
        return;
    }

    m_displayedDateIso = normalizedDateIso;
    emit displayedDateIsoChanged();
    rebuildAgenda();
}

void AgendaViewModel::shiftDay(int deltaDays)
{
    if (deltaDays == 0)
    {
        return;
    }

    QDate baseDate;
    if (!parseIsoDate(m_displayedDateIso, &baseDate))
    {
        baseDate = QDate::currentDate();
    }

    setDisplayedDateIso(baseDate.addDays(deltaDays).toString(Qt::ISODate));
}

void AgendaViewModel::requestAgendaView(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.agenda"),
        QStringLiteral("agenda.request"),
        QStringLiteral("date=%1 reason=%2").arg(m_displayedDateIso, normalizedReason));

    emit agendaRequested(normalizedReason);
}

bool AgendaViewModel::addEvent(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_displayedDateIso : dateIso.trimmed();
    return m_calendarBoardStore->addEvent(resolvedDateIso, timeText, title, detail);
}

bool AgendaViewModel::addAgendaItem(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_displayedDateIso : dateIso.trimmed();
    return m_calendarBoardStore->addTask(resolvedDateIso, timeText, title, detail);
}

QVariantList AgendaViewModel::entriesForDate(const QString& dateIso) const
{
    if (!m_calendarBoardStore)
    {
        return {};
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_displayedDateIso : dateIso.trimmed();
    return m_calendarBoardStore->entriesForDate(resolvedDateIso);
}

bool AgendaViewModel::removeEntry(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    return m_calendarBoardStore->removeEntry(entryId);
}

bool AgendaViewModel::setAgendaItemCompleted(const QString& entryId, bool completed)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    return m_calendarBoardStore->setTaskCompleted(entryId, completed);
}

bool AgendaViewModel::toggleAgendaItemCompleted(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    const QVariantList entries = m_calendarBoardStore->entriesForDate(m_displayedDateIso);
    for (const QVariant& entryValue : entries)
    {
        const QVariantMap entry = entryValue.toMap();
        if (entry.value(QStringLiteral("type")).toString() != QStringLiteral("task"))
        {
            continue;
        }
        if (entry.value(QStringLiteral("id")).toString() != entryId)
        {
            continue;
        }

        const bool completed = entry.value(QStringLiteral("completed")).toBool();
        return m_calendarBoardStore->setTaskCompleted(entryId, !completed);
    }

    return false;
}

void AgendaViewModel::setLocation(
    const QString& cityName,
    const QString& regionName,
    const QString& countryCode,
    const QString& timeZoneId)
{
    const AgendaLocationModel nextModel{
        cityName.trimmed(),
        regionName.trimmed(),
        countryCode.trimmed().toUpper(),
        timeZoneId.trimmed()};

    if (m_locationModel.cityName == nextModel.cityName
        && m_locationModel.regionName == nextModel.regionName
        && m_locationModel.countryCode == nextModel.countryCode
        && m_locationModel.timeZoneId == nextModel.timeZoneId)
    {
        return;
    }

    m_locationModel = nextModel;
    emit agendaChanged();
}

bool AgendaViewModel::parseIsoDate(const QString& dateIso, QDate* outDate)
{
    if (!outDate)
    {
        return false;
    }

    const QDate parsedDate = QDate::fromString(dateIso.trimmed(), Qt::ISODate);
    if (!parsedDate.isValid())
    {
        return false;
    }

    *outDate = parsedDate;
    return true;
}

QString AgendaViewModel::formatDateLabel(const QDate& date, const QLocale& locale)
{
    QString label = locale.toString(date, QStringLiteral("dddd, MMMM d")).trimmed();
    if (!label.isEmpty())
    {
        return label;
    }

    label = locale.toString(date, QLocale::LongFormat).trimmed();
    if (!label.isEmpty())
    {
        return label;
    }

    return date.toString(Qt::ISODate);
}

bool AgendaViewModel::isAllDayEvent(const QVariantMap& entry)
{
    if (entry.value(QStringLiteral("type")).toString() != QStringLiteral("event"))
    {
        return false;
    }

    if (entry.contains(QStringLiteral("allDay")))
    {
        return entry.value(QStringLiteral("allDay")).toBool();
    }

    const QTime entryTime = QTime::fromString(entry.value(QStringLiteral("time")).toString(), QStringLiteral("HH:mm"));
    return entryTime.isValid() && entryTime.hour() == 0 && entryTime.minute() == 0;
}

QVariantMap AgendaViewModel::toEventModel(const QVariantMap& entry, bool allDay)
{
    QVariantMap model = entry;
    model.insert(QStringLiteral("isAllDay"), allDay);
    model.insert(
        QStringLiteral("timeLabel"),
        allDay ? QStringLiteral("All day") : entry.value(QStringLiteral("time")).toString());
    return model;
}

QVariantMap AgendaViewModel::toAgendaItemModel(const QVariantMap& entry)
{
    QVariantMap model = entry;
    model.insert(
        QStringLiteral("statusText"),
        entry.value(QStringLiteral("completed")).toBool()
            ? QStringLiteral("Done")
            : QStringLiteral("Open"));
    return model;
}

QVariantMap AgendaViewModel::toLocationVariant(const AgendaLocationModel& model)
{
    const QString locationTitle = !model.regionName.isEmpty()
                                      ? QStringLiteral("%1, %2").arg(model.cityName, model.regionName)
                                      : model.cityName;

    return {
        {QStringLiteral("cityName"), model.cityName},
        {QStringLiteral("regionName"), model.regionName},
        {QStringLiteral("countryCode"), model.countryCode},
        {QStringLiteral("timeZoneId"), model.timeZoneId},
        {QStringLiteral("displayName"), locationTitle}
    };
}

void AgendaViewModel::rebuildAgenda()
{
    QDate displayedDate;
    if (!parseIsoDate(m_displayedDateIso, &displayedDate))
    {
        displayedDate = QDate::currentDate();
        m_displayedDateIso = displayedDate.toString(Qt::ISODate);
        emit displayedDateIsoChanged();
    }

    const QLocale locale = QLocale::system();
    const QVariantList entries = m_calendarBoardStore
                                     ? m_calendarBoardStore->entriesForDate(m_displayedDateIso)
                                     : QVariantList{};

    QVariantList nextAllDayEvents;
    QVariantList nextTimedEvents;
    QVariantList nextAgendaItems;
    int completedAgendaItemCount = 0;

    for (const QVariant& entryValue : entries)
    {
        const QVariantMap entry = entryValue.toMap();
        const QString entryType = entry.value(QStringLiteral("type")).toString();

        if (entryType == QStringLiteral("event"))
        {
            const bool allDay = isAllDayEvent(entry);
            if (allDay)
            {
                nextAllDayEvents.push_back(toEventModel(entry, true));
            }
            else
            {
                nextTimedEvents.push_back(toEventModel(entry, false));
            }
            continue;
        }

        if (entryType == QStringLiteral("task"))
        {
            if (entry.value(QStringLiteral("completed")).toBool())
            {
                completedAgendaItemCount += 1;
            }
            nextAgendaItems.push_back(toAgendaItemModel(entry));
        }
    }

    const QString nextDateLabel = formatDateLabel(displayedDate, locale);
    const QVariantMap nextSummary{
        {QStringLiteral("allDayEventCount"), nextAllDayEvents.size()},
        {QStringLiteral("timedEventCount"), nextTimedEvents.size()},
        {QStringLiteral("agendaItemCount"), nextAgendaItems.size()},
        {QStringLiteral("completedAgendaItemCount"), completedAgendaItemCount},
        {QStringLiteral("remainingAgendaItemCount"), qMax(0, nextAgendaItems.size() - completedAgendaItemCount)},
        {QStringLiteral("entryCount"), entries.size()}
    };

    const bool changed = m_dateLabel != nextDateLabel
        || m_allDayEvents != nextAllDayEvents
        || m_timedEvents != nextTimedEvents
        || m_agendaItems != nextAgendaItems
        || m_summary != nextSummary;

    m_dateLabel = nextDateLabel;
    m_allDayEvents = nextAllDayEvents;
    m_timedEvents = nextTimedEvents;
    m_agendaItems = nextAgendaItems;
    m_summary = nextSummary;

    if (changed)
    {
        emit agendaChanged();
    }
}
