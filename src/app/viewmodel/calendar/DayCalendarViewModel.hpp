#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ICalendarBoardStore;
class QDate;
class QLocale;

class DayCalendarViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayedDateIso READ displayedDateIso WRITE setDisplayedDateIso NOTIFY displayedDateIsoChanged)
    Q_PROPERTY(QString dayLabel READ dayLabel NOTIFY dayViewChanged)
    Q_PROPERTY(QVariantList dayEntries READ dayEntries NOTIFY dayEntriesChanged)
    Q_PROPERTY(QVariantList timeSlots READ timeSlots NOTIFY dayViewChanged)

public:
    explicit DayCalendarViewModel(QObject* parent = nullptr);

    QString displayedDateIso() const;
    QString dayLabel() const;
    QVariantList dayEntries() const;
    QVariantList timeSlots() const;
    void setCalendarBoardStore(ICalendarBoardStore* calendarBoardStore);

public slots:
    void setDisplayedDateIso(const QString& dateIso);

    Q_INVOKABLE void shiftDay(int deltaDays);
    Q_INVOKABLE void requestDayView(const QString& reason = QString());
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
    void displayedDateIsoChanged();
    void dayEntriesChanged();
    void dayViewChanged();
    void dayViewRequested(QString reason);

private:
    void rebuildDayModel();
    static bool parseIsoDate(const QString& dateIso, QDate* outDate);
    static QString formatDayLabel(const QDate& date, const QLocale& locale);

    ICalendarBoardStore* m_calendarBoardStore = nullptr;
    QString m_displayedDateIso;
    QString m_dayLabel;
    QVariantList m_dayEntries;
    QVariantList m_timeSlots;
};
