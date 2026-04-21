#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsWsnBodyBlockParser final
{
public:
    struct ParseResult final
    {
        QVariantMap agendaParseVerification;
        QString correctedSourceText;
        QVariantMap calloutParseVerification;
        QVariantList renderedAgendas;
        QVariantList renderedCallouts;
        QVariantList renderedDocumentBlocks;
        QVariantMap structuredParseVerification;
    };

    ContentsWsnBodyBlockParser();
    ~ContentsWsnBodyBlockParser();

    ParseResult parse(const QString& sourceText) const;
};
