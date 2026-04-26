#include "app/models/content/structured/ContentsStructuredDocumentHost.hpp"

#include "app/models/content/structured/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentFocusPolicy.hpp"
#include "app/models/content/structured/ContentsStructuredDocumentMutationPolicy.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <cmath>

ContentsStructuredDocumentHost::ContentsStructuredDocumentHost(QObject* parent)
    : QObject(parent)
    , m_collectionPolicy(new ContentsStructuredDocumentCollectionPolicy(this))
    , m_focusPolicy(new ContentsStructuredDocumentFocusPolicy(this))
    , m_mutationPolicy(new ContentsStructuredDocumentMutationPolicy(this))
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("ctor"));
}

ContentsStructuredDocumentHost::~ContentsStructuredDocumentHost()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("dtor"),
        QStringLiteral(
            "activeBlockIndex=%1 cursorRevision=%2 pendingFocusBlockIndex=%3 selectionClearRevision=%4 retainedBlockIndex=%5")
            .arg(m_activeBlockIndex)
            .arg(m_activeBlockCursorRevision)
            .arg(m_pendingFocusBlockIndex)
            .arg(m_selectionClearRevision)
            .arg(m_selectionClearRetainedBlockIndex));
}

QVariant ContentsStructuredDocumentHost::documentBlocks() const
{
    return m_documentBlocks;
}

void ContentsStructuredDocumentHost::setDocumentBlocks(const QVariant& documentBlocks)
{
    if (m_documentBlocks == documentBlocks)
    {
        return;
    }

    m_documentBlocks = documentBlocks;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setDocumentBlocks"),
        QStringLiteral("type=%1")
            .arg(QString::fromLatin1(documentBlocks.typeName() != nullptr ? documentBlocks.typeName() : "unknown")));
    emit documentBlocksChanged();
}

QVariant ContentsStructuredDocumentHost::renderedResources() const
{
    return m_renderedResources;
}

void ContentsStructuredDocumentHost::setRenderedResources(const QVariant& renderedResources)
{
    if (m_renderedResources == renderedResources)
    {
        return;
    }

    m_renderedResources = renderedResources;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setRenderedResources"),
        QStringLiteral("type=%1")
            .arg(QString::fromLatin1(renderedResources.typeName() != nullptr ? renderedResources.typeName() : "unknown")));
    emit renderedResourcesChanged();
}

QString ContentsStructuredDocumentHost::sourceText() const
{
    return m_sourceText;
}

void ContentsStructuredDocumentHost::setSourceText(const QString& sourceText)
{
    if (m_sourceText == sourceText)
    {
        return;
    }

    m_sourceText = sourceText;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setSourceText"),
        QStringLiteral("source=%1").arg(WhatSon::Debug::summarizeText(sourceText)));
    emit sourceTextChanged();
}

int ContentsStructuredDocumentHost::activeBlockIndex() const noexcept
{
    return m_activeBlockIndex;
}

void ContentsStructuredDocumentHost::setActiveBlockIndex(const int activeBlockIndex)
{
    if (m_activeBlockIndex == activeBlockIndex)
    {
        return;
    }

    m_activeBlockIndex = activeBlockIndex;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setActiveBlockIndex"),
        QStringLiteral("activeBlockIndex=%1").arg(activeBlockIndex));
    emit activeBlockIndexChanged();
}

int ContentsStructuredDocumentHost::activeBlockCursorRevision() const noexcept
{
    return m_activeBlockCursorRevision;
}

void ContentsStructuredDocumentHost::setActiveBlockCursorRevision(
    const int activeBlockCursorRevision)
{
    if (m_activeBlockCursorRevision == activeBlockCursorRevision)
    {
        return;
    }

    m_activeBlockCursorRevision = activeBlockCursorRevision;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setActiveBlockCursorRevision"),
        QStringLiteral("revision=%1").arg(activeBlockCursorRevision));
    emit activeBlockCursorRevisionChanged();
}

QVariantMap ContentsStructuredDocumentHost::pendingFocusRequest() const
{
    return m_pendingFocusRequest;
}

void ContentsStructuredDocumentHost::setPendingFocusRequest(
    const QVariantMap& pendingFocusRequest)
{
    if (m_pendingFocusRequest == pendingFocusRequest)
    {
        return;
    }

    m_pendingFocusRequest = pendingFocusRequest;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setPendingFocusRequest"),
        QStringLiteral("requestKeys=%1").arg(pendingFocusRequest.keys().join(QLatin1Char(','))));
    emit pendingFocusRequestChanged();
}

int ContentsStructuredDocumentHost::pendingFocusBlockIndex() const noexcept
{
    return m_pendingFocusBlockIndex;
}

void ContentsStructuredDocumentHost::setPendingFocusBlockIndex(
    const int pendingFocusBlockIndex)
{
    if (m_pendingFocusBlockIndex == pendingFocusBlockIndex)
    {
        return;
    }

    m_pendingFocusBlockIndex = pendingFocusBlockIndex;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setPendingFocusBlockIndex"),
        QStringLiteral("pendingFocusBlockIndex=%1").arg(pendingFocusBlockIndex));
    emit pendingFocusBlockIndexChanged();
}

bool ContentsStructuredDocumentHost::pendingFocusApplyQueued() const noexcept
{
    return m_pendingFocusApplyQueued;
}

void ContentsStructuredDocumentHost::setPendingFocusApplyQueued(
    const bool pendingFocusApplyQueued)
{
    if (m_pendingFocusApplyQueued == pendingFocusApplyQueued)
    {
        return;
    }

    m_pendingFocusApplyQueued = pendingFocusApplyQueued;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setPendingFocusApplyQueued"),
        QStringLiteral("queued=%1").arg(pendingFocusApplyQueued));
    emit pendingFocusApplyQueuedChanged();
}

int ContentsStructuredDocumentHost::selectionClearRevision() const noexcept
{
    return m_selectionClearRevision;
}

void ContentsStructuredDocumentHost::setSelectionClearRevision(
    const int selectionClearRevision)
{
    if (m_selectionClearRevision == selectionClearRevision)
    {
        return;
    }

    m_selectionClearRevision = selectionClearRevision;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setSelectionClearRevision"),
        QStringLiteral("selectionClearRevision=%1").arg(selectionClearRevision));
    emit selectionClearRevisionChanged();
}

int ContentsStructuredDocumentHost::selectionClearRetainedBlockIndex() const noexcept
{
    return m_selectionClearRetainedBlockIndex;
}

void ContentsStructuredDocumentHost::setSelectionClearRetainedBlockIndex(
    const int selectionClearRetainedBlockIndex)
{
    if (m_selectionClearRetainedBlockIndex == selectionClearRetainedBlockIndex)
    {
        return;
    }

    m_selectionClearRetainedBlockIndex = selectionClearRetainedBlockIndex;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setSelectionClearRetainedBlockIndex"),
        QStringLiteral("selectionClearRetainedBlockIndex=%1").arg(selectionClearRetainedBlockIndex));
    emit selectionClearRetainedBlockIndexChanged();
}

QVariantList ContentsStructuredDocumentHost::cachedLogicalLineEntries() const
{
    return m_cachedLogicalLineEntries;
}

void ContentsStructuredDocumentHost::setCachedLogicalLineEntries(
    const QVariantList& cachedLogicalLineEntries)
{
    if (m_cachedLogicalLineEntries == cachedLogicalLineEntries)
    {
        return;
    }

    m_cachedLogicalLineEntries = cachedLogicalLineEntries;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setCachedLogicalLineEntries"),
        QStringLiteral("entryCount=%1").arg(cachedLogicalLineEntries.size()));
    emit cachedLogicalLineEntriesChanged();
}

QVariantList ContentsStructuredDocumentHost::cachedBlockLayoutSummaries() const
{
    return m_cachedBlockLayoutSummaries;
}

void ContentsStructuredDocumentHost::setCachedBlockLayoutSummaries(
    const QVariantList& cachedBlockLayoutSummaries)
{
    if (m_cachedBlockLayoutSummaries == cachedBlockLayoutSummaries)
    {
        return;
    }

    m_cachedBlockLayoutSummaries = cachedBlockLayoutSummaries;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setCachedBlockLayoutSummaries"),
        QStringLiteral("entryCount=%1").arg(cachedBlockLayoutSummaries.size()));
    emit cachedBlockLayoutSummariesChanged();
}

bool ContentsStructuredDocumentHost::layoutCacheRefreshQueued() const noexcept
{
    return m_layoutCacheRefreshQueued;
}

void ContentsStructuredDocumentHost::setLayoutCacheRefreshQueued(
    const bool layoutCacheRefreshQueued)
{
    if (m_layoutCacheRefreshQueued == layoutCacheRefreshQueued)
    {
        return;
    }

    m_layoutCacheRefreshQueued = layoutCacheRefreshQueued;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setLayoutCacheRefreshQueued"),
        QStringLiteral("queued=%1").arg(layoutCacheRefreshQueued));
    emit layoutCacheRefreshQueuedChanged();
}

qreal ContentsStructuredDocumentHost::viewportContentY() const noexcept
{
    return m_viewportContentY;
}

void ContentsStructuredDocumentHost::setViewportContentY(const qreal viewportContentY)
{
    if (qFuzzyCompare(m_viewportContentY, viewportContentY))
    {
        return;
    }

    m_viewportContentY = viewportContentY;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setViewportContentY"),
        QStringLiteral("viewportContentY=%1").arg(viewportContentY));
    emit viewportContentYChanged();
}

qreal ContentsStructuredDocumentHost::viewportHeight() const noexcept
{
    return m_viewportHeight;
}

void ContentsStructuredDocumentHost::setViewportHeight(const qreal viewportHeight)
{
    if (qFuzzyCompare(m_viewportHeight, viewportHeight))
    {
        return;
    }

    m_viewportHeight = viewportHeight;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentHost"),
        QStringLiteral("setViewportHeight"),
        QStringLiteral("viewportHeight=%1").arg(viewportHeight));
    emit viewportHeightChanged();
}

QObject* ContentsStructuredDocumentHost::collectionPolicy() const noexcept
{
    return m_collectionPolicy;
}

QObject* ContentsStructuredDocumentHost::focusPolicy() const noexcept
{
    return m_focusPolicy;
}

QObject* ContentsStructuredDocumentHost::mutationPolicy() const noexcept
{
    return m_mutationPolicy;
}

int ContentsStructuredDocumentHost::resolvedInteractiveBlockIndex(
    const int focusedBlockIndex) const noexcept
{
    if (focusedBlockIndex >= 0)
    {
        return focusedBlockIndex;
    }
    if (m_activeBlockIndex >= 0)
    {
        return m_activeBlockIndex;
    }
    return -1;
}

int ContentsStructuredDocumentHost::shortcutInsertionSourceOffset(
    const int focusedBlockIndex,
    const QVariant& delegateInsertionOffset) const
{
    if (m_focusPolicy == nullptr)
    {
        return -1;
    }

    return m_focusPolicy->shortcutInsertionSourceOffset(
        m_documentBlocks,
        resolvedInteractiveBlockIndex(focusedBlockIndex),
        m_pendingFocusRequest,
        m_sourceText,
        delegateInsertionOffset);
}

void ContentsStructuredDocumentHost::noteActiveBlockInteraction(const int blockIndex)
{
    requestSelectionClear(blockIndex);
    setActiveBlockIndex(blockIndex);
    setActiveBlockCursorRevision(m_activeBlockCursorRevision + 1);
}

void ContentsStructuredDocumentHost::noteActiveBlockCursorInteraction(const int blockIndex)
{
    if (m_activeBlockIndex != blockIndex)
    {
        requestSelectionClear(blockIndex);
    }
    setActiveBlockIndex(blockIndex);
    setActiveBlockCursorRevision(m_activeBlockCursorRevision + 1);
}

void ContentsStructuredDocumentHost::requestSelectionClear(
    const int retainedBlockIndex)
{
    setSelectionClearRetainedBlockIndex(retainedBlockIndex);
    setSelectionClearRevision(m_selectionClearRevision + 1);
}

void ContentsStructuredDocumentHost::clearPendingFocusRequest()
{
    setPendingFocusRequest({});
    setPendingFocusBlockIndex(-1);
}
