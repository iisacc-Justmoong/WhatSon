#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class ISystemCalendarStore : public QObject
{
    Q_OBJECT

public:
    explicit ISystemCalendarStore(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~ISystemCalendarStore() override = default;

    virtual QString localeName() const = 0;
    virtual QString bcp47Name() const = 0;
    virtual QString languageName() const = 0;
    virtual QString territoryName() const = 0;
    virtual QString timeZoneId() const = 0;
    virtual QStringList uiLanguages() const = 0;
    virtual QString shortDateFormat() const = 0;
    virtual QString longDateFormat() const = 0;
    virtual QString shortDatePlaceholderText() const = 0;
    virtual QString shortDateExample() const = 0;
    virtual int firstDayOfWeek() const noexcept = 0;

    Q_INVOKABLE virtual QVariantMap snapshot() const = 0;
    Q_INVOKABLE virtual QString formatShortDate(const QString& value) const = 0;
    Q_INVOKABLE virtual QString formatNoteDate(
        const QString& primaryValue,
        const QString& fallbackValue = QString()) const = 0;

public slots:
    virtual void refreshFromSystem() = 0;
    virtual void requestStoreHook(const QString& reason = QString()) = 0;

signals:
    void systemInfoChanged();
    void storeHookRequested(QString reason);
};
