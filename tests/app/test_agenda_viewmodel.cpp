#include "calendar/CalendarBoardStore.hpp"
#include "viewmodel/calendar/AgendaViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class AgendaViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaults_mustExposeDateLocationAndWeather();
    void boardEntries_mustSplitAllDayTimedAndAgendaSections();
    void requestAgendaView_mustEmitRequestSignal();
    void toggleAgendaItemCompleted_mustFlipState();
};

void AgendaViewModelTest::defaults_mustExposeDateLocationAndWeather()
{
    AgendaViewModel viewModel;

    QCOMPARE(viewModel.displayedDateIso(), QDate::currentDate().toString(Qt::ISODate));
    QVERIFY(!viewModel.dateLabel().trimmed().isEmpty());

    const QVariantMap location = viewModel.location();
    QVERIFY(location.contains(QStringLiteral("cityName")));
    QVERIFY(location.contains(QStringLiteral("displayName")));
    QVERIFY(!location.value(QStringLiteral("displayName")).toString().trimmed().isEmpty());

    const QVariantMap weather = viewModel.weather();
    QVERIFY(weather.contains(QStringLiteral("conditionText")));
    QVERIFY(weather.contains(QStringLiteral("temperatureText")));
    QVERIFY(!weather.value(QStringLiteral("conditionText")).toString().trimmed().isEmpty());
}

void AgendaViewModelTest::boardEntries_mustSplitAllDayTimedAndAgendaSections()
{
    CalendarBoardStore boardStore;
    AgendaViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedDateIso(QStringLiteral("2026-08-14"));

    QVERIFY(viewModel.addEvent(QStringLiteral(""), QStringLiteral("00:00"), QStringLiteral("Company holiday")));
    QVERIFY(viewModel.addEvent(QStringLiteral(""), QStringLiteral("09:30"), QStringLiteral("Daily review")));
    QVERIFY(viewModel.addAgendaItem(QStringLiteral(""), QStringLiteral("11:10"), QStringLiteral("Ship release")));

    QCOMPARE(viewModel.allDayEvents().size(), 1);
    QCOMPARE(viewModel.timedEvents().size(), 1);
    QCOMPARE(viewModel.agendaItems().size(), 1);

    const QVariantMap summary = viewModel.summary();
    QCOMPARE(summary.value(QStringLiteral("allDayEventCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("timedEventCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("agendaItemCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("entryCount")).toInt(), 3);
}

void AgendaViewModelTest::requestAgendaView_mustEmitRequestSignal()
{
    AgendaViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(agendaRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(agendaChanged()));

    viewModel.requestAgendaView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

void AgendaViewModelTest::toggleAgendaItemCompleted_mustFlipState()
{
    CalendarBoardStore boardStore;
    AgendaViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedDateIso(QStringLiteral("2026-08-14"));

    QVERIFY(viewModel.addAgendaItem(
        QStringLiteral(""),
        QStringLiteral("13:45"),
        QStringLiteral("Prepare release notes")));

    const QVariantList initialAgendaItems = viewModel.agendaItems();
    QCOMPARE(initialAgendaItems.size(), 1);
    const QString agendaItemId = initialAgendaItems.first().toMap().value(QStringLiteral("id")).toString();
    QVERIFY(!agendaItemId.isEmpty());

    QVERIFY(viewModel.toggleAgendaItemCompleted(agendaItemId));

    const QVariantList completedAgendaItems = viewModel.agendaItems();
    QCOMPARE(completedAgendaItems.size(), 1);
    QVERIFY(completedAgendaItems.first().toMap().value(QStringLiteral("completed")).toBool());

    QVERIFY(viewModel.toggleAgendaItemCompleted(agendaItemId));

    const QVariantList reopenedAgendaItems = viewModel.agendaItems();
    QCOMPARE(reopenedAgendaItems.size(), 1);
    QVERIFY(!reopenedAgendaItems.first().toMap().value(QStringLiteral("completed")).toBool());
}

QTEST_APPLESS_MAIN(AgendaViewModelTest)

#include "test_agenda_viewmodel.moc"
