#pragma once

#include <QString>

class ContentsTextHighlightRenderer final
{
public:
    static bool isHighlightTagAlias(const QString& elementName);
    static QString highlightOpenHtmlTag();
    static QString highlightCloseHtmlTag();
    static QString highlightStackTagName();
};
