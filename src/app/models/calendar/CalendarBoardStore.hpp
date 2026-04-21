#pragma once

#include "ICalendarBoardStore.hpp"
#include "models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QDate>
#include <QHash>
#include <QTime>
#include <QTimer>
#include <QVector>

#include <functional>

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
    void setProjectedNotesProvider(std::function<QVector<LibraryNoteRecord>()> provider);
    void reloadProjectedNotesFromSnapshot(const QVector<LibraryNoteRecord>& notes);
    bool upsertProjectedNote(const LibraryNoteRecord& note);
    bool removeProjectedNoteBySourceId(const QString& noteId);
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
    struct EntryCounts final
    {
        int eventCount = 0;
        int taskCount = 0;
    };

    bool addEntry(
        EntryType type,
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail);
    void rebuildProjectedIndexes();
    static bool parseIsoDate(const QString& dateIso, QDate* outDate);
    static bool parseTimeText(const QString& timeText, QTime* outTime);
    static QString dateKey(const QDate& date);
    static QString entryTypeName(EntryType type);
    static QVariantMap toCountsVariant(const EntryCounts& counts);
    static QVariantMap toVariantMap(const CalendarEntry& entry);
    static void sortEntries(QVector<CalendarEntry>* entries);
    static void indexEntries(
        const QVector<CalendarEntry>& source,
        QHash<QString, QVector<CalendarEntry>>* entriesByDate,
        QHash<QString, EntryCounts>* countsByDate);
    static void insertIndexedEntry(
        const CalendarEntry& entry,
        QHash<QString, QVector<CalendarEntry>>* entriesByDate,
        QHash<QString, EntryCounts>* countsByDate);
    static void removeIndexedEntry(
        const CalendarEntry& entry,
        QHash<QString, QVector<CalendarEntry>>* entriesByDate,
        QHash<QString, EntryCounts>* countsByDate);
    static void updateIndexedEntry(
        const CalendarEntry& entry,
        QHash<QString, QVector<CalendarEntry>>* entriesByDate);
    static QVariantList toVariantList(const QVector<CalendarEntry>& entries);
    static QVector<CalendarEntry> mergeIndexedEntries(
        const QVector<CalendarEntry>& manualEntries,
        const QVector<CalendarEntry>& projectedEntries);
    void cacheProjectedEntriesForQueries(QVector<CalendarEntry> entries);
    void clearProjectedEntries();
    void replaceProjectedEntries(QVector<CalendarEntry> entries);
    bool buildProjectedNoteEntry(const LibraryNoteRecord& note, CalendarEntry* outEntry) const;
    QVector<CalendarEntry> buildProjectedNoteEntries(const QVector<LibraryNoteRecord>& notes) const;
    QVector<CalendarEntry> projectedEntriesForDateQuery(const QDate& date) const;
    EntryCounts projectedCountsForDateQuery(const QDate& date) const;

    QVector<CalendarEntry> m_entries;
    QHash<QString, QVector<CalendarEntry>> m_entriesByDate;
    QHash<QString, EntryCounts> m_entryCountsByDate;
    QHash<QString, CalendarEntry> m_projectedEntriesBySourceId;
    QHash<QString, QVector<CalendarEntry>> m_projectedEntriesByDate;
    QHash<QString, EntryCounts> m_projectedEntryCountsByDate;
    QString m_projectedNotesHubPath;
    QTimer m_projectedNotesReloadTimer;
    std::function<QVector<LibraryNoteRecord>()> m_projectedNotesProvider;
    bool m_projectedEntriesInitialized = false;
};
