#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ICalendarBoardStore : public QObject
{
    Q_OBJECT

public:
    explicit ICalendarBoardStore(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~ICalendarBoardStore() override = default;

    Q_INVOKABLE virtual bool addEvent(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString()) = 0;
    Q_INVOKABLE virtual bool addTask(
        const QString& dateIso,
        const QString& timeText,
        const QString& title,
        const QString& detail = QString()) = 0;
    Q_INVOKABLE virtual QVariantList entriesForDate(const QString& dateIso) const = 0;
    Q_INVOKABLE virtual QVariantMap countsForDate(const QString& dateIso) const = 0;
    Q_INVOKABLE virtual bool removeEntry(const QString& entryId) = 0;
    Q_INVOKABLE virtual bool setTaskCompleted(const QString& entryId, bool completed) = 0;

signals:
    void entriesChanged();
    void entryAdded(QString entryId, QString entryType, QString dateIso, QString timeText);
    void entryRemoved(QString entryId);
    void entryUpdated(QString entryId);
};
