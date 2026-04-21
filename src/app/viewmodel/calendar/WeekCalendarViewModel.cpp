#include "WeekCalendarViewModel.hpp"

#include "calendar/ICalendarBoardStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QDate>
#include <QLocale>
#include <QTime>
#include <QVariantMap>

#include <algorithm>
#include <utility>

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

QVariantList WeekCalendarViewModel::timelineDayModels() const
{
    return m_timelineDayModels;
}

void WeekCalendarViewModel::setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore)
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
            rebuildWeekModel();
            refreshTimelineDayModels();
        });
    }

    rebuildWeekModel();
    refreshTimelineDayModels();
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
}

void WeekCalendarViewModel::initializeTimelineWindow(const QString& anchorDateIso, int radius)
{
    QDate anchorDate;
    if (!parseIsoDate(anchorDateIso, &anchorDate))
    {
        return;
    }

    const int boundedRadius = std::max(0, radius);
    const QLocale locale = QLocale::system();
    QVariantList nextTimelineDayModels;
    nextTimelineDayModels.reserve((boundedRadius * 2) + 1);
    for (int offset = -boundedRadius; offset <= boundedRadius; ++offset)
    {
        nextTimelineDayModels.push_back(buildTimelineDayModel(anchorDate.addDays(offset), locale));
    }

    m_timelineDayModels = std::move(nextTimelineDayModels);
    emit timelineDayModelsChanged();
}

int WeekCalendarViewModel::prependTimelineDates(int count)
{
    if (count <= 0 || m_timelineDayModels.isEmpty())
    {
        return 0;
    }

    QDate firstDate;
    if (!parseIsoDate(
            m_timelineDayModels.constFirst().toMap().value(QStringLiteral("dateIso")).toString(),
            &firstDate))
    {
        return 0;
    }

    const QLocale locale = QLocale::system();
    QVariantList prependedDayModels;
    prependedDayModels.reserve(count);
    for (int offset = count; offset >= 1; --offset)
    {
        prependedDayModels.push_back(buildTimelineDayModel(firstDate.addDays(-offset), locale));
    }

    QVariantList nextTimelineDayModels;
    nextTimelineDayModels.reserve(prependedDayModels.size() + m_timelineDayModels.size());
    for (const QVariant& dayModel : prependedDayModels)
    {
        nextTimelineDayModels.push_back(dayModel);
    }
    for (const QVariant& dayModel : m_timelineDayModels)
    {
        nextTimelineDayModels.push_back(dayModel);
    }

    m_timelineDayModels = std::move(nextTimelineDayModels);
    emit timelineDayModelsChanged();
    return prependedDayModels.size();
}

int WeekCalendarViewModel::appendTimelineDates(int count)
{
    if (count <= 0 || m_timelineDayModels.isEmpty())
    {
        return 0;
    }

    QDate lastDate;
    if (!parseIsoDate(
            m_timelineDayModels.constLast().toMap().value(QStringLiteral("dateIso")).toString(),
            &lastDate))
    {
        return 0;
    }

    const QLocale locale = QLocale::system();
    for (int offset = 1; offset <= count; ++offset)
    {
        m_timelineDayModels.push_back(buildTimelineDayModel(lastDate.addDays(offset), locale));
    }

    emit timelineDayModelsChanged();
    return count;
}

QVariantMap WeekCalendarViewModel::trimTimelineWindow(int leadingIndex, int maxWindowSize, int chunkSize)
{
    QVariantMap result{
        {QStringLiteral("removedHead"), 0},
        {QStringLiteral("removedTail"), 0}
    };

    const int maxWindowCount = std::max(0, maxWindowSize);
    if (maxWindowCount == 0 || chunkSize <= 0)
    {
        return result;
    }

    const int initialTimelineCount = static_cast<int>(m_timelineDayModels.size());
    if (initialTimelineCount <= maxWindowCount)
    {
        return result;
    }

    int mutableLeadingIndex = std::max(0, leadingIndex);
    while (true)
    {
        const int timelineCount = static_cast<int>(m_timelineDayModels.size());
        if (timelineCount <= maxWindowCount)
        {
            break;
        }

        const bool removeHead = mutableLeadingIndex > (timelineCount / 2);
        const int overflowCount = timelineCount - maxWindowCount;
        const int removeCount = std::min(chunkSize, overflowCount);
        if (removeCount <= 0)
        {
            break;
        }

        if (removeHead)
        {
            m_timelineDayModels.erase(m_timelineDayModels.begin(), m_timelineDayModels.begin() + removeCount);
            result.insert(
                QStringLiteral("removedHead"),
                result.value(QStringLiteral("removedHead")).toInt() + removeCount);
            mutableLeadingIndex = std::max(0, mutableLeadingIndex - removeCount);
        }
        else
        {
            m_timelineDayModels.erase(m_timelineDayModels.end() - removeCount, m_timelineDayModels.end());
            result.insert(
                QStringLiteral("removedTail"),
                result.value(QStringLiteral("removedTail")).toInt() + removeCount);
        }
    }

    if (result.value(QStringLiteral("removedHead")).toInt() > 0
        || result.value(QStringLiteral("removedTail")).toInt() > 0)
    {
        emit timelineDayModelsChanged();
    }

    return result;
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

QVariantMap WeekCalendarViewModel::buildTimelineDayModel(const QDate& date, const QLocale& locale) const
{
    if (!date.isValid())
    {
        return {};
    }

    const QString dateIso = date.toString(Qt::ISODate);
    const QVariantList entries =
        m_calendarBoardStore ? m_calendarBoardStore->entriesForDate(dateIso) : QVariantList{};
    const QVariantMap entryCounts =
        m_calendarBoardStore ? m_calendarBoardStore->countsForDate(dateIso) : QVariantMap{};
    const QDate today = QDate::currentDate();
    const QDate currentWeekStart = startOfWeek(today, locale);

    QVariantMap dayModel;
    dayModel.insert(QStringLiteral("dateIso"), dateIso);
    dayModel.insert(QStringLiteral("day"), date.day());
    dayModel.insert(QStringLiteral("dayLabel"), locale.toString(date, QStringLiteral("ddd d")));
    dayModel.insert(QStringLiteral("weekdayLabel"), locale.toString(date, QStringLiteral("ddd")));
    dayModel.insert(QStringLiteral("dateLabel"), locale.toString(date, QStringLiteral("M/d")));
    dayModel.insert(QStringLiteral("isToday"), date == today);
    dayModel.insert(QStringLiteral("isInCurrentWeek"), startOfWeek(date, locale) == currentWeekStart);
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
    return dayModel;
}

void WeekCalendarViewModel::refreshTimelineDayModels()
{
    if (m_timelineDayModels.isEmpty())
    {
        return;
    }

    const QLocale locale = QLocale::system();
    QVariantList nextTimelineDayModels;
    nextTimelineDayModels.reserve(m_timelineDayModels.size());
    for (const QVariant& dayModelValue : m_timelineDayModels)
    {
        QDate date;
        if (!parseIsoDate(dayModelValue.toMap().value(QStringLiteral("dateIso")).toString(), &date))
        {
            continue;
        }
        nextTimelineDayModels.push_back(buildTimelineDayModel(date, locale));
    }

    m_timelineDayModels = std::move(nextTimelineDayModels);
    emit timelineDayModelsChanged();
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

    QVariantList nextDayModels;
    nextDayModels.reserve(7);
    for (int dayOffset = 0; dayOffset < 7; ++dayOffset)
    {
        const QDate date = weekStartDate.addDays(dayOffset);
        nextDayModels.push_back(buildTimelineDayModel(date, locale));
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
