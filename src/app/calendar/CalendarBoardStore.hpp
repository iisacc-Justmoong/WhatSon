#pragma once

#include <QDate>
#include <QObject>
#include <QString>
#include <QTime>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class CalendarBoardStore final : public QObject
{
    Q_OBJECT

public:
    explicit CalendarBoardStore(QObject* parent = nullptr);
    ~CalendarBoardStore() override;

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
    Q_INVOKABLE QVariantMap countsForDate(const QString& dateIso) const;
    Q_INVOKABLE bool removeEntry(const QString& entryId);
    Q_INVOKABLE bool setTaskCompleted(const QString& entryId, bool completed);

signals:
    void entriesChanged();
    void entryAdded(QString entryId, QString entryType, QString dateIso, QString timeText);
    void entryRemoved(QString entryId);
    void entryUpdated(QString entryId);

private:
    enum class EntryType
    {
        Event,
        Task
    };

    struct CalendarEntry
    {
        QString id;
        EntryType type = EntryType::Event;
        QDate date;
        QTime time;
        QString title;
        QString detail;
        bool completed = false;
    };

    bool addEntry(
        EntryType type,
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail);
    static bool parseIsoDate(const QString& dateIso, QDate* outDate);
    static bool parseTimeText(const QString& timeText, QTime* outTime);
    static QString entryTypeName(EntryType type);
    static QVariantMap toVariantMap(const CalendarEntry& entry);
    void sortEntries();

    QVector<CalendarEntry> m_entries;
};
