#include "ContentsStructuredBlockRenderer.hpp"

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"
#include "file/validator/WhatSonStructuredTagLinter.hpp"

#include <QMetaObject>
#include <QPointer>
#include <QRegularExpression>
#include <QThreadPool>
#include <QVariantMap>

#include <algorithm>
#include <vector>

namespace
{
    bool containsIgnoreCase(const QString& text, const QString& token)
    {
        return text.indexOf(token, 0, Qt::CaseInsensitive) >= 0;
    }

    bool mayContainAgendaBlock(const QString& sourceText)
    {
        return containsIgnoreCase(sourceText, QStringLiteral("<agenda"));
    }

    bool mayContainCalloutBlock(const QString& sourceText)
    {
        return containsIgnoreCase(sourceText, QStringLiteral("<callout"));
    }

    bool mayContainBreakBlock(const QString& sourceText)
    {
        return containsIgnoreCase(sourceText, QStringLiteral("</break"))
            || containsIgnoreCase(sourceText, QStringLiteral("<break"))
            || containsIgnoreCase(sourceText, QStringLiteral("<hr"))
            || containsIgnoreCase(sourceText, QStringLiteral("</hr"));
    }

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
        const QVariantList& callouts,
        const bool includeBreakBlocks)
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

        if (includeBreakBlocks)
        {
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

    struct StructuredRenderSnapshot final
    {
        QVariantMap agendaParseVerification;
        QString correctedSourceText;
        QVariantMap calloutParseVerification;
        QVariantList renderedAgendas;
        QVariantList renderedCallouts;
        QVariantList renderedDocumentBlocks;
        QVariantMap structuredParseVerification;
    };

    StructuredRenderSnapshot buildRenderSnapshot(const QString& sourceText)
    {
        const bool agendaBlocksPossible = mayContainAgendaBlock(sourceText);
        const bool calloutBlocksPossible = mayContainCalloutBlock(sourceText);
        const bool breakBlocksPossible = mayContainBreakBlock(sourceText);
        const WhatSonStructuredTagLinter tagLinter;
        ContentsAgendaBackend agendaBackend;
        ContentsCalloutBackend calloutBackend;

        StructuredRenderSnapshot snapshot;

        if (agendaBlocksPossible)
        {
            snapshot.renderedAgendas = agendaBackend.parseAgendas(sourceText);
            snapshot.agendaParseVerification = agendaBackend.lastParseVerification();
        }
        else
        {
            snapshot.agendaParseVerification = tagLinter.buildAgendaVerification(sourceText, 0, 0, 0);
        }

        if (calloutBlocksPossible)
        {
            snapshot.renderedCallouts = calloutBackend.parseCallouts(sourceText);
            snapshot.calloutParseVerification = calloutBackend.lastParseVerification();
        }
        else
        {
            snapshot.calloutParseVerification = tagLinter.buildCalloutVerification(sourceText, 0);
        }

        snapshot.correctedSourceText = tagLinter.normalizeStructuredSourceText(sourceText);
        snapshot.structuredParseVerification = tagLinter.buildStructuredVerification(
            snapshot.agendaParseVerification,
            snapshot.calloutParseVerification,
            sourceText);
        snapshot.renderedDocumentBlocks = buildRenderedDocumentBlocks(
            sourceText,
            snapshot.renderedAgendas,
            snapshot.renderedCallouts,
            breakBlocksPossible);
        return snapshot;
    }
} // namespace

struct ContentsStructuredBlockRenderer::RenderResult final
{
    QVariantMap agendaParseVerification;
    QString correctedSourceText;
    QVariantMap calloutParseVerification;
    QVariantList renderedAgendas;
    QVariantList renderedCallouts;
    QVariantList renderedDocumentBlocks;
    quint64 sequence = 0;
    QString sourceText;
    QVariantMap structuredParseVerification;
};

ContentsStructuredBlockRenderer::ContentsStructuredBlockRenderer(QObject* parent)
    : QObject(parent)
    , m_agendaParseVerification(defaultParseVerification(QStringLiteral("agenda")))
    , m_calloutParseVerification(defaultParseVerification(QStringLiteral("callout")))
{
    const WhatSonStructuredTagLinter tagLinter;
    m_structuredParseVerification = tagLinter.buildStructuredVerification(
        m_agendaParseVerification,
        m_calloutParseVerification,
        QString());
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

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedBlocks();
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

bool ContentsStructuredBlockRenderer::backgroundRefreshEnabled() const noexcept
{
    return m_backgroundRefreshEnabled;
}

void ContentsStructuredBlockRenderer::setBackgroundRefreshEnabled(const bool enabled)
{
    if (m_backgroundRefreshEnabled == enabled)
    {
        return;
    }

    m_backgroundRefreshEnabled = enabled;
    emit backgroundRefreshEnabledChanged();

    if (!m_backgroundRefreshEnabled && m_renderPending)
    {
        m_activeRenderSequence = 0;
        refreshRenderedBlocks();
    }
}

bool ContentsStructuredBlockRenderer::renderPending() const noexcept
{
    return m_renderPending;
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
    if (shouldRenderInBackground())
    {
        updateRenderPending(true);
        publishPlaceholderDocumentBlocks();
        dispatchAsyncRender();
        return;
    }

    updateRenderPending(false);
    m_activeRenderSequence = 0;

    const StructuredRenderSnapshot snapshot = buildRenderSnapshot(m_sourceText);
    RenderResult result;
    result.agendaParseVerification = snapshot.agendaParseVerification;
    result.correctedSourceText = snapshot.correctedSourceText;
    result.calloutParseVerification = snapshot.calloutParseVerification;
    result.renderedAgendas = snapshot.renderedAgendas;
    result.renderedCallouts = snapshot.renderedCallouts;
    result.renderedDocumentBlocks = snapshot.renderedDocumentBlocks;
    result.sourceText = m_sourceText;
    result.structuredParseVerification = snapshot.structuredParseVerification;
    applyRenderResult(result);
}

void ContentsStructuredBlockRenderer::updateAgendaParseVerification(const QVariantMap& verification)
{
    if (m_agendaParseVerification != verification)
    {
        m_agendaParseVerification = verification;
        emit agendaParseVerificationChanged();
    }
    emit agendaParseVerificationReported(verification);
}

void ContentsStructuredBlockRenderer::updateCalloutParseVerification(const QVariantMap& verification)
{
    if (m_calloutParseVerification != verification)
    {
        m_calloutParseVerification = verification;
        emit calloutParseVerificationChanged();
    }
    emit calloutParseVerificationReported(verification);
}

void ContentsStructuredBlockRenderer::updateStructuredParseVerification(const QVariantMap& verification)
{
    if (m_structuredParseVerification != verification)
    {
        m_structuredParseVerification = verification;
        emit structuredParseVerificationChanged();
    }
    emit structuredParseVerificationReported(verification);
}

void ContentsStructuredBlockRenderer::updateCorrectedSourceText(const QString& correctedSourceText)
{
    if (m_correctedSourceText == correctedSourceText)
    {
        return;
    }

    m_correctedSourceText = correctedSourceText;
    emit correctedSourceTextChanged();
}

void ContentsStructuredBlockRenderer::updateRenderPending(const bool pending)
{
    if (m_renderPending == pending)
    {
        return;
    }

    m_renderPending = pending;
    emit renderPendingChanged();
}

bool ContentsStructuredBlockRenderer::shouldRenderInBackground() const noexcept
{
    return m_backgroundRefreshEnabled
        && (mayContainAgendaBlock(m_sourceText)
            || mayContainCalloutBlock(m_sourceText)
            || mayContainBreakBlock(m_sourceText));
}

void ContentsStructuredBlockRenderer::applyRenderResult(const RenderResult& result)
{
    const bool previousCorrectionSuggested = correctionSuggested();

    updateAgendaParseVerification(result.agendaParseVerification);
    updateCalloutParseVerification(result.calloutParseVerification);
    updateCorrectedSourceText(result.correctedSourceText);
    updateStructuredParseVerification(result.structuredParseVerification);

    const bool renderPayloadChanged =
        m_renderedAgendas != result.renderedAgendas
        || m_renderedCallouts != result.renderedCallouts
        || m_renderedDocumentBlocks != result.renderedDocumentBlocks;
    if (renderPayloadChanged)
    {
        m_renderedAgendas = result.renderedAgendas;
        m_renderedCallouts = result.renderedCallouts;
        m_renderedDocumentBlocks = result.renderedDocumentBlocks;
        emit renderedBlocksChanged();
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

    if (previousCorrectionSuggested != correctionSuggested())
    {
        emit correctionSuggestedChanged();
    }
}

void ContentsStructuredBlockRenderer::dispatchAsyncRender()
{
    if (m_renderRequestInFlight)
    {
        return;
    }

    const QString sourceText = m_sourceText;
    const quint64 sequence = m_nextRenderSequence++;
    m_activeRenderSequence = sequence;
    m_renderRequestInFlight = true;

    QPointer<ContentsStructuredBlockRenderer> rendererGuard(this);
    QThreadPool::globalInstance()->start([rendererGuard, sequence, sourceText]()
    {
        const StructuredRenderSnapshot snapshot = buildRenderSnapshot(sourceText);
        RenderResult result;
        result.agendaParseVerification = snapshot.agendaParseVerification;
        result.correctedSourceText = snapshot.correctedSourceText;
        result.calloutParseVerification = snapshot.calloutParseVerification;
        result.renderedAgendas = snapshot.renderedAgendas;
        result.renderedCallouts = snapshot.renderedCallouts;
        result.renderedDocumentBlocks = snapshot.renderedDocumentBlocks;
        result.sequence = sequence;
        result.sourceText = sourceText;
        result.structuredParseVerification = snapshot.structuredParseVerification;
        if (rendererGuard != nullptr)
        {
            QMetaObject::invokeMethod(
                rendererGuard,
                [rendererGuard, result]()
                {
                    if (rendererGuard != nullptr)
                    {
                        rendererGuard->handleAsyncRenderFinished(result);
                    }
                },
                Qt::QueuedConnection);
        }
    });
}

void ContentsStructuredBlockRenderer::handleAsyncRenderFinished(const RenderResult& result)
{
    const bool sequenceMatches = result.sequence != 0 && result.sequence == m_activeRenderSequence;
    m_renderRequestInFlight = false;
    if (sequenceMatches)
    {
        m_activeRenderSequence = 0;
    }

    if (!sequenceMatches || result.sourceText != m_sourceText)
    {
        if (shouldRenderInBackground())
        {
            dispatchAsyncRender();
            return;
        }
        if (m_renderPending)
        {
            refreshRenderedBlocks();
        }
        return;
    }

    updateRenderPending(false);
    applyRenderResult(result);
}

void ContentsStructuredBlockRenderer::publishPlaceholderDocumentBlocks()
{
    const QVariantList placeholderBlocks = QVariantList {
        textBlockPayload(m_sourceText, 0, m_sourceText.size())
    };

    const bool previousCorrectionSuggested = correctionSuggested();
    updateCorrectedSourceText(QString());

    const bool renderPayloadChanged =
        !m_renderedAgendas.isEmpty()
        || !m_renderedCallouts.isEmpty()
        || m_renderedDocumentBlocks != placeholderBlocks;
    if (renderPayloadChanged)
    {
        m_renderedAgendas.clear();
        m_renderedCallouts.clear();
        m_renderedDocumentBlocks = placeholderBlocks;
        emit renderedBlocksChanged();
    }

    m_lastCorrectionSuggestionSourceText.clear();
    m_lastCorrectionSuggestionCorrectedText.clear();
    if (previousCorrectionSuggested != correctionSuggested())
    {
        emit correctionSuggestedChanged();
    }
}
