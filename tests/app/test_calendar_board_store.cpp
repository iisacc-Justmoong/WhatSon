#include "calendar/CalendarBoardStore.hpp"

#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class CalendarBoardStoreTest final : public QObject
{
    Q_OBJECT

private slots:
    void addEntry_mustRequireValidDateTimeAndTitle();
    void entriesForDate_mustReturnSortedEntriesWithCounts();
    void taskCompletionAndRemoval_mustMutateEntryState();
};

void CalendarBoardStoreTest::addEntry_mustRequireValidDateTimeAndTitle()
{
    CalendarBoardStore store;

    QVERIFY(store.addEvent(QStringLiteral("2026-04-12"), QStringLiteral("09:30"), QStringLiteral("Kickoff")));
    QVERIFY(store.addTask(QStringLiteral("2026-04-12"), QStringLiteral("18:00"), QStringLiteral("Send recap")));

    QVERIFY(!store.addEvent(QStringLiteral("2026-13-40"), QStringLiteral("09:30"), QStringLiteral("Invalid date")));
    QVERIFY(!store.addTask(QStringLiteral("2026-04-12"), QStringLiteral("9"), QStringLiteral("Invalid time")));
    QVERIFY(!store.addEvent(QStringLiteral("2026-04-12"), QStringLiteral("09:30"), QStringLiteral("   ")));
}

void CalendarBoardStoreTest::entriesForDate_mustReturnSortedEntriesWithCounts()
{
    CalendarBoardStore store;

    QVERIFY(store.addTask(QStringLiteral("2026-04-10"), QStringLiteral("13:00"), QStringLiteral("Task B")));
    QVERIFY(store.addEvent(QStringLiteral("2026-04-10"), QStringLiteral("09:00"), QStringLiteral("Event A")));
    QVERIFY(store.addTask(QStringLiteral("2026-04-10"), QStringLiteral("11:00"), QStringLiteral("Task A")));

    const QVariantList entries = store.entriesForDate(QStringLiteral("2026-04-10"));
    QCOMPARE(entries.size(), 3);

    const QVariantMap first = entries.at(0).toMap();
    const QVariantMap second = entries.at(1).toMap();
    const QVariantMap third = entries.at(2).toMap();
    QCOMPARE(first.value(QStringLiteral("time")).toString(), QStringLiteral("09:00"));
    QCOMPARE(second.value(QStringLiteral("time")).toString(), QStringLiteral("11:00"));
    QCOMPARE(third.value(QStringLiteral("time")).toString(), QStringLiteral("13:00"));

    const QVariantMap counts = store.countsForDate(QStringLiteral("2026-04-10"));
    QCOMPARE(counts.value(QStringLiteral("eventCount")).toInt(), 1);
    QCOMPARE(counts.value(QStringLiteral("taskCount")).toInt(), 2);
    QCOMPARE(counts.value(QStringLiteral("entryCount")).toInt(), 3);
}

void CalendarBoardStoreTest::taskCompletionAndRemoval_mustMutateEntryState()
{
    CalendarBoardStore store;
    QSignalSpy changedSpy(&store, SIGNAL(entriesChanged()));

    QVERIFY(store.addTask(QStringLiteral("2026-04-18"), QStringLiteral("08:30"), QStringLiteral("Morning routine")));
    const QVariantList initialEntries = store.entriesForDate(QStringLiteral("2026-04-18"));
    QCOMPARE(initialEntries.size(), 1);
    const QString entryId = initialEntries.first().toMap().value(QStringLiteral("id")).toString();
    QVERIFY(!entryId.isEmpty());

    QVERIFY(store.setTaskCompleted(entryId, true));
    const QVariantList completedEntries = store.entriesForDate(QStringLiteral("2026-04-18"));
    QCOMPARE(completedEntries.first().toMap().value(QStringLiteral("completed")).toBool(), true);

    QVERIFY(store.removeEntry(entryId));
    const QVariantList removedEntries = store.entriesForDate(QStringLiteral("2026-04-18"));
    QVERIFY(removedEntries.isEmpty());
    QVERIFY(changedSpy.count() >= 3);
}

QTEST_APPLESS_MAIN(CalendarBoardStoreTest)

#include "test_calendar_board_store.moc"
