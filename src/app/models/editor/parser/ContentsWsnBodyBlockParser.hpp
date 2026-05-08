#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsWsnBodyBlockParser final
{
public:
    struct ParseResult final
    {
        QString correctedSourceText;
        QVariantList renderedDocumentBlocks;
        QVariantMap structuredParseVerification;
    };

    ContentsWsnBodyBlockParser();
    ~ContentsWsnBodyBlockParser();

    ParseResult parse(const QString& sourceText) const;
};
