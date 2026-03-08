#include "calendar/SystemCalendarStore.hpp"

#include <QDate>
#include <QLocale>
#include <QTimeZone>
#include <QtTest>

class SystemCalendarStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void constructor_readsSystemLocaleMetadata();
    void formatNoteDate_usesSystemShortDateFormat();
};

void SystemCalendarStoreTest::constructor_readsSystemLocaleMetadata()
{
    SystemCalendarStore store;
    const QLocale systemLocale = QLocale::system();
    const QVariantMap snapshot = store.snapshot();

    QCOMPARE(store.localeName(), systemLocale.name());
    QCOMPARE(store.bcp47Name(), systemLocale.bcp47Name());
    QCOMPARE(store.languageName(), QLocale::languageToString(systemLocale.language()));
    QCOMPARE(store.territoryName(), QLocale::territoryToString(systemLocale.territory()));
    QCOMPARE(store.timeZoneId(), QString::fromUtf8(QTimeZone::systemTimeZoneId()));
    QCOMPARE(store.uiLanguages(), systemLocale.uiLanguages());
    QCOMPARE(store.shortDateFormat(), systemLocale.dateFormat(QLocale::ShortFormat).trimmed().isEmpty()
                                          ? QStringLiteral("Date")
                                          : systemLocale.dateFormat(QLocale::ShortFormat).trimmed());
    QCOMPARE(store.longDateFormat(), systemLocale.dateFormat(QLocale::LongFormat).trimmed().isEmpty()
                                         ? QStringLiteral("Date")
                                         : systemLocale.dateFormat(QLocale::LongFormat).trimmed());
    QCOMPARE(store.shortDatePlaceholderText(), store.shortDateFormat());
    QCOMPARE(store.shortDateExample(),
             systemLocale.toString(QDate::currentDate(), QLocale::ShortFormat).trimmed().isEmpty()
                 ? QDate::currentDate().toString(Qt::TextDate)
                 : systemLocale.toString(QDate::currentDate(), QLocale::ShortFormat).trimmed());
    QCOMPARE(store.firstDayOfWeek(), static_cast<int>(systemLocale.firstDayOfWeek()));
    QCOMPARE(snapshot.value(QStringLiteral("localeName")).toString(), store.localeName());
    QCOMPARE(snapshot.value(QStringLiteral("shortDateFormat")).toString(), store.shortDateFormat());
}

void SystemCalendarStoreTest::formatNoteDate_usesSystemShortDateFormat()
{
    SystemCalendarStore store;
    const QLocale systemLocale = QLocale::system();
    const QDate date(2026, 3, 8);
    const QString expected = systemLocale.toString(date, QLocale::ShortFormat).trimmed().isEmpty()
                                 ? date.toString(Qt::TextDate)
                                 : systemLocale.toString(date, QLocale::ShortFormat).trimmed();

    QCOMPARE(store.formatNoteDate(QStringLiteral("2026-03-08-12-34-56")), expected);
    QCOMPARE(store.formatNoteDate(QStringLiteral(""), QStringLiteral("2026-03-08")), expected);
    QCOMPARE(SystemCalendarStore::formatNoteDateForSystem(QStringLiteral("2026-03-08")), expected);
    QCOMPARE(store.formatShortDate(QStringLiteral("2026-03-08")), expected);
    QCOMPARE(store.formatNoteDate(QStringLiteral("not-a-date")), QString());
}

QTEST_APPLESS_MAIN(SystemCalendarStoreTest)

#include "test_system_calendar_store.moc"
