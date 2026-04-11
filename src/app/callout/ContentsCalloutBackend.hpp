#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsCalloutBackend : public QObject
{
    Q_OBJECT

public:
    explicit ContentsCalloutBackend(QObject* parent = nullptr);
    ~ContentsCalloutBackend() override;

    Q_INVOKABLE QVariantList parseCallouts(const QString& sourceText) const;
    Q_INVOKABLE QVariantMap buildCalloutInsertionPayload(
        const QString& bodyText = QString()) const;
    Q_INVOKABLE QVariantMap detectCalloutEnterReplacement(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& insertedText) const;
};
