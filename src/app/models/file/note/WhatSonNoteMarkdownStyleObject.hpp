#pragma once

#include <QString>
#include <QStringList>

namespace WhatSon
{
class WhatSonNoteMarkdownStyleObject final
{
public:
    enum class Role
    {
        UnorderedListMarker,
        OrderedListMarker,
        HeadingMarker,
        HeadingBody,
        BlockquoteMarker,
        BlockquoteBody,
        InlineCode,
        CodeFence,
        CodeBody,
        LinkLiteral,
        HorizontalRule,
    };

    struct PromotionMatch final
    {
        bool matched = false;
        QStringList promotedInlineTags;
    };

    static QString wrapHtmlSpan(Role role, const QString& innerHtml, int variant = 0);
    static PromotionMatch promotionMatchForCss(const QString& cssDeclaration);
    static QString canonicalUnorderedListSourceMarker();
};
} // namespace WhatSon
