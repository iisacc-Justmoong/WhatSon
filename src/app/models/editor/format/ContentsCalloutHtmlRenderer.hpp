#pragma once

#include <QString>

namespace WhatSon::ContentsCalloutHtmlRenderer
{
    struct CalloutSourceSpan final
    {
        int innerEnd = -1;
        QString innerSource;
        int innerStart = -1;
        bool valid = false;
    };

    bool containsCalloutTag(const QString& sourceText);
    CalloutSourceSpan singleCalloutSpan(const QString& sourceText);
    QString renderCalloutBlockHtml(const QString& innerHtml);
} // namespace WhatSon::ContentsCalloutHtmlRenderer
