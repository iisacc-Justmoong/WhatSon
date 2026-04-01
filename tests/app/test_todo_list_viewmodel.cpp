#include "calendar/CalendarBoardStore.hpp"
#include "viewmodel/calendar/TodoListViewModel.hpp"

#include <QDate>
#include <QSignalSpy>
#include <QVariantMap>
#include <QtTest>

class TodoListViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void defaults_mustExposeDateLocationAndWeather();
    void boardEntries_mustSplitAllDayTimedAndTaskSections();
    void requestTodoListView_mustEmitRequestSignal();
    void toggleTaskCompleted_mustFlipTaskState();
};

void TodoListViewModelTest::defaults_mustExposeDateLocationAndWeather()
{
    TodoListViewModel viewModel;

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

void TodoListViewModelTest::boardEntries_mustSplitAllDayTimedAndTaskSections()
{
    CalendarBoardStore boardStore;
    TodoListViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedDateIso(QStringLiteral("2026-08-14"));

    QVERIFY(viewModel.addEvent(QStringLiteral(""), QStringLiteral("00:00"), QStringLiteral("Company holiday")));
    QVERIFY(viewModel.addEvent(QStringLiteral(""), QStringLiteral("09:30"), QStringLiteral("Daily review")));
    QVERIFY(viewModel.addTask(QStringLiteral(""), QStringLiteral("11:10"), QStringLiteral("Ship release")));

    QCOMPARE(viewModel.allDayEvents().size(), 1);
    QCOMPARE(viewModel.timedEvents().size(), 1);
    QCOMPARE(viewModel.tasks().size(), 1);

    const QVariantMap summary = viewModel.summary();
    QCOMPARE(summary.value(QStringLiteral("allDayEventCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("timedEventCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("taskCount")).toInt(), 1);
    QCOMPARE(summary.value(QStringLiteral("entryCount")).toInt(), 3);
}

void TodoListViewModelTest::requestTodoListView_mustEmitRequestSignal()
{
    TodoListViewModel viewModel;

    QSignalSpy requestSpy(&viewModel, SIGNAL(todoListRequested(QString)));
    QSignalSpy changedSpy(&viewModel, SIGNAL(todoListChanged()));

    viewModel.requestTodoListView(QStringLiteral("manual-refresh"));

    QCOMPARE(requestSpy.count(), 1);
    QVERIFY(changedSpy.count() >= 1);

    const QList<QVariant> firstArgs = requestSpy.takeFirst();
    QCOMPARE(firstArgs.size(), 1);
    QCOMPARE(firstArgs.first().toString(), QStringLiteral("manual-refresh"));
}

void TodoListViewModelTest::toggleTaskCompleted_mustFlipTaskState()
{
    CalendarBoardStore boardStore;
    TodoListViewModel viewModel;
    viewModel.setCalendarBoardStore(&boardStore);
    viewModel.setDisplayedDateIso(QStringLiteral("2026-08-14"));

    QVERIFY(viewModel.addTask(
        QStringLiteral(""),
        QStringLiteral("13:45"),
        QStringLiteral("Prepare release notes")));

    const QVariantList initialTasks = viewModel.tasks();
    QCOMPARE(initialTasks.size(), 1);
    const QString taskId = initialTasks.first().toMap().value(QStringLiteral("id")).toString();
    QVERIFY(!taskId.isEmpty());

    QVERIFY(viewModel.toggleTaskCompleted(taskId));

    const QVariantList completedTasks = viewModel.tasks();
    QCOMPARE(completedTasks.size(), 1);
    QVERIFY(completedTasks.first().toMap().value(QStringLiteral("completed")).toBool());

    QVERIFY(viewModel.toggleTaskCompleted(taskId));

    const QVariantList reopenedTasks = viewModel.tasks();
    QCOMPARE(reopenedTasks.size(), 1);
    QVERIFY(!reopenedTasks.first().toMap().value(QStringLiteral("completed")).toBool());
}

QTEST_APPLESS_MAIN(TodoListViewModelTest)

#include "test_todo_list_viewmodel.moc"
