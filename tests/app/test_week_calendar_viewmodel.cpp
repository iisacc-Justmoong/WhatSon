#include "calendar/CalendarBoardStore.hpp"
#include "viewmodel/calendar/WeekCalendarViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class WeekCalendarViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaults_mustBuildSevenDayWeekModel();
    void shiftWeek_mustMoveDisplayedWeekStart();
    void boardEntries_mustExposeDayCounts();
    void requestWeekView_mustEmitRequestSignal();
};

void WeekCalendarViewModelTest::defaults_mustBuildSevenDayWeekModel()
{
    WeekCalendarViewModel viewModel;

    QVERIFY(!viewModel.displayedWeekStartIso().trimmed().isEmpty());
    QVERIFY(!viewModel.weekLabel().trimmed().isEmpty());
    QCOMPARE(viewModel.weekdayLabels().size(), 7);
    QCOMPARE(viewModel.dayModels().size(), 7);
}

void WeekCalendarViewModelTest::shiftWeek_mustMoveDisplayedWeekStart()
{
    WeekCalendarViewModel viewModel;

    viewModel.setDisplayedWeekStartIso(QStringLiteral("2026-07-06"));
    viewModel.shiftWeek(1);
    QCOMPARE(viewModel.displayedWeekStartIso(), QStringLiteral("2026-07-13"));

    viewModel.shiftWeek(-2);
    QCOMPARE(viewModel.displayedWeekStartIso(), QStringLiteral("2026-06-29"));
}

void WeekCalendarViewModelTest::boardEntries_mustExposeDayCounts()
{
    CalendarBoardStore boardStore;
    WeekCalendarViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedWeekStartIso(QStringLiteral("2026-07-06"));

    QVERIFY(viewModel.addEvent(
        QStringLiteral("2026-07-08"),
        QStringLiteral("10:00"),
        QStringLiteral("Planning")));
    QVERIFY(viewModel.addTask(
        QStringLiteral("2026-07-08"),
        QStringLiteral("16:00"),
        QStringLiteral("Publish notes")));

    const QVariantList dayModels = viewModel.dayModels();
    bool foundTargetDay = false;
    for (const QVariant& dayValue : dayModels)
    {
        const QVariantMap dayModel = dayValue.toMap();
        if (dayModel.value(QStringLiteral("dateIso")).toString() != QStringLiteral("2026-07-08"))
        {
            continue;
        }

        foundTargetDay = true;
        QCOMPARE(dayModel.value(QStringLiteral("eventCount")).toInt(), 1);
        QCOMPARE(dayModel.value(QStringLiteral("taskCount")).toInt(), 1);
        QCOMPARE(dayModel.value(QStringLiteral("entryCount")).toInt(), 2);
        break;
    }
    QVERIFY(foundTargetDay);
}

void WeekCalendarViewModelTest::requestWeekView_mustEmitRequestSignal()
{
    WeekCalendarViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(weekViewRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(weekViewChanged()));

    viewModel.requestWeekView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

QTEST_APPLESS_MAIN(WeekCalendarViewModelTest)

#include "test_week_calendar_viewmodel.moc"

