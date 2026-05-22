#pragma once

#include <QString>
#include <QStringList>

namespace WhatSon::EditorComponent
{
    struct StyleToken final
    {
        bool valid = false;
        QString name;
        int pixelSize = 0;
        int weight = 0;
        QString styleName;
        int lineHeight = 0;
        QString color;
    };

    struct StyleSourceBaseline final
    {
        bool italic = false;
        int weight = 0;
        QString background;
    };

    class Style final
    {
    public:
        static QString canonicalName();
        static QString openingToken();
        static QString closingToken();
        static QString defaultEditorFontFamily();
        static QStringList styleAttributeValues();
        static QString normalizedStyleAttributeValue(QString value);
        static QString openingTokenForStyleAttributeValue(QString value);
        static StyleToken lvrsTextStyleTokenFromName(QString tokenName);
        static QString bodyEditorCssDeclaration();
        static QString attributeValueFromRawToken(const QString& rawTagText, const QString& attributeName);
        static QString cssDeclarationFromRawToken(const QString& rawTagText);
        static QString openingHtmlFromRawToken(const QString& rawTagText);
        static QString closingHtml();
        static QString markerContentHtml(QString markerBody);
        static int weightValue(const QString& value);
        static bool spanMatchesOpeningToken(const QString& spanOpeningTag, const QString& openingToken);
        static StyleSourceBaseline sourceBaselineFromOpeningToken(const QString& openingToken);
    };
} // namespace WhatSon::EditorComponent
