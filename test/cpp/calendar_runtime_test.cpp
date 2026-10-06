#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QtTest>

class CalendarRuntimeTest : public QObject {
    Q_OBJECT
private slots:
    void pagePublishesTheCloseRequest_data() {
        QTest::addColumn<QString>("page");
        for (const auto &name : {"Day", "Week", "Month", "Year"}) QTest::newRow(name) << QString::fromLatin1(name);
    }
    void pagePublishesTheCloseRequest() {
        QFETCH(QString, page);
        QQmlEngine engine;
        engine.addImportPath(QStringLiteral(WHATSON_TEST_LVRS_IMPORT_PATH));
        QQmlComponent component(&engine, QUrl::fromLocalFile(QString(WHATSON_SOURCE_DIRECTORY)
            + "/src/app/qml/view/calendar/" + page + "CalendarPage.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object != nullptr, qPrintable(component.errorString()));
        QSignalSpy closed(object.get(), SIGNAL(overlayCloseRequested()));
        QVERIFY(closed.isValid());
        QVERIFY(QMetaObject::invokeMethod(object.get(), "overlayCloseRequested"));
        QCOMPARE(closed.size(), 1);
        if (page == "Year") {
            QSignalSpy month(object.get(), SIGNAL(monthCalendarOpenRequested(int,int,QString)));
            QVERIFY(month.isValid());
            QVERIFY(QMetaObject::invokeMethod(object.get(), "monthCalendarOpenRequested",
                Q_ARG(int, 2026), Q_ARG(int, 10), Q_ARG(QString, QStringLiteral("2026-10-06"))));
            QCOMPARE(month.size(), 1);
        }
    }
};
QTEST_MAIN(CalendarRuntimeTest)
#include "calendar_runtime_test.moc"
