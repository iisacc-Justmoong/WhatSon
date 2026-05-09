#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"

#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QPointer>
#include <QThreadPool>

#include <algorithm>

namespace
{
    constexpr int kBackgroundRenderSourceLengthThreshold = 2048;
    constexpr int kLargePlainNativeSurfaceLengthThreshold = 32 * 1024;

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

    bool canUseLargePlainNativeSurfaceRenderFastPath(const QString& sourceText)
    {
        return sourceText.size() >= kLargePlainNativeSurfaceLengthThreshold
            && !sourceText.contains(QLatin1Char('<'));
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

    QVariantMap largePlainNativeSurfaceBlockPayload(const QString& sourceText)
    {
        QVariantMap payload;
        payload.insert(QStringLiteral("atomicBlock"), false);
        payload.insert(QStringLiteral("flattenedInteractiveChildCount"), 1);
        payload.insert(QStringLiteral("flattenedInteractiveGroup"), true);
        payload.insert(QStringLiteral("focusSourceOffset"), 0);
        payload.insert(QStringLiteral("groupedBlocks"), QVariantList{});
        payload.insert(QStringLiteral("logicalLineCountHint"), 1);
        payload.insert(QStringLiteral("minimapRepresentativeCharCount"), 0);
        payload.insert(QStringLiteral("minimapVisualKind"), QStringLiteral("text"));
        payload.insert(QStringLiteral("plainText"), QString());
        payload.insert(QStringLiteral("sourceEnd"), sourceText.size());
        payload.insert(QStringLiteral("sourceStart"), 0);
        payload.insert(QStringLiteral("sourceText"), QString());
        payload.insert(QStringLiteral("textEditable"), true);
        payload.insert(QStringLiteral("type"), QStringLiteral("text-group"));
        return payload;
    }

    struct StructuredRenderSnapshot final
    {
        QString correctedSourceText;
        QVariantList renderedDocumentBlocks;
        QVariantMap structuredParseVerification;
    };

    StructuredRenderSnapshot buildRenderSnapshot(const QString& sourceText)
    {
        StructuredRenderSnapshot snapshot;
        const ContentsWsnBodyBlockParser parser;
        const ContentsWsnBodyBlockParser::ParseResult parseResult = parser.parse(sourceText);
        snapshot.correctedSourceText = parseResult.correctedSourceText;
        snapshot.renderedDocumentBlocks =
            ContentsStructuredDocumentCollectionPolicy::normalizeInteractiveDocumentBlockEntries(
                parseResult.renderedDocumentBlocks,
                sourceText);
        snapshot.structuredParseVerification = parseResult.structuredParseVerification;
        return snapshot;
    }

    QVariantMap buildRenderProfile(
        const QString& sourceText,
        const StructuredRenderSnapshot& snapshot,
        const QString& mode,
        const qint64 elapsedMs)
    {
        return QVariantMap {
            {QStringLiteral("blockCount"), snapshot.renderedDocumentBlocks.size()},
            {QStringLiteral("corrected"), !snapshot.correctedSourceText.isEmpty() && snapshot.correctedSourceText != sourceText},
            {QStringLiteral("elapsedMs"), elapsedMs},
            {QStringLiteral("mode"), mode},
            {QStringLiteral("sourceLength"), sourceText.size()}
        };
    }

    struct TimedRenderSnapshot final
{
    QVariantMap renderProfile;
    StructuredRenderSnapshot snapshot;
};

TimedRenderSnapshot buildTimedRenderSnapshot(const QString& sourceText, const QString& mode)
{
    QElapsedTimer timer;
    timer.start();

    TimedRenderSnapshot result;
    result.snapshot = buildRenderSnapshot(sourceText);
    result.renderProfile = buildRenderProfile(sourceText, result.snapshot, mode, timer.elapsed());
    return result;
}

} // namespace

struct ContentsStructuredBlockRenderer::RenderResult final
{
    QString correctedSourceText;
    QVariantList renderedDocumentBlocks;
    QVariantMap renderProfile;
    quint64 sequence = 0;
    QString sourceText;
    QVariantMap structuredParseVerification;
};

ContentsStructuredBlockRenderer::ContentsStructuredBlockRenderer(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("structuredBlockRenderer"), QStringLiteral("ctor"));
    const TimedRenderSnapshot initialSnapshot = buildTimedRenderSnapshot(QString(), QStringLiteral("initial"));
    m_correctedSourceText = initialSnapshot.snapshot.correctedSourceText;
    m_renderedDocumentBlocks = initialSnapshot.snapshot.renderedDocumentBlocks;
    m_structuredParseVerification = initialSnapshot.snapshot.structuredParseVerification;
    m_lastRenderProfile = initialSnapshot.renderProfile;
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
        if (m_renderedDocumentBlocks.isEmpty())
        {
            refreshRenderedBlocks();
        }
        return;
    }

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedBlocks();
}

QVariantList ContentsStructuredBlockRenderer::renderedDocumentBlocks() const
{
    return m_renderedDocumentBlocks;
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

QVariantMap ContentsStructuredBlockRenderer::lastRenderProfile() const
{
    return m_lastRenderProfile;
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
    if (canUseLargePlainNativeSurfaceRenderFastPath(m_sourceText))
    {
        updateRenderPending(false);
        m_activeRenderSequence = 0;

        RenderResult result;
        result.renderedDocumentBlocks = QVariantList{largePlainNativeSurfaceBlockPayload(m_sourceText)};
        result.renderProfile = QVariantMap{
            {QStringLiteral("blockCount"), result.renderedDocumentBlocks.size()},
            {QStringLiteral("corrected"), false},
            {QStringLiteral("elapsedMs"), 0},
            {QStringLiteral("mode"), QStringLiteral("large-plain-native")},
            {QStringLiteral("sourceLength"), m_sourceText.size()}
        };
        result.sourceText = m_sourceText;
        applyRenderResult(result);
        return;
    }

    if (shouldRenderInBackground())
    {
        updateRenderPending(true);
        publishPlaceholderDocumentBlocks();
        dispatchAsyncRender();
        return;
    }

    updateRenderPending(false);
    m_activeRenderSequence = 0;

    const TimedRenderSnapshot timedSnapshot = buildTimedRenderSnapshot(m_sourceText, QStringLiteral("sync"));
    RenderResult result;
    result.correctedSourceText = timedSnapshot.snapshot.correctedSourceText;
    result.renderedDocumentBlocks = timedSnapshot.snapshot.renderedDocumentBlocks;
    result.renderProfile = timedSnapshot.renderProfile;
    result.sourceText = m_sourceText;
    result.structuredParseVerification = timedSnapshot.snapshot.structuredParseVerification;
    applyRenderResult(result);
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
        && m_sourceText.size() >= kBackgroundRenderSourceLengthThreshold
        && !canUseLargePlainNativeSurfaceRenderFastPath(m_sourceText);
}

void ContentsStructuredBlockRenderer::applyRenderResult(const RenderResult& result)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredBlockRenderer"),
        QStringLiteral("applyRenderResult"),
        QStringLiteral("blocks=%1 correction=%2 mode=%3 elapsedMs=%4")
            .arg(result.renderedDocumentBlocks.size())
            .arg(!result.correctedSourceText.isEmpty() && result.correctedSourceText != result.sourceText)
            .arg(result.renderProfile.value(QStringLiteral("mode")).toString())
            .arg(result.renderProfile.value(QStringLiteral("elapsedMs")).toLongLong()));
    const bool previousCorrectionSuggested = correctionSuggested();

    updateLastRenderProfile(result.renderProfile);
    updateCorrectedSourceText(result.correctedSourceText);
    updateStructuredParseVerification(result.structuredParseVerification);

    const bool renderPayloadChanged =
        m_renderedDocumentBlocks != result.renderedDocumentBlocks;
    if (renderPayloadChanged)
    {
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

void ContentsStructuredBlockRenderer::updateLastRenderProfile(const QVariantMap& profile)
{
    if (m_lastRenderProfile != profile)
    {
        m_lastRenderProfile = profile;
        emit lastRenderProfileChanged();
    }
    emit renderProfileReported(profile);
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
        const TimedRenderSnapshot timedSnapshot = buildTimedRenderSnapshot(sourceText, QStringLiteral("async"));
        RenderResult result;
        result.correctedSourceText = timedSnapshot.snapshot.correctedSourceText;
        result.renderedDocumentBlocks = timedSnapshot.snapshot.renderedDocumentBlocks;
        result.renderProfile = timedSnapshot.renderProfile;
        result.sequence = sequence;
        result.sourceText = sourceText;
        result.structuredParseVerification = timedSnapshot.snapshot.structuredParseVerification;
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
        m_renderedDocumentBlocks != placeholderBlocks;
    if (renderPayloadChanged)
    {
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
