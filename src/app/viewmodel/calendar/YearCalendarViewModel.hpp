#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class ICalendarBoardStore;
class QCalendar;

class YearCalendarViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int displayedYear READ displayedYear WRITE setDisplayedYear NOTIFY displayedYearChanged)
    Q_PROPERTY(
        CalendarSystem calendarSystem READ calendarSystem WRITE setCalendarSystemByEnum NOTIFY calendarSystemChanged)
    Q_PROPERTY(QString calendarSystemName READ calendarSystemName NOTIFY calendarSystemChanged)
    Q_PROPERTY(QStringList weekdayLabels READ weekdayLabels NOTIFY yearViewChanged)
    Q_PROPERTY(QVariantList monthModels READ monthModels NOTIFY yearViewChanged)
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

    explicit YearCalendarViewModel(QObject* parent = nullptr);

    int displayedYear() const noexcept;
    CalendarSystem calendarSystem() const noexcept;
    QString calendarSystemName() const;
    QStringList weekdayLabels() const;
    QVariantList monthModels() const;
    QVariantList calendarSystemOptions() const;
    void setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore);

public slots:
    void setDisplayedYear(int year);
    void setCalendarSystemByEnum(CalendarSystem system);

    Q_INVOKABLE void setCalendarSystemByValue(int value);
    Q_INVOKABLE void shiftYear(int delta);
    Q_INVOKABLE void focusToday();
    Q_INVOKABLE void requestYearView(const QString& reason = QString());
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
    void calendarSystemChanged();
    void yearViewChanged();
    void yearViewRequested(QString reason);

private:
    void rebuildYearModel();
    QCalendar resolveCalendarSystem() const;
    static QString calendarSystemLabel(CalendarSystem system);

    int m_displayedYear = 1970;
    CalendarSystem m_calendarSystem = Gregorian;
    QStringList m_weekdayLabels;
    QVariantList m_monthModels;
    ICalendarBoardStore* m_calendarBoardStore = nullptr;
};
