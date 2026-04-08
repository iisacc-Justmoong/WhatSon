#pragma once

#include "ICalendarBoardStore.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QDate>
#include <QTime>
#include <QTimer>
#include <QVector>

class CalendarBoardStore final : public ICalendarBoardStore
{
    Q_OBJECT

public:
    explicit CalendarBoardStore(QObject* parent = nullptr);
    ~CalendarBoardStore() override;

    Q_INVOKABLE bool addEvent(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString()) override;
    Q_INVOKABLE bool addTask(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString()) override;
    Q_INVOKABLE QVariantList entriesForDate(const QString& dateIso) const override;
    Q_INVOKABLE QVariantMap countsForDate(const QString& dateIso) const override;
    Q_INVOKABLE bool removeEntry(const QString& entryId) override;
    Q_INVOKABLE bool setTaskCompleted(const QString& entryId, bool completed) override;
    void setProjectedNotesHubPath(const QString& wshubPath);
    QString projectedNotesHubPath() const;
    bool reloadProjectedNotes(QString* errorMessage = nullptr);

public slots:
    void requestProjectedNotesReload();

public:
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
        bool allDay = false;
        bool readOnly = false;
        bool projected = false;
        QString sourceKind;
        QString sourceId;
        QString dateRole;
    };

private:
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
    static void sortEntries(QVector<CalendarEntry>* entries);
    void clearProjectedEntries();
    void replaceProjectedEntries(QVector<CalendarEntry> entries);
    QVector<CalendarEntry> buildProjectedNoteEntries(const QVector<LibraryNoteRecord>& notes) const;

    QVector<CalendarEntry> m_entries;
    QVector<CalendarEntry> m_projectedEntries;
    QString m_projectedNotesHubPath;
    QTimer m_projectedNotesReloadTimer;
};
