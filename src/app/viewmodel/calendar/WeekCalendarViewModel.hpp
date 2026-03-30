#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class CalendarBoardStore;
class QDate;
class QLocale;

class WeekCalendarViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QString displayedWeekStartIso READ displayedWeekStartIso WRITE setDisplayedWeekStartIso
            NOTIFY displayedWeekStartIsoChanged)
    Q_PROPERTY(QString weekLabel READ weekLabel NOTIFY weekViewChanged)
    Q_PROPERTY(QStringList weekdayLabels READ weekdayLabels NOTIFY weekViewChanged)
    Q_PROPERTY(QVariantList dayModels READ dayModels NOTIFY weekViewChanged)

public:
    explicit WeekCalendarViewModel(QObject* parent = nullptr);

    QString displayedWeekStartIso() const;
    QString weekLabel() const;
    QStringList weekdayLabels() const;
    QVariantList dayModels() const;
    void setCalendarBoardStore(CalendarBoardStore* calendarBoardStore);

public slots:
    void setDisplayedWeekStartIso(const QString& dateIso);

    Q_INVOKABLE void shiftWeek(int deltaWeeks);
    Q_INVOKABLE void requestWeekView(const QString& reason = QString());
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
    void displayedWeekStartIsoChanged();
    void weekViewChanged();
    void weekViewRequested(QString reason);

private:
    void rebuildWeekModel();
    static bool parseIsoDate(const QString& dateIso, QDate* outDate);
    static QDate startOfWeek(const QDate& date, const QLocale& locale);
    static QString formatWeekLabel(const QDate& weekStartDate, const QLocale& locale);

    CalendarBoardStore* m_calendarBoardStore = nullptr;
    QString m_displayedWeekStartIso;
    QString m_weekLabel;
    QStringList m_weekdayLabels;
    QVariantList m_dayModels;
};

