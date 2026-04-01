#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class CalendarBoardStore;
class QDate;
class QLocale;

class AgendaViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayedDateIso READ displayedDateIso WRITE setDisplayedDateIso NOTIFY displayedDateIsoChanged)
    Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY agendaChanged)
    Q_PROPERTY(QVariantMap location READ location NOTIFY agendaChanged)
    Q_PROPERTY(QVariantMap weather READ weather NOTIFY agendaChanged)
    Q_PROPERTY(QVariantList allDayEvents READ allDayEvents NOTIFY agendaChanged)
    Q_PROPERTY(QVariantList timedEvents READ timedEvents NOTIFY agendaChanged)
    Q_PROPERTY(QVariantList agendaItems READ agendaItems NOTIFY agendaChanged)
    Q_PROPERTY(QVariantMap summary READ summary NOTIFY agendaChanged)

public:
    explicit AgendaViewModel(QObject* parent = nullptr);

    QString displayedDateIso() const;
    QString dateLabel() const;
    QVariantMap location() const;
    QVariantMap weather() const;
    QVariantList allDayEvents() const;
    QVariantList timedEvents() const;
    QVariantList agendaItems() const;
    QVariantMap summary() const;
    void setCalendarBoardStore(CalendarBoardStore* calendarBoardStore);

public slots:
    void setDisplayedDateIso(const QString& dateIso);

    Q_INVOKABLE void shiftDay(int deltaDays);
    Q_INVOKABLE void requestAgendaView(const QString& reason = QString());
    Q_INVOKABLE bool addEvent(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString());
    Q_INVOKABLE bool addAgendaItem(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString());
    Q_INVOKABLE QVariantList entriesForDate(const QString& dateIso) const;
    Q_INVOKABLE bool removeEntry(const QString& entryId);
    Q_INVOKABLE bool setAgendaItemCompleted(const QString& entryId, bool completed);
    Q_INVOKABLE bool toggleAgendaItemCompleted(const QString& entryId);
    Q_INVOKABLE void setLocation(
        const QString& cityName,
        const QString& regionName,
        const QString& countryCode,
        const QString& timeZoneId = QString());
    Q_INVOKABLE void setWeather(
        const QString& conditionText,
        int temperatureCelsius,
        int highCelsius,
        int lowCelsius,
        int precipitationPercent);

signals:
    void displayedDateIsoChanged();
    void agendaChanged();
    void agendaRequested(QString reason);

private:
    struct AgendaLocationModel
    {
        QString cityName;
        QString regionName;
        QString countryCode;
        QString timeZoneId;
    };

    struct AgendaWeatherModel
    {
        QString conditionText;
        QString iconName;
        int temperatureCelsius = 0;
        int highCelsius = 0;
        int lowCelsius = 0;
        int precipitationPercent = 0;
    };

    static bool parseIsoDate(const QString& dateIso, QDate* outDate);
    static QString formatDateLabel(const QDate& date, const QLocale& locale);
    static bool isAllDayEvent(const QVariantMap& entry);
    static QVariantMap toEventModel(const QVariantMap& entry, bool allDay);
    static QVariantMap toAgendaItemModel(const QVariantMap& entry);
    static QVariantMap toLocationVariant(const AgendaLocationModel& model);
    static QVariantMap toWeatherVariant(const AgendaWeatherModel& model);
    static AgendaWeatherModel buildSampleWeatherForDate(const QDate& date);

    void rebuildAgenda();

    CalendarBoardStore* m_calendarBoardStore = nullptr;
    QString m_displayedDateIso;
    QString m_dateLabel;
    AgendaLocationModel m_locationModel;
    AgendaWeatherModel m_weatherModel;
    bool m_customWeatherEnabled = false;
    QVariantList m_allDayEvents;
    QVariantList m_timedEvents;
    QVariantList m_agendaItems;
    QVariantMap m_summary;
};
