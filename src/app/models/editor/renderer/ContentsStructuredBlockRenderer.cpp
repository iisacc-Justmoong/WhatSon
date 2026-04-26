#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"

#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/editor/tags/WhatSonStructuredTagLinter.hpp"

#include <QMetaObject>
#include <QPointer>
#include <QThreadPool>

#include <algorithm>

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

    bool mayContainResourceBlock(const QString& sourceText)
    {
        return containsIgnoreCase(sourceText, QStringLiteral("<resource"));
    }

    int boundedTextIndex(const QString& text, const int index)
    {
        return std::clamp(index, 0, static_cast<int>(text.size()));
    }

    QString resolvedDocumentBlockTypeName(const QString& normalizedTypeName)
    {
        return normalizedTypeName.isEmpty() ? QStringLiteral("text") : normalizedTypeName;
    }

    bool isAtomicDocumentBlockType(const QString& normalizedTypeName)
    {
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        return resolvedTypeName == QStringLiteral("resource")
            || resolvedTypeName == QStringLiteral("break");
    }

    QString minimapVisualKindForDocumentBlockType(const QString& normalizedTypeName)
    {
        return resolvedDocumentBlockTypeName(normalizedTypeName) == QStringLiteral("resource")
            ? QStringLiteral("block")
            : QStringLiteral("text");
    }

    int minimapRepresentativeCharCountForDocumentBlockType(const QString& normalizedTypeName)
    {
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        if (resolvedTypeName == QStringLiteral("resource"))
        {
            return 160;
        }
        if (resolvedTypeName == QStringLiteral("break"))
        {
            return 8;
        }
        return 0;
    }

    int logicalLineCountHintForPlainText(const QString& plainText)
    {
        return std::max(1, static_cast<int>(plainText.count(QLatin1Char('\n'))) + 1);
    }

    void applyDocumentBlockTraits(
        QVariantMap* payload,
        const QString& normalizedTypeName,
        const QString& plainText)
    {
        if (payload == nullptr)
        {
            return;
        }

        const bool atomicBlock = isAtomicDocumentBlockType(normalizedTypeName);
        payload->insert(QStringLiteral("plainText"), plainText);
        payload->insert(QStringLiteral("textEditable"), !atomicBlock);
        payload->insert(QStringLiteral("atomicBlock"), atomicBlock);
        payload->insert(QStringLiteral("gutterCollapsed"), atomicBlock);
        payload->insert(
            QStringLiteral("minimapVisualKind"),
            minimapVisualKindForDocumentBlockType(normalizedTypeName));
        payload->insert(
            QStringLiteral("minimapRepresentativeCharCount"),
            minimapRepresentativeCharCountForDocumentBlockType(normalizedTypeName));
        payload->insert(
            QStringLiteral("logicalLineCountHint"),
            logicalLineCountHintForPlainText(plainText));
    }

    QVariantMap documentBlockPayload(
        const QString& sourceText,
        const int sourceStart,
        const int sourceEnd,
        const QString& typeName = QString(),
        const QString& tagName = QString())
    {
        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedEnd = boundedTextIndex(sourceText, std::max(boundedStart, sourceEnd));
        const QString normalizedTypeName = typeName.trimmed().toCaseFolded();
        const QString normalizedTagName = tagName.trimmed().toCaseFolded();
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        const QString blockSourceText = sourceText.mid(boundedStart, boundedEnd - boundedStart);

        QVariantMap payload;
        payload.insert(
            QStringLiteral("type"),
            resolvedTypeName);
        payload.insert(QStringLiteral("sourceStart"), boundedStart);
        payload.insert(QStringLiteral("sourceEnd"), boundedEnd);
        payload.insert(QStringLiteral("sourceText"), blockSourceText);
        if (!normalizedTypeName.isEmpty())
        {
            payload.insert(QStringLiteral("explicitBlock"), true);
            payload.insert(
                QStringLiteral("tagName"),
                normalizedTagName.isEmpty() ? normalizedTypeName : normalizedTagName);
            if (WhatSon::NoteBodySemanticTagSupport::isRenderedTextBlockElement(normalizedTypeName))
            {
                payload.insert(QStringLiteral("semanticTagName"), normalizedTypeName);
            }
        }
        applyDocumentBlockTraits(
            &payload,
            resolvedTypeName,
            isAtomicDocumentBlockType(resolvedTypeName) ? QString() : blockSourceText);
        return payload;
    }

    QVariantMap defaultParseVerification(const QString& tagName)
    {
        return QVariantMap {
            {QStringLiteral("tagName"), tagName},
            {QStringLiteral("wellFormed"), true},
            {QStringLiteral("sourceLength"), 0},
            {QStringLiteral("issues"), QVariantList()}
        };
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
        StructuredRenderSnapshot snapshot;
        const ContentsWsnBodyBlockParser parser;
        const ContentsWsnBodyBlockParser::ParseResult parseResult = parser.parse(sourceText);
        snapshot.agendaParseVerification = parseResult.agendaParseVerification;
        snapshot.correctedSourceText = parseResult.correctedSourceText;
        snapshot.calloutParseVerification = parseResult.calloutParseVerification;
        snapshot.renderedAgendas = parseResult.renderedAgendas;
        snapshot.renderedCallouts = parseResult.renderedCallouts;
        snapshot.renderedDocumentBlocks =
            ContentsStructuredDocumentCollectionPolicy::normalizeInteractiveDocumentBlockEntries(
                parseResult.renderedDocumentBlocks,
                sourceText);
        snapshot.structuredParseVerification = parseResult.structuredParseVerification;
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
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("structuredBlockRenderer"), QStringLiteral("ctor"));
    const WhatSonStructuredTagLinter tagLinter;
    m_structuredParseVerification = tagLinter.buildStructuredVerification(
        m_agendaParseVerification,
        m_calloutParseVerification,
        QString());
}

ContentsStructuredBlockRenderer::~ContentsStructuredBlockRenderer()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("dtor"),
        QStringLiteral("renderPending=%1 sourceSummary=%2")
            .arg(m_renderPending)
            .arg(WhatSon::Debug::summarizeText(m_sourceText)));
}

QString ContentsStructuredBlockRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsStructuredBlockRenderer::setSourceText(const QString& sourceText)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("setSourceText"),
        QStringLiteral("changed=%1 %2").arg(m_sourceText != sourceText).arg(WhatSon::Debug::summarizeText(sourceText)));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("setBackgroundRefreshEnabled"),
        QStringLiteral("previous=%1 next=%2").arg(m_backgroundRefreshEnabled).arg(enabled));
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
    return !m_renderedDocumentBlocks.isEmpty();
}

bool ContentsStructuredBlockRenderer::hasNonResourceRenderedBlocks() const noexcept
{
    for (const QVariant& blockValue : m_renderedDocumentBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        const QString blockType = block.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
        if (blockType != QStringLiteral("resource"))
        {
            return true;
        }
    }
    return false;
}

void ContentsStructuredBlockRenderer::requestRenderRefresh()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("requestRenderRefresh"),
        QStringLiteral("renderPending=%1").arg(m_renderPending));
    refreshRenderedBlocks();
}

void ContentsStructuredBlockRenderer::refreshRenderedBlocks()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("refreshRenderedBlocks"),
        QStringLiteral("background=%1 shouldBackground=%2 %3")
            .arg(m_backgroundRefreshEnabled)
            .arg(shouldRenderInBackground())
            .arg(WhatSon::Debug::summarizeText(m_sourceText)));
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
            || mayContainResourceBlock(m_sourceText)
            || mayContainBreakBlock(m_sourceText));
}

void ContentsStructuredBlockRenderer::applyRenderResult(const RenderResult& result)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("applyRenderResult"),
        QStringLiteral("agendas=%1 callouts=%2 blocks=%3 correction=%4")
            .arg(result.renderedAgendas.size())
            .arg(result.renderedCallouts.size())
            .arg(result.renderedDocumentBlocks.size())
            .arg(!result.correctedSourceText.isEmpty() && result.correctedSourceText != result.sourceText));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("dispatchAsyncRender"),
        QStringLiteral("inFlight=%1 activeSequence=%2").arg(m_renderRequestInFlight).arg(m_activeRenderSequence));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("handleAsyncRenderFinished"),
        QStringLiteral("resultSequence=%1 activeSequence=%2 sourceMatches=%3")
            .arg(result.sequence)
            .arg(m_activeRenderSequence)
            .arg(result.sourceText == m_sourceText));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("publishPlaceholderDocumentBlocks"),
        QStringLiteral("sourceSummary=%1").arg(WhatSon::Debug::summarizeText(m_sourceText)));
    const QVariantList placeholderBlocks =
        ContentsStructuredDocumentCollectionPolicy::normalizeInteractiveDocumentBlockEntries(
            QVariantList{documentBlockPayload(m_sourceText, 0, m_sourceText.size())},
            m_sourceText);

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
