#include "ContentsStructuredBlockRenderer.hpp"

#include "file/validator/WhatSonStructuredTagLinter.hpp"

#include <QRegularExpression>
#include <QVariantMap>

#include <algorithm>
#include <vector>

namespace
{
    struct DocumentBlockSpan final
    {
        int end = 0;
        QVariantMap payload;
        int start = 0;
    };

    QVariantMap defaultParseVerification(const QString& tagName)
    {
        return QVariantMap {
            {QStringLiteral("tagName"), tagName},
            {QStringLiteral("wellFormed"), true},
            {QStringLiteral("sourceLength"), 0},
            {QStringLiteral("issues"), QVariantList()}
        };
    }

    int boundedTextIndex(const QString& text, const int index)
    {
        return std::clamp(index, 0, static_cast<int>(text.size()));
    }

    QVariantMap textBlockPayload(const QString& sourceText, const int sourceStart, const int sourceEnd)
    {
        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedEnd = boundedTextIndex(sourceText, std::max(boundedStart, sourceEnd));

        QVariantMap payload;
        payload.insert(QStringLiteral("type"), QStringLiteral("text"));
        payload.insert(QStringLiteral("sourceStart"), boundedStart);
        payload.insert(QStringLiteral("sourceEnd"), boundedEnd);
        payload.insert(
            QStringLiteral("sourceText"),
            sourceText.mid(boundedStart, boundedEnd - boundedStart));
        return payload;
    }

    QVariantList buildRenderedDocumentBlocks(
        const QString& sourceText,
        const QVariantList& agendas,
        const QVariantList& callouts)
    {
        static const QRegularExpression breakTagPattern(
            QStringLiteral(R"((?:</break>|<\s*(?:break|hr)\b[^>]*?/?>|</\s*hr\s*>))"),
            QRegularExpression::CaseInsensitiveOption);

        std::vector<DocumentBlockSpan> spans;
        spans.reserve(
            static_cast<std::size_t>(agendas.size())
            + static_cast<std::size_t>(callouts.size())
            + 8U);

        const auto appendStructuredBlocks = [&](const QVariantList& sourceEntries, const QString& typeName) {
            for (const QVariant& entryValue : sourceEntries)
            {
                const QVariantMap entry = entryValue.toMap();
                const int sourceStart = entry.value(QStringLiteral("sourceStart")).toInt();
                const int sourceEnd = entry.value(QStringLiteral("sourceEnd")).toInt();
                if (sourceEnd <= sourceStart)
                {
                    continue;
                }

                QVariantMap payload = entry;
                payload.insert(QStringLiteral("type"), typeName);
                payload.insert(
                    QStringLiteral("sourceText"),
                    sourceText.mid(sourceStart, sourceEnd - sourceStart));
                spans.push_back(DocumentBlockSpan{sourceEnd, payload, sourceStart});
            }
        };

        appendStructuredBlocks(agendas, QStringLiteral("agenda"));
        appendStructuredBlocks(callouts, QStringLiteral("callout"));

        QRegularExpressionMatchIterator breakIterator = breakTagPattern.globalMatch(sourceText);
        while (breakIterator.hasNext())
        {
            const QRegularExpressionMatch match = breakIterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            const int sourceStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int sourceEnd = std::max(sourceStart, static_cast<int>(match.capturedEnd(0)));
            QVariantMap payload;
            payload.insert(QStringLiteral("type"), QStringLiteral("break"));
            payload.insert(QStringLiteral("sourceStart"), sourceStart);
            payload.insert(QStringLiteral("sourceEnd"), sourceEnd);
            payload.insert(QStringLiteral("sourceText"), sourceText.mid(sourceStart, sourceEnd - sourceStart));
            spans.push_back(DocumentBlockSpan{sourceEnd, payload, sourceStart});
        }

        std::sort(
            spans.begin(),
            spans.end(),
            [](const DocumentBlockSpan& lhs, const DocumentBlockSpan& rhs) {
                if (lhs.start != rhs.start)
                {
                    return lhs.start < rhs.start;
                }
                return lhs.end < rhs.end;
            });

        QVariantList renderedBlocks;
        int cursor = 0;
        for (const DocumentBlockSpan& span : spans)
        {
            const int boundedStart = boundedTextIndex(sourceText, span.start);
            const int boundedEnd = boundedTextIndex(sourceText, std::max(span.start, span.end));
            if (boundedStart < cursor)
            {
                continue;
            }

            if (cursor < boundedStart)
            {
                renderedBlocks.push_back(textBlockPayload(sourceText, cursor, boundedStart));
            }

            renderedBlocks.push_back(span.payload);
            cursor = boundedEnd;
        }

        if (cursor < sourceText.size())
        {
            renderedBlocks.push_back(textBlockPayload(sourceText, cursor, sourceText.size()));
        }

        if (renderedBlocks.isEmpty())
        {
            renderedBlocks.push_back(textBlockPayload(sourceText, 0, sourceText.size()));
        }

        return renderedBlocks;
    }
} // namespace

ContentsStructuredBlockRenderer::ContentsStructuredBlockRenderer(QObject* parent)
    : QObject(parent)
    , m_agendaParseVerification(defaultParseVerification(QStringLiteral("agenda")))
    , m_calloutParseVerification(defaultParseVerification(QStringLiteral("callout")))
{
    connect(
        &m_agendaBackend,
        &ContentsAgendaBackend::parseVerificationReported,
        this,
        &ContentsStructuredBlockRenderer::handleAgendaParseVerificationReported);
    connect(
        &m_calloutBackend,
        &ContentsCalloutBackend::parseVerificationReported,
        this,
        &ContentsStructuredBlockRenderer::handleCalloutParseVerificationReported);
    refreshStructuredParseVerification();
}

ContentsStructuredBlockRenderer::~ContentsStructuredBlockRenderer() = default;

QString ContentsStructuredBlockRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsStructuredBlockRenderer::setSourceText(const QString& sourceText)
{
    if (m_sourceText == sourceText)
    {
        return;
    }

    const bool previousCorrectionSuggested = correctionSuggested();
    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedBlocks();
    if (previousCorrectionSuggested != correctionSuggested())
    {
        emit correctionSuggestedChanged();
    }
}

QVariantList ContentsStructuredBlockRenderer::renderedAgendas() const
{
    return m_renderedAgendas;
}

QVariantList ContentsStructuredBlockRenderer::renderedCallouts() const
{
    return m_renderedCallouts;
}

QVariantList ContentsStructuredBlockRenderer::renderedDocumentBlocks() const
{
    return m_renderedDocumentBlocks;
}

QVariantMap ContentsStructuredBlockRenderer::agendaParseVerification() const
{
    return m_agendaParseVerification;
}

QVariantMap ContentsStructuredBlockRenderer::calloutParseVerification() const
{
    return m_calloutParseVerification;
}

QVariantMap ContentsStructuredBlockRenderer::structuredParseVerification() const
{
    return m_structuredParseVerification;
}

QString ContentsStructuredBlockRenderer::correctedSourceText() const
{
    return m_correctedSourceText;
}

bool ContentsStructuredBlockRenderer::correctionSuggested() const noexcept
{
    return !m_correctedSourceText.isEmpty() && m_correctedSourceText != m_sourceText;
}

int ContentsStructuredBlockRenderer::agendaCount() const noexcept
{
    return m_renderedAgendas.size();
}

int ContentsStructuredBlockRenderer::calloutCount() const noexcept
{
    return m_renderedCallouts.size();
}

bool ContentsStructuredBlockRenderer::hasRenderedBlocks() const noexcept
{
    for (const QVariant& blockValue : m_renderedDocumentBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        if (block.value(QStringLiteral("type")).toString() != QStringLiteral("text"))
        {
            return true;
        }
    }
    return false;
}

void ContentsStructuredBlockRenderer::requestRenderRefresh()
{
    refreshRenderedBlocks();
}

void ContentsStructuredBlockRenderer::refreshRenderedBlocks()
{
    const QVariantList nextRenderedAgendas = m_agendaBackend.parseAgendas(m_sourceText);
    const QVariantList nextRenderedCallouts = m_calloutBackend.parseCallouts(m_sourceText);
    const QVariantList nextRenderedDocumentBlocks = buildRenderedDocumentBlocks(
        m_sourceText,
        nextRenderedAgendas,
        nextRenderedCallouts);
    if (m_renderedAgendas == nextRenderedAgendas
        && m_renderedCallouts == nextRenderedCallouts
        && m_renderedDocumentBlocks == nextRenderedDocumentBlocks)
    {
        return;
    }

    m_renderedAgendas = nextRenderedAgendas;
    m_renderedCallouts = nextRenderedCallouts;
    m_renderedDocumentBlocks = nextRenderedDocumentBlocks;
    emit renderedBlocksChanged();
}

void ContentsStructuredBlockRenderer::refreshStructuredParseVerification()
{
    const WhatSonStructuredTagLinter tagLinter;
    const QString nextCorrectedSourceText = tagLinter.normalizeStructuredSourceText(m_sourceText);
    if (m_correctedSourceText != nextCorrectedSourceText)
    {
        m_correctedSourceText = nextCorrectedSourceText;
        emit correctedSourceTextChanged();
    }

    const QVariantMap nextStructuredVerification = tagLinter.buildStructuredVerification(
        m_agendaParseVerification,
        m_calloutParseVerification,
        m_sourceText);
    if (m_structuredParseVerification != nextStructuredVerification)
    {
        m_structuredParseVerification = nextStructuredVerification;
        emit structuredParseVerificationChanged();
        emit structuredParseVerificationReported(m_structuredParseVerification);
    }

    if (correctionSuggested())
    {
        if (m_lastCorrectionSuggestionSourceText != m_sourceText
            || m_lastCorrectionSuggestionCorrectedText != m_correctedSourceText)
        {
            m_lastCorrectionSuggestionSourceText = m_sourceText;
            m_lastCorrectionSuggestionCorrectedText = m_correctedSourceText;
            emit structuredCorrectionSuggested(
                m_sourceText,
                m_correctedSourceText,
                m_structuredParseVerification);
        }
    }
    else
    {
        m_lastCorrectionSuggestionSourceText.clear();
        m_lastCorrectionSuggestionCorrectedText.clear();
    }
}

void ContentsStructuredBlockRenderer::handleAgendaParseVerificationReported(const QVariantMap& verification)
{
    if (m_agendaParseVerification != verification)
    {
        m_agendaParseVerification = verification;
        emit agendaParseVerificationChanged();
    }
    emit agendaParseVerificationReported(verification);
    refreshStructuredParseVerification();
}

void ContentsStructuredBlockRenderer::handleCalloutParseVerificationReported(const QVariantMap& verification)
{
    if (m_calloutParseVerification != verification)
    {
        m_calloutParseVerification = verification;
        emit calloutParseVerificationChanged();
    }
    emit calloutParseVerificationReported(verification);
    refreshStructuredParseVerification();
}
