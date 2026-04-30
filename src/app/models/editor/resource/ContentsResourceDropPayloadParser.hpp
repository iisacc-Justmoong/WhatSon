#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>

class ContentsResourceDropPayloadParser : public QObject
{
    Q_OBJECT

public:
    explicit ContentsResourceDropPayloadParser(QObject* parent = nullptr);
    ~ContentsResourceDropPayloadParser() override;

    Q_INVOKABLE QVariantList extractResourceDropUrls(const QVariant& drop) const;

private:
    static void appendResourceDropPayloadLines(const QString& rawText, QVariantList& urls);
    static void appendResourceDropMimePayload(QObject* dropObject, const QString& mimeType, QVariantList& urls);
};
