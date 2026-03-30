#include "MonthCalendarViewModel.hpp"

#include "calendar/CalendarBoardStore.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QCalendar>
#include <QDate>
#include <QLocale>
#include <QVariantMap>

namespace
{
    constexpr int kMinimumSupportedYear = 1;
    constexpr int kMaximumSupportedYear = 9999;
    constexpr int kMonthGridCellCount = 42;
}

MonthCalendarViewModel::MonthCalendarViewModel(QObject* parent)
    : QObject(parent)
      , m_displayedYear(QDate::currentDate().year())
      , m_displayedMonth(QDate::currentDate().month())
      , m_calendarSystem(Gregorian)
      , m_selectedDateIso(QDate::currentDate().toString(Qt::ISODate))
{
    rebuildMonthModel();
    refreshSelectedDateEntries();
}

int MonthCalendarViewModel::displayedYear() const noexcept
{
    return m_displayedYear;
}

int MonthCalendarViewModel::displayedMonth() const noexcept
{
    return m_displayedMonth;
}

MonthCalendarViewModel::CalendarSystem MonthCalendarViewModel::calendarSystem() const noexcept
{
    return m_calendarSystem;
}

QString MonthCalendarViewModel::calendarSystemName() const
{
    return calendarSystemLabel(m_calendarSystem);
}

QString MonthCalendarViewModel::monthLabel() const
{
    return m_monthLabel;
}

QStringList MonthCalendarViewModel::weekdayLabels() const
{
    return m_weekdayLabels;
}

QVariantList MonthCalendarViewModel::dayModels() const
{
    return m_dayModels;
}

QString MonthCalendarViewModel::selectedDateIso() const
{
    return m_selectedDateIso;
}

QVariantList MonthCalendarViewModel::selectedDateEntries() const
{
    return m_selectedDateEntries;
}

QVariantList MonthCalendarViewModel::calendarSystemOptions() const
{
    return {
        QVariantMap{
            {QStringLiteral("value"), static_cast<int>(Gregorian)},
            {QStringLiteral("label"), calendarSystemLabel(Gregorian)}
        },
        QVariantMap{
            {QStringLiteral("value"), static_cast<int>(Julian)},
            {QStringLiteral("label"), calendarSystemLabel(Julian)}
        },
        QVariantMap{
            {QStringLiteral("value"), static_cast<int>(IslamicCivil)},
            {QStringLiteral("label"), calendarSystemLabel(IslamicCivil)}
        },
        QVariantMap{
            {QStringLiteral("value"), static_cast<int>(Custom)},
            {QStringLiteral("label"), calendarSystemLabel(Custom)}
        }
    };
}

void MonthCalendarViewModel::setCalendarBoardStore(CalendarBoardStore* calendarBoardStore)
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
            rebuildMonthModel();
            refreshSelectedDateEntries();
        });
    }

    rebuildMonthModel();
    refreshSelectedDateEntries();
}

void MonthCalendarViewModel::setDisplayedYear(int year)
{
    const int boundedYear = qBound(kMinimumSupportedYear, year, kMaximumSupportedYear);
    if (m_displayedYear == boundedYear)
    {
        return;
    }

    m_displayedYear = boundedYear;
    emit displayedYearChanged();
    rebuildMonthModel();
}

void MonthCalendarViewModel::setDisplayedMonth(int month)
{
    const QCalendar calendar = resolveCalendarSystem();
    const int monthCount = qMax(1, calendar.monthsInYear(m_displayedYear));
    const int boundedMonth = qBound(1, month, monthCount);
    if (m_displayedMonth == boundedMonth)
    {
        return;
    }

    m_displayedMonth = boundedMonth;
    emit displayedMonthChanged();
    rebuildMonthModel();
}

void MonthCalendarViewModel::setCalendarSystemByEnum(CalendarSystem system)
{
    if (m_calendarSystem == system)
    {
        return;
    }

    m_calendarSystem = system;
    emit calendarSystemChanged();
    rebuildMonthModel();
}

void MonthCalendarViewModel::setSelectedDateIso(const QString& dateIso)
{
    const QDate parsedDate = QDate::fromString(dateIso.trimmed(), Qt::ISODate);
    if (!parsedDate.isValid())
    {
        return;
    }

    const QString normalizedDateIso = parsedDate.toString(Qt::ISODate);
    if (m_selectedDateIso == normalizedDateIso)
    {
        return;
    }

    m_selectedDateIso = normalizedDateIso;
    emit selectedDateIsoChanged();
    refreshSelectedDateEntries();
}

void MonthCalendarViewModel::setCalendarSystemByValue(int value)
{
    switch (value)
    {
    case static_cast<int>(Gregorian):
        setCalendarSystemByEnum(Gregorian);
        return;
    case static_cast<int>(Julian):
        setCalendarSystemByEnum(Julian);
        return;
    case static_cast<int>(IslamicCivil):
        setCalendarSystemByEnum(IslamicCivil);
        return;
    case static_cast<int>(Custom):
        setCalendarSystemByEnum(Custom);
        return;
    default:
        break;
    }

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.month"),
        QStringLiteral("setCalendarSystemByValue.rejected"),
        QStringLiteral("value=%1 reason=unsupported enum value").arg(value));
}

void MonthCalendarViewModel::shiftMonth(int delta)
{
    if (delta == 0)
    {
        return;
    }

    int nextYear = m_displayedYear;
    int nextMonth = m_displayedMonth + delta;
    const QCalendar calendar = resolveCalendarSystem();

    while (nextMonth < 1)
    {
        if (nextYear <= kMinimumSupportedYear)
        {
            nextYear = kMinimumSupportedYear;
            nextMonth = 1;
            break;
        }
        nextYear -= 1;
        nextMonth += qMax(1, calendar.monthsInYear(nextYear));
    }

    while (nextYear < kMaximumSupportedYear)
    {
        const int monthCount = qMax(1, calendar.monthsInYear(nextYear));
        if (nextMonth <= monthCount)
        {
            break;
        }

        nextMonth -= monthCount;
        nextYear += 1;
    }

    nextYear = qBound(kMinimumSupportedYear, nextYear, kMaximumSupportedYear);
    const int monthCount = qMax(1, calendar.monthsInYear(nextYear));
    nextMonth = qBound(1, nextMonth, monthCount);

    bool changed = false;
    if (m_displayedYear != nextYear)
    {
        m_displayedYear = nextYear;
        emit displayedYearChanged();
        changed = true;
    }
    if (m_displayedMonth != nextMonth)
    {
        m_displayedMonth = nextMonth;
        emit displayedMonthChanged();
        changed = true;
    }
    if (!changed)
    {
        return;
    }

    rebuildMonthModel();
}

void MonthCalendarViewModel::requestMonthView(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.month"),
        QStringLiteral("monthView.request"),
        QStringLiteral("year=%1 month=%2 system=%3 reason=%4")
        .arg(m_displayedYear)
        .arg(m_displayedMonth)
        .arg(calendarSystemName(), normalizedReason));

    emit monthViewRequested(normalizedReason);
    rebuildMonthModel();
}

bool MonthCalendarViewModel::addEvent(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_selectedDateIso : dateIso;
    return m_calendarBoardStore->addEvent(resolvedDateIso, timeText, title, detail);
}

bool MonthCalendarViewModel::addTask(
    const QString& dateIso,
    const QString& timeText,
    const QString& title,
    const QString& detail)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_selectedDateIso : dateIso;
    return m_calendarBoardStore->addTask(resolvedDateIso, timeText, title, detail);
}

QVariantList MonthCalendarViewModel::entriesForDate(const QString& dateIso) const
{
    if (!m_calendarBoardStore)
    {
        return {};
    }

    const QString resolvedDateIso = dateIso.trimmed().isEmpty() ? m_selectedDateIso : dateIso.trimmed();
    return m_calendarBoardStore->entriesForDate(resolvedDateIso);
}

bool MonthCalendarViewModel::removeEntry(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->removeEntry(entryId);
}

bool MonthCalendarViewModel::setTaskCompleted(const QString& entryId, bool completed)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->setTaskCompleted(entryId, completed);
}

void MonthCalendarViewModel::rebuildMonthModel()
{
    const QCalendar calendar = resolveCalendarSystem();
    const QLocale locale = QLocale::system();
    const int firstWeekDay = static_cast<int>(locale.firstDayOfWeek());
    const QDate today = QDate::currentDate();

    const int monthCount = qMax(1, calendar.monthsInYear(m_displayedYear));
    const int boundedMonth = qBound(1, m_displayedMonth, monthCount);
    if (boundedMonth != m_displayedMonth)
    {
        m_displayedMonth = boundedMonth;
        emit displayedMonthChanged();
    }

    const QDate firstDate = calendar.dateFromParts(m_displayedYear, m_displayedMonth, 1);
    if (!firstDate.isValid())
    {
        m_weekdayLabels.clear();
        m_dayModels.clear();
        m_monthLabel = QStringLiteral("Month");
        emit monthViewChanged();
        refreshSelectedDateEntries();
        return;
    }

    QStringList nextWeekdayLabels;
    nextWeekdayLabels.reserve(7);
    for (int offset = 0; offset < 7; ++offset)
    {
        const int weekDay = ((firstWeekDay - 1 + offset) % 7) + 1;
        nextWeekdayLabels.push_back(
            calendar.standaloneWeekDayName(locale, weekDay, QLocale::ShortFormat));
    }

    const int daysInMonth = qMax(1, calendar.daysInMonth(m_displayedMonth, m_displayedYear));

    int previousYear = m_displayedYear;
    int previousMonth = m_displayedMonth - 1;
    if (previousMonth < 1)
    {
        previousYear -= 1;
        previousMonth = qMax(1, calendar.monthsInYear(previousYear));
    }

    int nextYear = m_displayedYear;
    int nextMonth = m_displayedMonth + 1;
    if (nextMonth > monthCount)
    {
        nextYear += 1;
        nextMonth = 1;
    }

    const int previousMonthDays = qMax(1, calendar.daysInMonth(previousMonth, previousYear));
    const int firstMonthColumn = ((calendar.dayOfWeek(firstDate) - firstWeekDay + 7) % 7);

    QVariantList nextDayModels;
    nextDayModels.reserve(kMonthGridCellCount);
    for (int cell = 0; cell < kMonthGridCellCount; ++cell)
    {
        const int dayOffset = cell - firstMonthColumn;

        int targetYear = m_displayedYear;
        int targetMonth = m_displayedMonth;
        int targetDay = dayOffset + 1;
        bool inCurrentMonth = true;

        if (dayOffset < 0)
        {
            targetYear = previousYear;
            targetMonth = previousMonth;
            targetDay = previousMonthDays + dayOffset + 1;
            inCurrentMonth = false;
        }
        else if (dayOffset >= daysInMonth)
        {
            targetYear = nextYear;
            targetMonth = nextMonth;
            targetDay = dayOffset - daysInMonth + 1;
            inCurrentMonth = false;
        }

        const QDate cellDate = calendar.dateFromParts(targetYear, targetMonth, targetDay);
        const QString dateIso = cellDate.isValid() ? cellDate.toString(Qt::ISODate) : QString();
        const QVariantMap entryCounts = m_calendarBoardStore
                                            ? m_calendarBoardStore->countsForDate(dateIso)
                                            : QVariantMap{};

        QVariantMap dayModel;
        dayModel.insert(QStringLiteral("day"), targetDay);
        dayModel.insert(QStringLiteral("month"), targetMonth);
        dayModel.insert(QStringLiteral("year"), targetYear);
        dayModel.insert(QStringLiteral("dateIso"), dateIso);
        dayModel.insert(QStringLiteral("inCurrentMonth"), inCurrentMonth);
        dayModel.insert(QStringLiteral("isToday"), cellDate.isValid() && cellDate == today);
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

    QString nextMonthLabel = calendar.standaloneMonthName(
        locale,
        m_displayedMonth,
        m_displayedYear,
        QLocale::LongFormat);
    if (nextMonthLabel.trimmed().isEmpty())
    {
        nextMonthLabel = QStringLiteral("Month %1").arg(m_displayedMonth);
    }

    m_weekdayLabels = nextWeekdayLabels;
    m_dayModels = nextDayModels;
    m_monthLabel = nextMonthLabel;
    emit monthViewChanged();
    refreshSelectedDateEntries();
}

void MonthCalendarViewModel::refreshSelectedDateEntries()
{
    const QVariantList nextEntries =
        m_calendarBoardStore && !m_selectedDateIso.trimmed().isEmpty()
            ? m_calendarBoardStore->entriesForDate(m_selectedDateIso)
            : QVariantList{};
    if (m_selectedDateEntries == nextEntries)
    {
        return;
    }

    m_selectedDateEntries = nextEntries;
    emit selectedDateEntriesChanged();
}

QCalendar MonthCalendarViewModel::resolveCalendarSystem() const
{
    switch (m_calendarSystem)
    {
    case Gregorian:
        return QCalendar(QCalendar::System::Gregorian);
    case Julian:
        {
            const QCalendar calendar(QStringLiteral("julian"));
            if (calendar.isValid())
            {
                return calendar;
            }
            break;
        }
    case IslamicCivil:
        {
            const QCalendar calendar(QStringLiteral("islamic-civil"));
            if (calendar.isValid())
            {
                return calendar;
            }
            break;
        }
    case Custom:
        {
            const QCalendar calendar(QStringLiteral("custom"));
            if (calendar.isValid())
            {
                return calendar;
            }
            break;
        }
    }

    return QCalendar(QCalendar::System::Gregorian);
}

QString MonthCalendarViewModel::calendarSystemLabel(CalendarSystem system)
{
    switch (system)
    {
    case Gregorian:
        return QStringLiteral("Gregorian");
    case Julian:
        return QStringLiteral("Julian");
    case IslamicCivil:
        return QStringLiteral("Islamic Civil");
    case Custom:
        return QStringLiteral("Custom");
    }

    return QStringLiteral("Gregorian");
}
