#include "app/models/editor/structure/ContentsStructuredDocumentFocusPolicy.hpp"

#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    QVariantMap normalizedMap(const QVariant& value)
    {
        return value.typeId() == QMetaType::QVariantMap ? value.toMap() : QVariantMap{};
    }

    QVariantList normalizedTasks(const QVariantMap& blockEntry)
    {
        const QVariant tasksValue = blockEntry.value(QStringLiteral("tasks"));
        return tasksValue.typeId() == QMetaType::QVariantList ? tasksValue.toList() : QVariantList{};
    }
}

ContentsStructuredDocumentFocusPolicy::ContentsStructuredDocumentFocusPolicy(QObject* parent)
    : QObject(parent)
    , m_collectionPolicy(new ContentsStructuredDocumentCollectionPolicy(this))
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentFocusPolicy"),
        QStringLiteral("ctor"));
}

ContentsStructuredDocumentFocusPolicy::~ContentsStructuredDocumentFocusPolicy()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentFocusPolicy"),
        QStringLiteral("dtor"));
}

int ContentsStructuredDocumentFocusPolicy::focusTargetBlockIndex(
    const QVariant& rawBlocks,
    const int activeBlockIndex,
    const QVariantMap& request) const
{
    const QVariantList blocks = normalizedBlocks(rawBlocks);
    if (blocks.isEmpty())
    {
        return -1;
    }

    const int explicitTargetBlockIndex = normalizedFocusTargetBlockIndex(request);
    if (explicitTargetBlockIndex >= 0 && explicitTargetBlockIndex < blocks.size())
    {
        return explicitTargetBlockIndex;
    }

    const int taskOpenTagStart = normalizedFocusTaskOpenTagStart(request);
    if (taskOpenTagStart >= 0)
    {
        for (int index = 0; index < blocks.size(); ++index)
        {
            if (blockContainsTaskOpenTagStart(normalizedMap(blocks.at(index)), taskOpenTagStart))
            {
                return index;
            }
        }
    }

    const int sourceOffset = normalizedFocusSourceOffset(request);
    if (sourceOffset >= 0)
    {
        int fallbackBeforeIndex = -1;
        int fallbackAfterIndex = -1;
        int fallbackBeforeEditableIndex = -1;
        int fallbackAfterEditableIndex = -1;

        for (int index = 0; index < blocks.size(); ++index)
        {
            const QVariantMap blockEntry = normalizedMap(blocks.at(index));
            if (blockContainsSourceOffset(blockEntry, sourceOffset))
            {
                return index;
            }

            const int sourceStart = std::max(
                0,
                m_collectionPolicy->floorNumberOrFallback(
                    blockEntry.value(QStringLiteral("sourceStart")),
                    0));
            const int sourceEnd = std::max(
                sourceStart,
                m_collectionPolicy->floorNumberOrFallback(
                    blockEntry.value(QStringLiteral("sourceEnd")),
                    sourceStart));

            if (sourceEnd < sourceOffset)
            {
                fallbackBeforeIndex = index;
                if (blockTextEditable(blockEntry))
                {
                    fallbackBeforeEditableIndex = index;
                }
                continue;
            }

            if (sourceStart > sourceOffset && fallbackAfterIndex < 0)
            {
                fallbackAfterIndex = index;
                if (blockTextEditable(blockEntry))
                {
                    fallbackAfterEditableIndex = index;
                }
            }
        }

        if (requestPrefersNearestTextBlock(request))
        {
            if (fallbackAfterEditableIndex >= 0)
            {
                return fallbackAfterEditableIndex;
            }
            if (fallbackBeforeEditableIndex >= 0)
            {
                return fallbackBeforeEditableIndex;
            }
            if (fallbackAfterIndex >= 0)
            {
                return fallbackAfterIndex;
            }
            if (fallbackBeforeIndex >= 0)
            {
                return fallbackBeforeIndex;
            }
        }
    }

    if (activeBlockIndex >= 0 && activeBlockIndex < blocks.size())
    {
        return activeBlockIndex;
    }

    return -1;
}

int ContentsStructuredDocumentFocusPolicy::shortcutInsertionSourceOffset(
    const QVariant& rawBlocks,
    const int interactiveBlockIndex,
    const QVariantMap& pendingFocusRequest,
    const QString& sourceText,
    const QVariant& delegateInsertionOffset) const
{
    const QVariantList blocks = normalizedBlocks(rawBlocks);
    const QString currentSourceText = m_collectionPolicy != nullptr
                                          ? m_collectionPolicy->normalizeSourceText(sourceText)
                                          : sourceText;
    const int currentSourceLength = static_cast<int>(currentSourceText.size());
    const int pendingSourceOffset =
        boundedPendingFocusSourceOffset(pendingFocusRequest, currentSourceLength);

    if (blocks.isEmpty())
    {
        return currentSourceLength == 0 ? 0 : pendingSourceOffset;
    }

    if (interactiveBlockIndex < 0 || interactiveBlockIndex >= blocks.size())
    {
        return pendingSourceOffset;
    }

    bool delegateOffsetOk = false;
    const double delegateOffset = delegateInsertionOffset.toDouble(&delegateOffsetOk);
    if (delegateOffsetOk && std::isfinite(delegateOffset))
    {
        return std::clamp(
            static_cast<int>(std::floor(delegateOffset)),
            0,
            currentSourceLength);
    }

    const QVariantMap blockEntry = normalizedMap(blocks.at(interactiveBlockIndex));
    if (blockTextEditable(blockEntry))
    {
        return pendingSourceOffset;
    }

    const int blockSourceStart = std::max(
        0,
        m_collectionPolicy->floorNumberOrFallback(
            blockEntry.value(QStringLiteral("sourceStart")),
            0));
    const int blockSourceEnd = std::max(
        blockSourceStart,
        m_collectionPolicy->floorNumberOrFallback(
            blockEntry.value(QStringLiteral("sourceEnd")),
            blockSourceStart));
    return std::clamp(blockSourceEnd, 0, currentSourceLength);
}

QVariantMap ContentsStructuredDocumentFocusPolicy::focusRequestAfterBlockDeletion(
    const QVariant& rawBlocks,
    const int activeBlockIndex,
    const QVariantMap& blockData,
    const QString& nextSourceText) const
{
    const QVariantList blocks = normalizedBlocks(rawBlocks);
    const int blockSourceStart = std::max(
        0,
        m_collectionPolicy->floorNumberOrFallback(
            blockData.value(QStringLiteral("sourceStart")),
            0));
    const int blockSourceEnd = std::max(
        blockSourceStart,
        m_collectionPolicy->floorNumberOrFallback(
            blockData.value(QStringLiteral("sourceEnd")),
            blockSourceStart));
    const int blockIndex = blockIndexForEntry(blocks, activeBlockIndex, blockData);
    const int deletedLength = std::max(0, blockSourceEnd - blockSourceStart);
    const int previousEditableOffset = blockIndex >= 0
                                           ? previousEditableBlockFocusSourceOffset(blocks, blockIndex)
                                           : -1;
    const int nextEditableOffset =
        blockIndex >= 0 ? nextEditableBlockFocusSourceOffset(blocks, blockIndex) : -1;
    const int adjustedNextEditableOffset =
        nextEditableOffset >= 0 ? std::max(0, nextEditableOffset - deletedLength) : -1;
    const QString boundedNextSourceText = m_collectionPolicy->normalizeSourceText(nextSourceText);
    const int boundedNextSourceLength = static_cast<int>(boundedNextSourceText.size());

    QVariantMap focusRequest;
    focusRequest.insert(QStringLiteral("preferNearestTextBlock"), true);
    if (adjustedNextEditableOffset >= 0)
    {
        focusRequest.insert(
            QStringLiteral("sourceOffset"),
            std::min(boundedNextSourceLength, adjustedNextEditableOffset));
        return focusRequest;
    }

    if (previousEditableOffset >= 0)
    {
        focusRequest.insert(
            QStringLiteral("sourceOffset"),
            std::min(boundedNextSourceLength, previousEditableOffset));
        return focusRequest;
    }

    focusRequest.insert(
        QStringLiteral("sourceOffset"),
        std::min(boundedNextSourceLength, blockSourceStart));
    return focusRequest;
}

QVariantList ContentsStructuredDocumentFocusPolicy::normalizedBlocks(
    const QVariant& rawBlocks) const
{
    return m_collectionPolicy != nullptr ? m_collectionPolicy->normalizeEntries(rawBlocks)
                                         : QVariantList{};
}

int ContentsStructuredDocumentFocusPolicy::normalizedFocusTaskOpenTagStart(
    const QVariantMap& request) const
{
    bool ok = false;
    const int value = request.value(QStringLiteral("taskOpenTagStart")).toInt(&ok);
    return ok ? value : -1;
}

int ContentsStructuredDocumentFocusPolicy::normalizedFocusTargetBlockIndex(
    const QVariantMap& request) const
{
    bool ok = false;
    const int value = request.value(QStringLiteral("targetBlockIndex")).toInt(&ok);
    return ok ? std::max(0, value) : -1;
}

int ContentsStructuredDocumentFocusPolicy::normalizedFocusSourceOffset(
    const QVariantMap& request) const
{
    bool ok = false;
    const int value = request.value(QStringLiteral("sourceOffset")).toInt(&ok);
    return ok ? std::max(0, value) : -1;
}

int ContentsStructuredDocumentFocusPolicy::boundedPendingFocusSourceOffset(
    const QVariantMap& request,
    const int sourceLength) const
{
    const int sourceOffset = normalizedFocusSourceOffset(request);
    if (sourceOffset < 0)
    {
        return -1;
    }
    return std::clamp(sourceOffset, 0, std::max(0, sourceLength));
}

bool ContentsStructuredDocumentFocusPolicy::requestPrefersNearestTextBlock(
    const QVariantMap& request) const
{
    return request.value(QStringLiteral("preferNearestTextBlock")).toBool();
}

bool ContentsStructuredDocumentFocusPolicy::blockContainsTaskOpenTagStart(
    const QVariantMap& blockEntry,
    const int taskOpenTagStart) const
{
    if (taskOpenTagStart < 0)
    {
        return false;
    }

    const QVariantList tasks = normalizedTasks(blockEntry);
    for (const QVariant& taskValue : tasks)
    {
        const QVariantMap taskData = normalizedMap(taskValue);
        if (m_collectionPolicy->floorNumberOrFallback(
                taskData.value(QStringLiteral("openTagStart")),
                -1)
            == taskOpenTagStart)
        {
            return true;
        }
    }
    return false;
}

bool ContentsStructuredDocumentFocusPolicy::blockUsesExclusiveTrailingBoundary(
    const QVariantMap& blockEntry) const
{
    const QString type = blockEntry.value(QStringLiteral("type")).toString().trimmed().toLower();
    if (blockEntry.contains(QStringLiteral("atomicBlock")))
    {
        return blockEntry.value(QStringLiteral("atomicBlock")).toBool();
    }
    return type == QStringLiteral("resource") || type == QStringLiteral("break");
}

bool ContentsStructuredDocumentFocusPolicy::blockContainsSourceOffset(
    const QVariantMap& blockEntry,
    const int sourceOffset) const
{
    if (sourceOffset < 0)
    {
        return false;
    }

    const int sourceStart = std::max(
        0,
        m_collectionPolicy->floorNumberOrFallback(
            blockEntry.value(QStringLiteral("sourceStart")),
            0));
    const int sourceEnd = std::max(
        sourceStart,
        m_collectionPolicy->floorNumberOrFallback(
            blockEntry.value(QStringLiteral("sourceEnd")),
            sourceStart));
    if (sourceStart == sourceEnd)
    {
        return sourceOffset == sourceStart;
    }

    if (blockUsesExclusiveTrailingBoundary(blockEntry))
    {
        return sourceOffset >= sourceStart && sourceOffset < sourceEnd;
    }
    return sourceOffset >= sourceStart && sourceOffset <= sourceEnd;
}

bool ContentsStructuredDocumentFocusPolicy::blockTextEditable(
    const QVariantMap& blockEntry) const
{
    if (blockEntry.contains(QStringLiteral("textEditable")))
    {
        return blockEntry.value(QStringLiteral("textEditable")).toBool();
    }
    return !blockUsesExclusiveTrailingBoundary(blockEntry);
}

int ContentsStructuredDocumentFocusPolicy::blockIndexForEntry(
    const QVariantList& blocks,
    const int activeBlockIndex,
    const QVariantMap& blockData) const
{
    const int targetSourceStart = std::max(
        0,
        m_collectionPolicy->floorNumberOrFallback(
            blockData.value(QStringLiteral("sourceStart")),
            0));
    const int targetSourceEnd = std::max(
        targetSourceStart,
        m_collectionPolicy->floorNumberOrFallback(
            blockData.value(QStringLiteral("sourceEnd")),
            targetSourceStart));
    const QString targetType = blockData.value(QStringLiteral("type")).toString().trimmed().toLower();

    for (int index = 0; index < blocks.size(); ++index)
    {
        const QVariantMap blockEntry = normalizedMap(blocks.at(index));
        const int blockSourceStart = std::max(
            0,
            m_collectionPolicy->floorNumberOrFallback(
                blockEntry.value(QStringLiteral("sourceStart")),
                0));
        const int blockSourceEnd = std::max(
            blockSourceStart,
            m_collectionPolicy->floorNumberOrFallback(
                blockEntry.value(QStringLiteral("sourceEnd")),
                blockSourceStart));
        const QString blockType =
            blockEntry.value(QStringLiteral("type")).toString().trimmed().toLower();
        if (blockSourceStart == targetSourceStart && blockSourceEnd == targetSourceEnd
            && blockType == targetType)
        {
            return index;
        }
    }

    if (activeBlockIndex >= 0 && activeBlockIndex < blocks.size())
    {
        return activeBlockIndex;
    }
    return -1;
}

int ContentsStructuredDocumentFocusPolicy::previousEditableBlockFocusSourceOffset(
    const QVariantList& blocks,
    const int blockIndex) const
{
    const int maxIndex = std::max(0, static_cast<int>(blocks.size()) - 1);
    const int safeBlockIndex = std::clamp(blockIndex, 0, maxIndex);
    for (int index = safeBlockIndex - 1; index >= 0; --index)
    {
        const QVariantMap blockEntry = normalizedMap(blocks.at(index));
        if (!blockTextEditable(blockEntry))
        {
            continue;
        }
        return std::max(
            0,
            m_collectionPolicy->floorNumberOrFallback(
                blockEntry.value(QStringLiteral("sourceEnd")),
                0));
    }
    return -1;
}

int ContentsStructuredDocumentFocusPolicy::nextEditableBlockFocusSourceOffset(
    const QVariantList& blocks,
    const int blockIndex) const
{
    const int maxIndex = std::max(0, static_cast<int>(blocks.size()) - 1);
    const int safeBlockIndex = std::clamp(blockIndex, 0, maxIndex);
    for (int index = safeBlockIndex + 1; index < blocks.size(); ++index)
    {
        const QVariantMap blockEntry = normalizedMap(blocks.at(index));
        if (!blockTextEditable(blockEntry))
        {
            continue;
        }
        return std::max(
            0,
            m_collectionPolicy->floorNumberOrFallback(
                blockEntry.value(QStringLiteral("sourceStart")),
                0));
    }
    return -1;
}
