#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsCalloutBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap lastParseVerification READ lastParseVerification NOTIFY lastParseVerificationChanged)

public:
    explicit ContentsCalloutBackend(QObject* parent = nullptr);
    ~ContentsCalloutBackend() override;

    QVariantMap lastParseVerification() const;

    Q_INVOKABLE QVariantList parseCallouts(const QString& sourceText);
    Q_INVOKABLE QVariantMap buildCalloutInsertionPayload(
        const QString& bodyText = QString()) const;
    Q_INVOKABLE QVariantMap detectCalloutEnterReplacement(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& insertedText) const;

signals:
    void lastParseVerificationChanged();
    void parseVerificationReported(const QVariantMap& verification);

private:
    void updateLastParseVerification(const QVariantMap& verification);

    QVariantMap m_lastParseVerification;
};
