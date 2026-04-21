#include "app/models/calendar/SystemCalendarStore.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>

namespace
{
    constexpr auto kCalendarScope = "calendar.system";
}

SystemCalendarStore::SystemCalendarStore(QObject* parent)
    : ISystemCalendarStore(parent)
{
    refreshFromSystem();
}

SystemCalendarStore::~SystemCalendarStore() = default;

QString SystemCalendarStore::localeName() const
{
    return m_localeName;
}

QString SystemCalendarStore::bcp47Name() const
{
    return m_bcp47Name;
}

QString SystemCalendarStore::languageName() const
{
    return m_languageName;
}

QString SystemCalendarStore::territoryName() const
{
    return m_territoryName;
}

QString SystemCalendarStore::timeZoneId() const
{
    return m_timeZoneId;
}

QStringList SystemCalendarStore::uiLanguages() const
{
    return m_uiLanguages;
}

QString SystemCalendarStore::shortDateFormat() const
{
    return m_shortDateFormat;
}

QString SystemCalendarStore::longDateFormat() const
{
    return m_longDateFormat;
}

QString SystemCalendarStore::shortDatePlaceholderText() const
{
    return m_shortDateFormat;
}

QString SystemCalendarStore::shortDateExample() const
{
    return formatDateForLocale(QDate::currentDate(), m_locale);
}

int SystemCalendarStore::firstDayOfWeek() const noexcept
{
    return m_firstDayOfWeek;
}

QVariantMap SystemCalendarStore::snapshot() const
{
    return QVariantMap{
        {QStringLiteral("localeName"), m_localeName},
        {QStringLiteral("bcp47Name"), m_bcp47Name},
        {QStringLiteral("languageName"), m_languageName},
        {QStringLiteral("territoryName"), m_territoryName},
        {QStringLiteral("timeZoneId"), m_timeZoneId},
        {QStringLiteral("uiLanguages"), m_uiLanguages},
        {QStringLiteral("shortDateFormat"), m_shortDateFormat},
        {QStringLiteral("longDateFormat"), m_longDateFormat},
        {QStringLiteral("shortDatePlaceholderText"), shortDatePlaceholderText()},
        {QStringLiteral("shortDateExample"), shortDateExample()},
        {QStringLiteral("firstDayOfWeek"), m_firstDayOfWeek}
    };
}

QString SystemCalendarStore::formatShortDate(const QString& value) const
{
    return formatNoteDate(value);
}

QString SystemCalendarStore::formatNoteDate(const QString& primaryValue, const QString& fallbackValue) const
{
    return formatNoteDateForLocale(primaryValue, fallbackValue, m_locale);
}

QString SystemCalendarStore::formatNoteDateForSystem(const QString& primaryValue, const QString& fallbackValue)
{
    return formatNoteDateForLocale(primaryValue, fallbackValue, QLocale::system());
}

void SystemCalendarStore::refreshFromSystem()
{
    const QLocale locale = QLocale::system();
    const QString localeName = locale.name().trimmed();
    const QString bcp47Name = locale.bcp47Name().trimmed();
    const QString languageName = QLocale::languageToString(locale.language()).trimmed();
    const QString territoryName = QLocale::territoryToString(locale.territory()).trimmed();
    const QString timeZoneId = QString::fromUtf8(QTimeZone::systemTimeZoneId()).trimmed();
    const QStringList uiLanguages = locale.uiLanguages();
    const QString shortDateFormat = normalizedDateFormat(locale.dateFormat(QLocale::ShortFormat));
    const QString longDateFormat = normalizedDateFormat(locale.dateFormat(QLocale::LongFormat));
    const int firstDayOfWeek = static_cast<int>(locale.firstDayOfWeek());

    const bool changed = m_localeName != localeName || m_bcp47Name != bcp47Name || m_languageName != languageName
        || m_territoryName != territoryName || m_timeZoneId != timeZoneId
        || m_uiLanguages != uiLanguages || m_shortDateFormat != shortDateFormat
        || m_longDateFormat != longDateFormat || m_firstDayOfWeek != firstDayOfWeek;

    m_locale = locale;
    m_localeName = localeName;
    m_bcp47Name = bcp47Name;
    m_languageName = languageName;
    m_territoryName = territoryName;
    m_timeZoneId = timeZoneId;
    m_uiLanguages = uiLanguages;
    m_shortDateFormat = shortDateFormat;
    m_longDateFormat = longDateFormat;
    m_firstDayOfWeek = firstDayOfWeek;

    WhatSon::Debug::traceSelf(
        this,
        QString::fromLatin1(kCalendarScope),
        QStringLiteral("refreshFromSystem"),
        QStringLiteral("locale=%1 bcp47=%2 territory=%3 timezone=%4 short=%5 long=%6")
        .arg(m_localeName, m_bcp47Name, m_territoryName, m_timeZoneId, m_shortDateFormat, m_longDateFormat));

    if (changed)
    {
        emit systemInfoChanged();
    }
}

void SystemCalendarStore::requestStoreHook(const QString& reason)
{
    const QString normalizedReason = reason.trimmed().isEmpty() ? QStringLiteral("manual") : reason.trimmed();
    emit storeHookRequested(normalizedReason);
}

QDate SystemCalendarStore::parseFlexibleDate(const QString& value)
{
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }

    const QList<QString> formats{
        QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
        QStringLiteral("yyyy-MM-dd hh:mm:ss"),
        QStringLiteral("yyyy/MM/dd hh:mm:ss"),
        QStringLiteral("yyyy-MM-dd")
    };

    for (const QString& format : formats)
    {
        const QDateTime dateTime = QDateTime::fromString(trimmed, format);
        if (dateTime.isValid())
        {
            return dateTime.date();
        }

        const QDate date = QDate::fromString(trimmed, format);
        if (date.isValid())
        {
            return date;
        }
    }

    const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
    if (isoDateTime.isValid())
    {
        return isoDateTime.date();
    }

    const QDateTime isoDateTimeWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
    if (isoDateTimeWithMs.isValid())
    {
        return isoDateTimeWithMs.date();
    }

    return {};
}

QString SystemCalendarStore::fallbackShortDateFormat()
{
    return QStringLiteral("Date");
}

QString SystemCalendarStore::normalizedDateFormat(QString value)
{
    value = value.trimmed();
    if (value.isEmpty())
    {
        return fallbackShortDateFormat();
    }
    return value;
}

QString SystemCalendarStore::formatDateForLocale(const QDate& date, const QLocale& locale)
{
    if (!date.isValid())
    {
        return {};
    }

    QString text = locale.toString(date, QLocale::ShortFormat).trimmed();
    if (!text.isEmpty())
    {
        return text;
    }

    const QString localeDateFormat = locale.dateFormat(QLocale::ShortFormat).trimmed();
    if (!localeDateFormat.isEmpty())
    {
        return date.toString(localeDateFormat);
    }

    return date.toString(Qt::TextDate);
}

QString SystemCalendarStore::formatNoteDateForLocale(
    const QString& primaryValue,
    const QString& fallbackValue,
    const QLocale& locale)
{
    QDate date = parseFlexibleDate(primaryValue);
    if (!date.isValid())
    {
        date = parseFlexibleDate(fallbackValue);
    }
    return formatDateForLocale(date, locale);
}
