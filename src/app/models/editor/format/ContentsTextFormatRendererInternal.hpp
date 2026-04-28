#pragma once

#include <QString>
#include <QVariantList>

namespace WhatSon::ContentsTextFormatRendererInternal
{
    enum class LiteralRenderMode
    {
        MarkdownAware,
        SourceEditing,
    };

    QString renderInlineTaggedTextFragmentToHtml(
        const QString& sourceText,
        LiteralRenderMode renderMode = LiteralRenderMode::MarkdownAware);
    QString renderInlineStyleEditingSurfaceHtml(const QString& sourceText);
    QString renderMarkdownAwareTextToHtml(const QString& sourceText);
    QString applyPaperPaletteToHtml(QString html);
    QVariantList applyPaperPaletteToHtmlField(const QVariantList& entries, const QString& fieldName);
}
