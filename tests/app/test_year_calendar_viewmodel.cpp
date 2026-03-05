#include "viewmodel/calendar/YearCalendarViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class YearCalendarViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustBuildGregorianYearView();
    void setCalendarSystemByValue_mustAcceptKnownEnumValuesOnly();
    void yearMutation_mustClampAndShiftYear();
    void requestYearView_mustEmitRequestSignal();
};

void YearCalendarViewModelTest::defaults_mustBuildGregorianYearView()
{
    YearCalendarViewModel viewModel;

    QCOMPARE(viewModel.calendarSystem(), YearCalendarViewModel::Gregorian);
    QCOMPARE(viewModel.displayedYear(), QDate::currentDate().year());

    const QStringList weekdayLabels = viewModel.weekdayLabels();
    QCOMPARE(weekdayLabels.size(), 7);

    const QVariantList monthModels = viewModel.monthModels();
    QVERIFY(!monthModels.isEmpty());

    const QVariantMap firstMonth = monthModels.first().toMap();
    QVERIFY(firstMonth.contains(QStringLiteral("monthLabel")));
    QVERIFY(firstMonth.contains(QStringLiteral("days")));

    const QVariantList firstMonthDays = firstMonth.value(QStringLiteral("days")).toList();
    QCOMPARE(firstMonthDays.size(), 42);
}

void YearCalendarViewModelTest::setCalendarSystemByValue_mustAcceptKnownEnumValuesOnly()
{
    YearCalendarViewModel viewModel;

    QSignalSpy systemChangedSpy(&viewModel, SIGNAL(calendarSystemChanged()));

    viewModel.setCalendarSystemByValue(static_cast<int>(YearCalendarViewModel::Julian));
    QCOMPARE(viewModel.calendarSystem(), YearCalendarViewModel::Julian);
    QCOMPARE(systemChangedSpy.count(), 1);
    QVERIFY(!viewModel.monthModels().isEmpty());

    viewModel.setCalendarSystemByValue(static_cast<int>(YearCalendarViewModel::IslamicCivil));
    QCOMPARE(viewModel.calendarSystem(), YearCalendarViewModel::IslamicCivil);
    QCOMPARE(systemChangedSpy.count(), 2);

    viewModel.setCalendarSystemByValue(999);
    QCOMPARE(viewModel.calendarSystem(), YearCalendarViewModel::IslamicCivil);
    QCOMPARE(systemChangedSpy.count(), 2);
}

void YearCalendarViewModelTest::yearMutation_mustClampAndShiftYear()
{
    YearCalendarViewModel viewModel;

    viewModel.setDisplayedYear(-120);
    QCOMPARE(viewModel.displayedYear(), 1);

    viewModel.shiftYear(-10);
    QCOMPARE(viewModel.displayedYear(), 1);

    viewModel.setDisplayedYear(12000);
    QCOMPARE(viewModel.displayedYear(), 9999);

    viewModel.shiftYear(1);
    QCOMPARE(viewModel.displayedYear(), 9999);

    viewModel.shiftYear(-1);
    QCOMPARE(viewModel.displayedYear(), 9998);
}

void YearCalendarViewModelTest::requestYearView_mustEmitRequestSignal()
{
    YearCalendarViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(yearViewRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(yearViewChanged()));

    viewModel.requestYearView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

QTEST_APPLESS_MAIN(YearCalendarViewModelTest)

#include "test_year_calendar_viewmodel.moc"
