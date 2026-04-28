#include "app/models/editor/format/ContentsPlainTextSourceMutator.hpp"

#include "app/models/file/note/WhatSonNoteBodyWebLinkSupport.hpp"

#include <algorithm>
#include <limits>

namespace
{
    namespace WebLinks = WhatSon::NoteBodyWebLinkSupport;

    int boundedQStringSize(const QString& text) noexcept
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::min<qsizetype>(text.size(), maxIntSize));
    }

    QString normalizeLineEndings(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
    }

    QString escapeHtmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }
} // namespace

ContentsPlainTextSourceMutator::ContentsPlainTextSourceMutator(QObject* parent)
    : QObject(parent)
{
}

ContentsPlainTextSourceMutator::~ContentsPlainTextSourceMutator() = default;

QString ContentsPlainTextSourceMutator::applyPlainTextReplacementToSource(
    const QString& sourceText,
    int sourceStart,
    int sourceEnd,
    const QString& replacementText) const
{
    const QString normalizedSourceText = normalizeLineEndings(sourceText);
    const QString normalizedReplacementText = normalizeLineEndings(replacementText);
    const int maximumSourceLength = boundedQStringSize(normalizedSourceText);
    const int boundedStart = std::clamp(std::min(sourceStart, sourceEnd), 0, maximumSourceLength);
    const int boundedEnd = std::clamp(std::max(sourceStart, sourceEnd), 0, maximumSourceLength);
    const QString nextSourceText = normalizedSourceText.left(boundedStart)
        + escapeHtmlText(normalizedReplacementText)
        + normalizedSourceText.mid(boundedEnd);

    const bool replacementContainsStandaloneWebLink =
        WebLinks::containsDetectableWebLink(normalizedReplacementText);
    const bool replacementCommitsPotentialWebLink = normalizedReplacementText.size() == 1
        && QStringLiteral(" \t\n.,;:!?)]}\"'").contains(normalizedReplacementText);
    if (!replacementContainsStandaloneWebLink && !replacementCommitsPotentialWebLink)
    {
        return nextSourceText;
    }

    return WebLinks::autoWrapDetectedWebLinks(nextSourceText);
}
