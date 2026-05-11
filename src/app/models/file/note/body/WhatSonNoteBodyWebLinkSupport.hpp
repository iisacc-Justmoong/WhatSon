#pragma once

#include <QString>

namespace WhatSon::NoteBodyWebLinkSupport
{
    bool isWebLinkTagName(const QString& elementName);
    bool containsDetectableWebLink(const QString& text);
    QString canonicalStartTag(const QString& href);
    QString canonicalStartTagFromRawToken(const QString& rawTagText);
    QString openingHtmlForHref(const QString& href);
    QString openingHtmlFromRawToken(const QString& rawTagText);
    QString activationUrlForHref(const QString& href);
    QString autoWrapDetectedWebLinks(const QString& sourceText);
} // namespace WhatSon::NoteBodyWebLinkSupport
