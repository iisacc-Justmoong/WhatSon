#pragma once

#include <QString>
#include <QVariantList>

class ContentsHtmlBlockRenderPipeline final
{
public:
    struct RenderResult final
    {
        QString correctedSourceText;
        QString documentHtml;
        QVariantList htmlTokens;
        QVariantList normalizedHtmlBlocks;
        bool htmlOverlayVisible = false;
        bool requiresLegacyDocumentComposition = false;
    };

    ContentsHtmlBlockRenderPipeline();
    ~ContentsHtmlBlockRenderPipeline();

    RenderResult renderEditorDocument(const QString& sourceText) const;
};
