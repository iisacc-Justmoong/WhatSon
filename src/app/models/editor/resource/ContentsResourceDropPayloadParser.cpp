#include "app/models/editor/resource/ContentsResourceDropPayloadParser.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QRegularExpression>
#include <QRegularExpression>
#include <QStringList>

using namespace WhatSon::Editor::DynamicObjectSupport;

ContentsResourceDropPayloadParser::ContentsResourceDropPayloadParser(QObject* parent)
    : QObject(parent)
{
}

ContentsResourceDropPayloadParser::~ContentsResourceDropPayloadParser() = default;

QVariantList ContentsResourceDropPayloadParser::extractResourceDropUrls(const QVariant& drop) const
{
    QVariantList urls;
    QObject* dropObject = qvariant_cast<QObject*>(drop);
    if (dropObject)
    {
        const QVariant urlsValue = propertyValue(dropObject, "urls");
        QVariantList explicitUrls = normalizeSequentialVariant(urlsValue);
        if (!explicitUrls.isEmpty())
        {
            return explicitUrls;
        }

        appendResourceDropPayloadLines(stringProperty(dropObject, "text"), urls);
        const QStringList mimeTypes = {
            QStringLiteral("text/uri-list"),
            QStringLiteral("text/plain"),
            QStringLiteral("public.file-url"),
            QStringLiteral("public.url"),
            QStringLiteral("text/x-moz-url")
        };
        for (const QString& mimeType : mimeTypes)
        {
            appendResourceDropMimePayload(dropObject, mimeType, urls);
        }
        return urls;
    }

    appendResourceDropPayloadLines(drop.toString(), urls);
    return urls;
}

void ContentsResourceDropPayloadParser::appendResourceDropPayloadLines(const QString& rawText, QVariantList& urls)
{
    const QString normalizedText = rawText.trimmed();
    if (normalizedText.isEmpty())
    {
        return;
    }

    const QStringList payloadLines = normalizedText.split(QRegularExpression(QStringLiteral("\\r?\\n|\\0")));
    for (const QString& payloadLine : payloadLines)
    {
        const QString line = payloadLine.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        urls.append(line);
    }
}

void ContentsResourceDropPayloadParser::appendResourceDropMimePayload(
    QObject* dropObject,
    const QString& mimeType,
    QVariantList& urls)
{
    if (!dropObject)
    {
        return;
    }

    appendResourceDropPayloadLines(
        invokeString(dropObject, "getDataAsString", { mimeType }),
        urls);
}
