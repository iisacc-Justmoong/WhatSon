#include "calendar/CalendarBoardStore.hpp"
#include "viewmodel/calendar/MonthCalendarViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class MonthCalendarViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaults_mustBuildGregorianMonthView();
    void setCalendarSystemByValue_mustAcceptKnownEnumValuesOnly();
    void monthMutation_mustClampAndShiftMonth();
    void focusToday_mustAlignWithCurrentDate();
    void requestMonthView_mustEmitRequestSignal();
    void boardEntries_mustExposeDayCountsAndSelectedDateEntries();
};

void MonthCalendarViewModelTest::defaults_mustBuildGregorianMonthView()
{
    MonthCalendarViewModel viewModel;

    QCOMPARE(viewModel.calendarSystem(), MonthCalendarViewModel::Gregorian);
    QCOMPARE(viewModel.displayedYear(), QDate::currentDate().year());
    QCOMPARE(viewModel.displayedMonth(), QDate::currentDate().month());
    QVERIFY(!viewModel.monthLabel().trimmed().isEmpty());

    const QStringList weekdayLabels = viewModel.weekdayLabels();
    QCOMPARE(weekdayLabels.size(), 7);

    const QVariantList dayModels = viewModel.dayModels();
    QCOMPARE(dayModels.size(), 42);
    QVERIFY(dayModels.first().toMap().contains(QStringLiteral("day")));
}

void MonthCalendarViewModelTest::setCalendarSystemByValue_mustAcceptKnownEnumValuesOnly()
{
    MonthCalendarViewModel viewModel;

    QSignalSpy systemChangedSpy(&viewModel, SIGNAL(calendarSystemChanged()));

    viewModel.setCalendarSystemByValue(static_cast<int>(MonthCalendarViewModel::Julian));
    QCOMPARE(viewModel.calendarSystem(), MonthCalendarViewModel::Julian);
    QCOMPARE(systemChangedSpy.count(), 1);
    QCOMPARE(viewModel.dayModels().size(), 42);

    viewModel.setCalendarSystemByValue(static_cast<int>(MonthCalendarViewModel::IslamicCivil));
    QCOMPARE(viewModel.calendarSystem(), MonthCalendarViewModel::IslamicCivil);
    QCOMPARE(systemChangedSpy.count(), 2);

    viewModel.setCalendarSystemByValue(999);
    QCOMPARE(viewModel.calendarSystem(), MonthCalendarViewModel::IslamicCivil);
    QCOMPARE(systemChangedSpy.count(), 2);
}

void MonthCalendarViewModelTest::monthMutation_mustClampAndShiftMonth()
{
    MonthCalendarViewModel viewModel;

    viewModel.setDisplayedYear(2024);
    viewModel.setDisplayedMonth(1);
    QCOMPARE(viewModel.displayedYear(), 2024);
    QCOMPARE(viewModel.displayedMonth(), 1);

    viewModel.shiftMonth(-1);
    QCOMPARE(viewModel.displayedYear(), 2023);
    QCOMPARE(viewModel.displayedMonth(), 12);

    viewModel.shiftMonth(2);
    QCOMPARE(viewModel.displayedYear(), 2024);
    QCOMPARE(viewModel.displayedMonth(), 2);

    viewModel.setDisplayedYear(1);
    viewModel.setDisplayedMonth(-10);
    QCOMPARE(viewModel.displayedYear(), 1);
    QCOMPARE(viewModel.displayedMonth(), 1);

    viewModel.shiftMonth(-1);
    QCOMPARE(viewModel.displayedYear(), 1);
    QCOMPARE(viewModel.displayedMonth(), 1);
}

void MonthCalendarViewModelTest::focusToday_mustAlignWithCurrentDate()
{
    MonthCalendarViewModel viewModel;
    viewModel.setDisplayedYear(1999);
    viewModel.setDisplayedMonth(7);
    viewModel.setSelectedDateIso(QStringLiteral("1999-07-01"));

    viewModel.focusToday();

    const QDate today = QDate::currentDate();
    QCOMPARE(viewModel.displayedYear(), today.year());
    QCOMPARE(viewModel.displayedMonth(), today.month());
    QCOMPARE(viewModel.selectedDateIso(), today.toString(Qt::ISODate));
}

void MonthCalendarViewModelTest::requestMonthView_mustEmitRequestSignal()
{
    MonthCalendarViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(monthViewRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(monthViewChanged()));

    viewModel.requestMonthView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

void MonthCalendarViewModelTest::boardEntries_mustExposeDayCountsAndSelectedDateEntries()
{
    CalendarBoardStore boardStore;
    MonthCalendarViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedYear(2026);
    viewModel.setDisplayedMonth(4);

    QVERIFY(viewModel.addEvent(
        QStringLiteral("2026-04-12"),
        QStringLiteral("09:00"),
        QStringLiteral("Kickoff"),
        QStringLiteral("Roadmap sync")));
    QVERIFY(viewModel.addTask(
        QStringLiteral("2026-04-12"),
        QStringLiteral("15:30"),
        QStringLiteral("Share notes")));

    viewModel.setSelectedDateIso(QStringLiteral("2026-04-12"));
    const QVariantList selectedEntries = viewModel.selectedDateEntries();
    QCOMPARE(selectedEntries.size(), 2);

    bool foundDayCell = false;
    const QVariantList dayModels = viewModel.dayModels();
    for (const QVariant& dayModelValue : dayModels)
    {
        const QVariantMap dayModel = dayModelValue.toMap();
        if (dayModel.value(QStringLiteral("dateIso")).toString() != QStringLiteral("2026-04-12"))
        {
            continue;
        }

        foundDayCell = true;
        QCOMPARE(dayModel.value(QStringLiteral("eventCount")).toInt(), 1);
        QCOMPARE(dayModel.value(QStringLiteral("taskCount")).toInt(), 1);
        QCOMPARE(dayModel.value(QStringLiteral("entryCount")).toInt(), 2);
    }
    QVERIFY(foundDayCell);

    QString taskId;
    for (const QVariant& entryValue : selectedEntries)
    {
        const QVariantMap entry = entryValue.toMap();
        if (entry.value(QStringLiteral("type")).toString() == QStringLiteral("task"))
        {
            taskId = entry.value(QStringLiteral("id")).toString();
            break;
        }
    }
    QVERIFY(!taskId.isEmpty());
    QVERIFY(viewModel.setTaskCompleted(taskId, true));

    const QVariantList updatedEntries = viewModel.selectedDateEntries();
    bool taskCompleted = false;
    for (const QVariant& entryValue : updatedEntries)
    {
        const QVariantMap entry = entryValue.toMap();
        if (entry.value(QStringLiteral("id")).toString() == taskId)
        {
            taskCompleted = entry.value(QStringLiteral("completed")).toBool();
            break;
        }
    }
    QVERIFY(taskCompleted);
}

QTEST_APPLESS_MAIN(MonthCalendarViewModelTest)

#include "test_month_calendar_viewmodel.moc"
