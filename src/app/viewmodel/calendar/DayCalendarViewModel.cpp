#include "app/viewmodel/calendar/DayCalendarViewModel.hpp"

#include "app/models/calendar/ICalendarBoardStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QVariantMap>

DayCalendarViewModel::DayCalendarViewModel(QObject* parent)
    : QObject(parent)
      , m_displayedDateIso(QDate::currentDate().toString(Qt::ISODate))
{
    rebuildDayModel();
}

QString DayCalendarViewModel::displayedDateIso() const
{
    return m_displayedDateIso;
}

QString DayCalendarViewModel::dayLabel() const
{
    return m_dayLabel;
}

QVariantList DayCalendarViewModel::dayEntries() const
{
    return m_dayEntries;
}

QVariantList DayCalendarViewModel::timeSlots() const
{
    return m_timeSlots;
}

void DayCalendarViewModel::setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore)
{
    if (calendarBoardStore != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::ViewModel,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("DayCalendarViewModel::setCalendarBoardStore")))
    {
        return;
    }

    if (calendarBoardStore == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DayCalendarViewModel::setCalendarBoardStore")))
    {
        return;
    }

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
            rebuildDayModel();
        });
    }

    rebuildDayModel();
}

void DayCalendarViewModel::setDisplayedDateIso(const QString& dateIso)
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
    rebuildDayModel();
}

void DayCalendarViewModel::shiftDay(int deltaDays)
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

void DayCalendarViewModel::requestDayView(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.day"),
        QStringLiteral("dayView.request"),
        QStringLiteral("date=%1 reason=%2").arg(m_displayedDateIso, normalizedReason));

    emit dayViewRequested(normalizedReason);
}

bool DayCalendarViewModel::addEvent(
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

bool DayCalendarViewModel::addTask(
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

QVariantList DayCalendarViewModel::entriesForDate(const QString& dateIso) const
{
    if (!m_calendarBoardStore)
    {
        return {};
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_displayedDateIso : dateIso.trimmed();
    return m_calendarBoardStore->entriesForDate(resolvedDateIso);
}

bool DayCalendarViewModel::removeEntry(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    return m_calendarBoardStore->removeEntry(entryId);
}

bool DayCalendarViewModel::setTaskCompleted(const QString& entryId, bool completed)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    return m_calendarBoardStore->setTaskCompleted(entryId, completed);
}

void DayCalendarViewModel::rebuildDayModel()
{
    QDate dayDate;
    if (!parseIsoDate(m_displayedDateIso, &dayDate))
    {
        dayDate = QDate::currentDate();
        m_displayedDateIso = dayDate.toString(Qt::ISODate);
        emit displayedDateIsoChanged();
    }

    const QLocale locale = QLocale::system();
    const QVariantList entries = m_calendarBoardStore
                                     ? m_calendarBoardStore->entriesForDate(m_displayedDateIso)
                                     : QVariantList{};

    QVariantList nextTimeSlots;
    nextTimeSlots.reserve(24);
    for (int hour = 0; hour < 24; ++hour)
    {
        QVariantList hourEntries;
        for (const QVariant& entryValue : entries)
        {
            const QVariantMap entry = entryValue.toMap();
            const QTime entryTime = QTime::fromString(entry.value(QStringLiteral("time")).toString(), QStringLiteral("HH:mm"));
            if (entryTime.isValid() && entryTime.hour() == hour)
            {
                hourEntries.push_back(entry);
            }
        }

        QVariantMap hourModel;
        hourModel.insert(QStringLiteral("hour"), hour);
        hourModel.insert(QStringLiteral("timeLabel"), QTime(hour, 0).toString(QStringLiteral("HH:mm")));
        hourModel.insert(QStringLiteral("entries"), hourEntries);
        hourModel.insert(QStringLiteral("entryCount"), hourEntries.size());
        nextTimeSlots.push_back(hourModel);
    }

    const QString nextDayLabel = formatDayLabel(dayDate, locale);
    const bool entriesChanged = (m_dayEntries != entries);

    m_dayLabel = nextDayLabel;
    m_dayEntries = entries;
    m_timeSlots = nextTimeSlots;

    if (entriesChanged)
    {
        emit dayEntriesChanged();
    }
    emit dayViewChanged();
}

bool DayCalendarViewModel::parseIsoDate(const QString& dateIso, QDate* outDate)
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

QString DayCalendarViewModel::formatDayLabel(const QDate& date, const QLocale& locale)
{
    QString label = locale.toString(date, QLocale::LongFormat).trimmed();
    if (!label.isEmpty())
    {
        return label;
    }
    label = locale.toString(date, QStringLiteral("yyyy-MM-dd")).trimmed();
    if (!label.isEmpty())
    {
        return label;
    }
    return date.toString(Qt::ISODate);
}
