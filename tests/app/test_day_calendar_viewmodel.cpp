#include "calendar/CalendarBoardStore.hpp"
#include "viewmodel/calendar/DayCalendarViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class DayCalendarViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaults_mustBuildTodayDayTimeline();
    void shiftDay_mustMoveDisplayedDate();
    void boardEntries_mustPopulateTimeSlots();
    void requestDayView_mustEmitRequestSignal();
};

void DayCalendarViewModelTest::defaults_mustBuildTodayDayTimeline()
{
    DayCalendarViewModel viewModel;

    QCOMPARE(viewModel.displayedDateIso(), QDate::currentDate().toString(Qt::ISODate));
    QVERIFY(!viewModel.dayLabel().trimmed().isEmpty());
    QCOMPARE(viewModel.timeSlots().size(), 24);
}

void DayCalendarViewModelTest::shiftDay_mustMoveDisplayedDate()
{
    DayCalendarViewModel viewModel;

    viewModel.setDisplayedDateIso(QStringLiteral("2026-06-10"));
    viewModel.shiftDay(-1);
    QCOMPARE(viewModel.displayedDateIso(), QStringLiteral("2026-06-09"));

    viewModel.shiftDay(2);
    QCOMPARE(viewModel.displayedDateIso(), QStringLiteral("2026-06-11"));
}

void DayCalendarViewModelTest::boardEntries_mustPopulateTimeSlots()
{
    CalendarBoardStore boardStore;
    DayCalendarViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedDateIso(QStringLiteral("2026-06-12"));

    QVERIFY(viewModel.addEvent(QStringLiteral(""), QStringLiteral("09:30"), QStringLiteral("Daily stand-up")));
    QVERIFY(viewModel.addTask(QStringLiteral(""), QStringLiteral("14:10"), QStringLiteral("Follow-up")));

    const QVariantList dayEntries = viewModel.dayEntries();
    QCOMPARE(dayEntries.size(), 2);

    bool foundMorningSlot = false;
    const QVariantList timeSlots = viewModel.timeSlots();
    for (const QVariant& slotValue : timeSlots)
    {
        const QVariantMap slot = slotValue.toMap();
        if (slot.value(QStringLiteral("hour")).toInt() != 9)
        {
            continue;
        }

        foundMorningSlot = true;
        QCOMPARE(slot.value(QStringLiteral("entryCount")).toInt(), 1);
        break;
    }
    QVERIFY(foundMorningSlot);
}

void DayCalendarViewModelTest::requestDayView_mustEmitRequestSignal()
{
    DayCalendarViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(dayViewRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(dayViewChanged()));

    viewModel.requestDayView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

QTEST_APPLESS_MAIN(DayCalendarViewModelTest)

#include "test_day_calendar_viewmodel.moc"

