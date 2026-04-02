#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class ICalendarBoardStore;
class QCalendar;

class MonthCalendarViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int displayedYear READ displayedYear WRITE setDisplayedYear NOTIFY displayedYearChanged)
    Q_PROPERTY(int displayedMonth READ displayedMonth WRITE setDisplayedMonth NOTIFY displayedMonthChanged)
    Q_PROPERTY(
        CalendarSystem calendarSystem READ calendarSystem WRITE setCalendarSystemByEnum NOTIFY calendarSystemChanged)
    Q_PROPERTY(QString calendarSystemName READ calendarSystemName NOTIFY calendarSystemChanged)
    Q_PROPERTY(QString monthLabel READ monthLabel NOTIFY monthViewChanged)
    Q_PROPERTY(QStringList weekdayLabels READ weekdayLabels NOTIFY monthViewChanged)
    Q_PROPERTY(QVariantList dayModels READ dayModels NOTIFY monthViewChanged)
    Q_PROPERTY(QString selectedDateIso READ selectedDateIso WRITE setSelectedDateIso NOTIFY selectedDateIsoChanged)
    Q_PROPERTY(QVariantList selectedDateEntries READ selectedDateEntries NOTIFY selectedDateEntriesChanged)
    Q_PROPERTY(QVariantList calendarSystemOptions READ calendarSystemOptions CONSTANT)

public:
    enum CalendarSystem
    {
        Gregorian = 0,
        Julian = 1,
        IslamicCivil = 2,
        Custom = 3
    };

    Q_ENUM(CalendarSystem)

    explicit MonthCalendarViewModel(QObject* parent = nullptr);

    int displayedYear() const noexcept;
    int displayedMonth() const noexcept;
    CalendarSystem calendarSystem() const noexcept;
    QString calendarSystemName() const;
    QString monthLabel() const;
    QStringList weekdayLabels() const;
    QVariantList dayModels() const;
    QString selectedDateIso() const;
    QVariantList selectedDateEntries() const;
    QVariantList calendarSystemOptions() const;
    void setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore);

public slots:
    void setDisplayedYear(int year);
    void setDisplayedMonth(int month);
    void setCalendarSystemByEnum(CalendarSystem system);
    void setSelectedDateIso(const QString& dateIso);

    Q_INVOKABLE void setCalendarSystemByValue(int value);
    Q_INVOKABLE void shiftMonth(int delta);
    Q_INVOKABLE void focusToday();
    Q_INVOKABLE void requestMonthView(const QString& reason = QString());
    Q_INVOKABLE bool addEvent(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString());
    Q_INVOKABLE bool addTask(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString());
    Q_INVOKABLE QVariantList entriesForDate(const QString& dateIso) const;
    Q_INVOKABLE bool removeEntry(const QString& entryId);
    Q_INVOKABLE bool setTaskCompleted(const QString& entryId, bool completed);

signals:
    void displayedYearChanged();
    void displayedMonthChanged();
    void calendarSystemChanged();
    void monthViewChanged();
    void monthViewRequested(QString reason);
    void selectedDateIsoChanged();
    void selectedDateEntriesChanged();

private:
    void rebuildMonthModel();
    void refreshSelectedDateEntries();
    QCalendar resolveCalendarSystem() const;
    static QString calendarSystemLabel(CalendarSystem system);

    int m_displayedYear = 1970;
    int m_displayedMonth = 1;
    CalendarSystem m_calendarSystem = Gregorian;
    QString m_monthLabel;
    QStringList m_weekdayLabels;
    QVariantList m_dayModels;
    ICalendarBoardStore* m_calendarBoardStore = nullptr;
    QString m_selectedDateIso;
    QVariantList m_selectedDateEntries;
};
