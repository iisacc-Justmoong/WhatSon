#pragma once

#include <QLocale>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QDate;

class SystemCalendarStore final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString localeName READ localeName NOTIFY systemInfoChanged)
    Q_PROPERTY(QString bcp47Name READ bcp47Name NOTIFY systemInfoChanged)
    Q_PROPERTY(QString languageName READ languageName NOTIFY systemInfoChanged)
    Q_PROPERTY(QString territoryName READ territoryName NOTIFY systemInfoChanged)
    Q_PROPERTY(QString timeZoneId READ timeZoneId NOTIFY systemInfoChanged)
    Q_PROPERTY(QStringList uiLanguages READ uiLanguages NOTIFY systemInfoChanged)
    Q_PROPERTY(QString shortDateFormat READ shortDateFormat NOTIFY systemInfoChanged)
    Q_PROPERTY(QString longDateFormat READ longDateFormat NOTIFY systemInfoChanged)
    Q_PROPERTY(QString shortDatePlaceholderText READ shortDatePlaceholderText NOTIFY systemInfoChanged)
    Q_PROPERTY(QString shortDateExample READ shortDateExample NOTIFY systemInfoChanged)
    Q_PROPERTY(int firstDayOfWeek READ firstDayOfWeek NOTIFY systemInfoChanged)

public:
    explicit SystemCalendarStore(QObject* parent = nullptr);
    ~SystemCalendarStore() override;

    QString localeName() const;
    QString bcp47Name() const;
    QString languageName() const;
    QString territoryName() const;
    QString timeZoneId() const;
    QStringList uiLanguages() const;
    QString shortDateFormat() const;
    QString longDateFormat() const;
    QString shortDatePlaceholderText() const;
    QString shortDateExample() const;
    int firstDayOfWeek() const noexcept;

    Q_INVOKABLE QVariantMap snapshot() const;
    Q_INVOKABLE QString formatShortDate(const QString& value) const;
    Q_INVOKABLE QString formatNoteDate(const QString& primaryValue, const QString& fallbackValue = QString()) const;

    static QString formatNoteDateForSystem(const QString& primaryValue, const QString& fallbackValue = QString());

public
    slots  :



    void refreshFromSystem();
    void requestStoreHook(const QString& reason = QString());

    signals  :



    void systemInfoChanged();
    void storeHookRequested(QString reason);

private:
    static QDate parseFlexibleDate(const QString& value);
    static QString fallbackShortDateFormat();
    static QString normalizedDateFormat(QString value);
    static QString formatDateForLocale(const QDate& date, const QLocale& locale);
    static QString formatNoteDateForLocale(
        const QString& primaryValue,
        const QString& fallbackValue,
        const QLocale& locale);

    QLocale m_locale;
    QString m_localeName;
    QString m_bcp47Name;
    QString m_languageName;
    QString m_territoryName;
    QString m_timeZoneId;
    QStringList m_uiLanguages;
    QString m_shortDateFormat;
    QString m_longDateFormat;
    int m_firstDayOfWeek = 1;
};
