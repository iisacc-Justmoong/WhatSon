#include "app/viewmodel/calendar/YearCalendarViewModel.hpp"

#include "app/models/calendar/ICalendarBoardStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

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

YearCalendarViewModel::YearCalendarViewModel(QObject* parent)
    : QObject(parent)
      , m_displayedYear(QDate::currentDate().year())
      , m_calendarSystem(Gregorian)
{
    rebuildYearModel();
}

int YearCalendarViewModel::displayedYear() const noexcept
{
    return m_displayedYear;
}

YearCalendarViewModel::CalendarSystem YearCalendarViewModel::calendarSystem() const noexcept
{
    return m_calendarSystem;
}

QString YearCalendarViewModel::calendarSystemName() const
{
    return calendarSystemLabel(m_calendarSystem);
}

QStringList YearCalendarViewModel::weekdayLabels() const
{
    return m_weekdayLabels;
}

QVariantList YearCalendarViewModel::monthModels() const
{
    return m_monthModels;
}

QVariantList YearCalendarViewModel::calendarSystemOptions() const
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

void YearCalendarViewModel::setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore)
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
            rebuildYearModel();
        });
    }

    rebuildYearModel();
}

void YearCalendarViewModel::setDisplayedYear(int year)
{
    const int boundedYear = qBound(kMinimumSupportedYear, year, kMaximumSupportedYear);
    if (m_displayedYear == boundedYear)
    {
        return;
    }

    m_displayedYear = boundedYear;
    emit displayedYearChanged();
    rebuildYearModel();
}

void YearCalendarViewModel::setCalendarSystemByEnum(CalendarSystem system)
{
    if (m_calendarSystem == system)
    {
        return;
    }

    m_calendarSystem = system;
    emit calendarSystemChanged();
    rebuildYearModel();
}

void YearCalendarViewModel::setCalendarSystemByValue(int value)
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
        QStringLiteral("calendar.year"),
        QStringLiteral("setCalendarSystemByValue.rejected"),
        QStringLiteral("value=%1 reason=unsupported enum value").arg(value));
}

void YearCalendarViewModel::shiftYear(int delta)
{
    if (delta == 0)
    {
        return;
    }

    setDisplayedYear(m_displayedYear + delta);
}

void YearCalendarViewModel::focusToday()
{
    const QDate today = QDate::currentDate();
    if (!today.isValid())
    {
        return;
    }

    const QCalendar calendar = resolveCalendarSystem();
    const QCalendar::YearMonthDay parts = calendar.partsFromDate(today);
    if (parts.isValid())
    {
        setDisplayedYear(parts.year);
    }
    else
    {
        setDisplayedYear(today.year());
    }
}

void YearCalendarViewModel::requestYearView(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();

    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("calendar.year"),
        QStringLiteral("yearView.request"),
        QStringLiteral("year=%1 system=%2 reason=%3")
        .arg(m_displayedYear)
        .arg(calendarSystemName(), normalizedReason));

    emit yearViewRequested(normalizedReason);
}

bool YearCalendarViewModel::addEvent(
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

bool YearCalendarViewModel::addTask(
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

QVariantList YearCalendarViewModel::entriesForDate(const QString& dateIso) const
{
    if (!m_calendarBoardStore)
    {
        return {};
    }
    return m_calendarBoardStore->entriesForDate(dateIso);
}

bool YearCalendarViewModel::removeEntry(const QString& entryId)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->removeEntry(entryId);
}

bool YearCalendarViewModel::setTaskCompleted(const QString& entryId, bool completed)
{
    if (!m_calendarBoardStore)
    {
        return false;
    }
    return m_calendarBoardStore->setTaskCompleted(entryId, completed);
}

void YearCalendarViewModel::rebuildYearModel()
{
    const QCalendar calendar = resolveCalendarSystem();
    const QLocale locale = QLocale::system();
    const int firstWeekDay = static_cast<int>(locale.firstDayOfWeek());
    const QDate today = QDate::currentDate();

    QStringList nextWeekdayLabels;
    nextWeekdayLabels.reserve(7);
    for (int offset = 0; offset < 7; ++offset)
    {
        const int weekDay = ((firstWeekDay - 1 + offset) % 7) + 1;
        nextWeekdayLabels.push_back(
            calendar.standaloneWeekDayName(locale, weekDay, QLocale::ShortFormat));
    }

    QVariantList nextMonthModels;

    const int monthCount = qMax(1, calendar.monthsInYear(m_displayedYear));
    for (int month = 1; month <= monthCount; ++month)
    {
        const QDate firstDate = calendar.dateFromParts(m_displayedYear, month, 1);
        if (!firstDate.isValid())
        {
            continue;
        }

        const int daysInMonth = calendar.daysInMonth(month, m_displayedYear);
        if (daysInMonth <= 0)
        {
            continue;
        }

        int previousYear = m_displayedYear;
        int previousMonth = month - 1;
        if (previousMonth < 1)
        {
            previousYear -= 1;
            previousMonth = qMax(1, calendar.monthsInYear(previousYear));
        }

        int nextYear = m_displayedYear;
        int nextMonth = month + 1;
        if (nextMonth > monthCount)
        {
            nextYear += 1;
            nextMonth = 1;
        }

        const int previousMonthDays = qMax(1, calendar.daysInMonth(previousMonth, previousYear));
        const int firstMonthColumn = ((calendar.dayOfWeek(firstDate) - firstWeekDay + 7) % 7);

        QVariantList dayCells;
        dayCells.reserve(kMonthGridCellCount);
        for (int cell = 0; cell < kMonthGridCellCount; ++cell)
        {
            const int dayOffset = cell - firstMonthColumn;

            int targetYear = m_displayedYear;
            int targetMonth = month;
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
            QVariantMap cellModel;
            cellModel.insert(QStringLiteral("day"), targetDay);
            cellModel.insert(QStringLiteral("month"), targetMonth);
            cellModel.insert(QStringLiteral("year"), targetYear);
            cellModel.insert(QStringLiteral("dateIso"), dateIso);
            cellModel.insert(QStringLiteral("inCurrentMonth"), inCurrentMonth);
            cellModel.insert(QStringLiteral("isToday"), cellDate.isValid() && cellDate == today);
            cellModel.insert(
                QStringLiteral("eventCount"),
                entryCounts.value(QStringLiteral("eventCount"), 0).toInt());
            cellModel.insert(
                QStringLiteral("taskCount"),
                entryCounts.value(QStringLiteral("taskCount"), 0).toInt());
            cellModel.insert(
                QStringLiteral("entryCount"),
                entryCounts.value(QStringLiteral("entryCount"), 0).toInt());
            dayCells.push_back(cellModel);
        }

        QString monthLabel = calendar.standaloneMonthName(
            locale,
            month,
            m_displayedYear,
            QLocale::LongFormat);
        if (monthLabel.trimmed().isEmpty())
        {
            monthLabel = QStringLiteral("Month %1").arg(month);
        }

        QVariantMap monthModel;
        monthModel.insert(QStringLiteral("month"), month);
        monthModel.insert(QStringLiteral("monthLabel"), monthLabel);
        monthModel.insert(QStringLiteral("days"), dayCells);
        nextMonthModels.push_back(monthModel);
    }

    m_weekdayLabels = nextWeekdayLabels;
    m_monthModels = nextMonthModels;
    emit yearViewChanged();
}

QCalendar YearCalendarViewModel::resolveCalendarSystem() const
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

QString YearCalendarViewModel::calendarSystemLabel(CalendarSystem system)
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
