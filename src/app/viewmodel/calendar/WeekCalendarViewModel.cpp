#include "WeekCalendarViewModel.hpp"

#include "calendar/CalendarBoardStore.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QVariantMap>

WeekCalendarViewModel::WeekCalendarViewModel(QObject* parent)
    : QObject(parent)
{
    const QLocale locale = QLocale::system();
    const QDate weekStartDate = startOfWeek(QDate::currentDate(), locale);
    m_displayedWeekStartIso = weekStartDate.toString(Qt::ISODate);
    rebuildWeekModel();
}

QString WeekCalendarViewModel::displayedWeekStartIso() const
{
    return m_displayedWeekStartIso;
}

QString WeekCalendarViewModel::weekLabel() const
{
    return m_weekLabel;
}

QStringList WeekCalendarViewModel::weekdayLabels() const
{
    return m_weekdayLabels;
}

QVariantList WeekCalendarViewModel::dayModels() const
{
    return m_dayModels;
}

void WeekCalendarViewModel::setCalendarBoardStore(CalendarBoardStore* calendarBoardStore)
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
        connect(m_calendarBoardStore, &CalendarBoardStore::entriesChanged, this, [this]()
        {
            rebuildWeekModel();
        });
    }

    rebuildWeekModel();
}

void WeekCalendarViewModel::setDisplayedWeekStartIso(const QString& dateIso)
{
    QDate parsedDate;
    if (!parseIsoDate(dateIso, &parsedDate))
    {
        return;
    }

    const QLocale locale = QLocale::system();
    const QString normalizedWeekStartIso = startOfWeek(parsedDate, locale).toString(Qt::ISODate);
    if (m_displayedWeekStartIso == normalizedWeekStartIso)
    {
        return;
    }

    m_displayedWeekStartIso = normalizedWeekStartIso;
    emit displayedWeekStartIsoChanged();
    rebuildWeekModel();
}

void WeekCalendarViewModel::shiftWeek(int deltaWeeks)
{
    if (deltaWeeks == 0)
    {
        return;
    }

    QDate baseWeekStart;
    if (!parseIsoDate(m_displayedWeekStartIso, &baseWeekStart))
    {
        baseWeekStart = startOfWeek(QDate::currentDate(), QLocale::system());
    }

    setDisplayedWeekStartIso(baseWeekStart.addDays(deltaWeeks * 7).toString(Qt::ISODate));
}

void WeekCalendarViewModel::requestWeekView(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.week"),
        QStringLiteral("weekView.request"),
        QStringLiteral("weekStart=%1 reason=%2").arg(m_displayedWeekStartIso, normalizedReason));

    emit weekViewRequested(normalizedReason);
    rebuildWeekModel();
}

bool WeekCalendarViewModel::addEvent(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->addEvent(dateIso, timeText, title, detail);
}

bool WeekCalendarViewModel::addTask(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->addTask(dateIso, timeText, title, detail);
}

QVariantList WeekCalendarViewModel::entriesForDate(const QString& dateIso) const
{
    if (!m_calendarBoardStore)
    {
        return {};
    }
    return m_calendarBoardStore->entriesForDate(dateIso);
}

bool WeekCalendarViewModel::removeEntry(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->removeEntry(entryId);
}

bool WeekCalendarViewModel::setTaskCompleted(const QString& entryId, bool completed)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->setTaskCompleted(entryId, completed);
}

void WeekCalendarViewModel::rebuildWeekModel()
{
    const QLocale locale = QLocale::system();

    QDate weekStartDate;
    if (!parseIsoDate(m_displayedWeekStartIso, &weekStartDate))
    {
        weekStartDate = startOfWeek(QDate::currentDate(), locale);
        m_displayedWeekStartIso = weekStartDate.toString(Qt::ISODate);
        emit displayedWeekStartIsoChanged();
    }
    else
    {
        const QDate normalizedWeekStart = startOfWeek(weekStartDate, locale);
        if (normalizedWeekStart != weekStartDate)
        {
            weekStartDate = normalizedWeekStart;
            m_displayedWeekStartIso = weekStartDate.toString(Qt::ISODate);
            emit displayedWeekStartIsoChanged();
        }
    }

    QStringList nextWeekdayLabels;
    nextWeekdayLabels.reserve(7);
    for (int offset = 0; offset < 7; ++offset)
    {
        const int weekDay = ((static_cast<int>(locale.firstDayOfWeek()) - 1 + offset) % 7) + 1;
        nextWeekdayLabels.push_back(locale.standaloneDayName(weekDay, QLocale::ShortFormat));
    }

    const QDate today = QDate::currentDate();
    QVariantList nextDayModels;
    nextDayModels.reserve(7);
    for (int dayOffset = 0; dayOffset < 7; ++dayOffset)
    {
        const QDate date = weekStartDate.addDays(dayOffset);
        const QString dateIso = date.toString(Qt::ISODate);
        const QVariantList entries =
            m_calendarBoardStore ? m_calendarBoardStore->entriesForDate(dateIso) : QVariantList{};
        const QVariantMap entryCounts =
            m_calendarBoardStore ? m_calendarBoardStore->countsForDate(dateIso) : QVariantMap{};

        QVariantMap dayModel;
        dayModel.insert(QStringLiteral("dateIso"), dateIso);
        dayModel.insert(QStringLiteral("day"), date.day());
        dayModel.insert(QStringLiteral("dayLabel"), locale.toString(date, QStringLiteral("ddd d")));
        dayModel.insert(QStringLiteral("isToday"), date == today);
        dayModel.insert(QStringLiteral("entries"), entries);
        dayModel.insert(
            QStringLiteral("eventCount"),
            entryCounts.value(QStringLiteral("eventCount"), 0).toInt());
        dayModel.insert(
            QStringLiteral("taskCount"),
            entryCounts.value(QStringLiteral("taskCount"), 0).toInt());
        dayModel.insert(
            QStringLiteral("entryCount"),
            entryCounts.value(QStringLiteral("entryCount"), 0).toInt());
        nextDayModels.push_back(dayModel);
    }

    m_weekLabel = formatWeekLabel(weekStartDate, locale);
    m_weekdayLabels = nextWeekdayLabels;
    m_dayModels = nextDayModels;
    emit weekViewChanged();
}

bool WeekCalendarViewModel::parseIsoDate(const QString& dateIso, QDate* outDate)
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

QDate WeekCalendarViewModel::startOfWeek(const QDate& date, const QLocale& locale)
{
    if (!date.isValid())
    {
        return {};
    }

    const int firstDayOfWeek = static_cast<int>(locale.firstDayOfWeek());
    const int dayOfWeek = date.dayOfWeek();
    const int distance = (dayOfWeek - firstDayOfWeek + 7) % 7;
    return date.addDays(-distance);
}

QString WeekCalendarViewModel::formatWeekLabel(const QDate& weekStartDate, const QLocale& locale)
{
    if (!weekStartDate.isValid())
    {
        return QStringLiteral("Week");
    }

    const QDate weekEndDate = weekStartDate.addDays(6);
    return QStringLiteral("%1 - %2")
        .arg(locale.toString(weekStartDate, QLocale::ShortFormat))
        .arg(locale.toString(weekEndDate, QLocale::ShortFormat));
}

