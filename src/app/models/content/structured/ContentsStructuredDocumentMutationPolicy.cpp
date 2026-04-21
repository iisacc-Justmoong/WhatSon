#include "ContentsStructuredDocumentMutationPolicy.hpp"

#include "ContentsStructuredDocumentCollectionPolicy.hpp"
#include "models/file/WhatSonDebugTrace.hpp"
#include "models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <QJSValue>
#include <QMetaType>
#include <QSequentialIterable>
#include <QStringList>

#include <algorithm>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    QVariantList normalizedVariantList(QVariant value)
    {
        if (!value.isValid())
        {
            return {};
        }

        if (value.metaType() == QMetaType::fromType<QJSValue>())
        {
            value = value.value<QJSValue>().toVariant();
        }

        if (value.metaType() == QMetaType::fromType<QVariantList>())
        {
            return value.toList();
        }

        if (value.metaType() == QMetaType::fromType<QStringList>())
        {
            const QStringList stringValues = value.toStringList();
            QVariantList normalizedValues;
            normalizedValues.reserve(stringValues.size());
            for (const QString& stringValue : stringValues)
            {
                normalizedValues.push_back(stringValue);
            }
            return normalizedValues;
        }

        if (value.canConvert<QSequentialIterable>())
        {
            const QSequentialIterable iterable = value.value<QSequentialIterable>();
            QVariantList normalizedValues;
            for (auto it = iterable.begin(); it != iterable.end(); ++it)
            {
                normalizedValues.push_back(*it);
            }
            if (!normalizedValues.isEmpty())
            {
                return normalizedValues;
            }
        }

        return QVariantList{value};
    }

    QString canonicalParagraphBoundaryType(const QVariantMap& blockData)
    {
        static const QStringList candidateKeys{
            QStringLiteral("semanticTagName"),
            QStringLiteral("tagName"),
            QStringLiteral("type"),
        };
        for (const QString& candidateKey : candidateKeys)
        {
            const QString rawValue = blockData.value(candidateKey).toString().trimmed();
            if (rawValue.isEmpty())
            {
                continue;
            }

            const QString renderedType = SemanticTags::canonicalRenderedTextBlockTagName(rawValue);
            if (!renderedType.isEmpty())
            {
                return renderedType;
            }

            const QString documentType = SemanticTags::canonicalDocumentBlockTypeName(rawValue);
            if (!documentType.isEmpty())
            {
                return documentType;
            }
        }

        return {};
    }

    bool isParagraphBoundaryType(const QString& canonicalTypeName)
    {
        return canonicalTypeName == QStringLiteral("paragraph")
            || canonicalTypeName == QStringLiteral("p");
    }

    QVariantMap paragraphBoundaryFocusRequest(const int sourceOffset)
    {
        QVariantMap focusRequest;
        focusRequest.insert(QStringLiteral("preferNearestTextBlock"), true);
        focusRequest.insert(QStringLiteral("sourceOffset"), std::max(0, sourceOffset));
        return focusRequest;
    }

    int boundedBlockOffset(
        ContentsStructuredDocumentCollectionPolicy* collectionPolicy,
        const QVariantMap& blockData,
        const QString& key,
        const int fallbackValue,
        const int sourceLength)
    {
        return std::clamp(
            collectionPolicy->floorNumberOrFallback(blockData.value(key), fallbackValue),
            0,
            sourceLength);
    }

    int blockContentStart(
        ContentsStructuredDocumentCollectionPolicy* collectionPolicy,
        const QVariantMap& blockData,
        const int sourceLength)
    {
        const int sourceStart = boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("sourceStart"),
            0,
            sourceLength);
        return boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("contentStart"),
            sourceStart,
            sourceLength);
    }

    int blockContentEnd(
        ContentsStructuredDocumentCollectionPolicy* collectionPolicy,
        const QVariantMap& blockData,
        const int contentStart,
        const int sourceLength)
    {
        const int sourceEnd = boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("sourceEnd"),
            contentStart,
            sourceLength);
        return boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("contentEnd"),
            sourceEnd,
            sourceLength);
    }

    int blockSourceStart(
        ContentsStructuredDocumentCollectionPolicy* collectionPolicy,
        const QVariantMap& blockData,
        const int contentStart,
        const int sourceLength)
    {
        return boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("blockSourceStart"),
            contentStart,
            sourceLength);
    }

    int blockSourceEnd(
        ContentsStructuredDocumentCollectionPolicy* collectionPolicy,
        const QVariantMap& blockData,
        const int contentEnd,
        const int sourceLength)
    {
        return boundedBlockOffset(
            collectionPolicy,
            blockData,
            QStringLiteral("blockSourceEnd"),
            contentEnd,
            sourceLength);
    }
} // namespace

ContentsStructuredDocumentMutationPolicy::ContentsStructuredDocumentMutationPolicy(QObject* parent)
    : QObject(parent)
    , m_collectionPolicy(new ContentsStructuredDocumentCollectionPolicy(this))
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentMutationPolicy"),
        QStringLiteral("ctor"));
}

ContentsStructuredDocumentMutationPolicy::~ContentsStructuredDocumentMutationPolicy()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentMutationPolicy"),
        QStringLiteral("dtor"));
}

QVariantMap ContentsStructuredDocumentMutationPolicy::emptyTextBlockDeletionRange(
    const QVariantMap& blockData,
    const QString& direction,
    const QString& sourceText) const
{
    const QString currentSourceText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int currentSourceLength = static_cast<int>(currentSourceText.size());
    const int anchorOffset = std::clamp(
        m_collectionPolicy->floorNumberOrFallback(blockData.value(QStringLiteral("sourceStart")), 0),
        0,
        currentSourceLength);
    const int previousNewlineStart = anchorOffset > 0
                                         && currentSourceText.at(anchorOffset - 1)
                                                == QLatin1Char('\n')
                                     ? anchorOffset - 1
                                     : -1;
    const int nextNewlineStart = anchorOffset < currentSourceLength
                                     && currentSourceText.at(anchorOffset)
                                            == QLatin1Char('\n')
                                 ? anchorOffset
                                 : -1;

    int deletionStart = -1;
    if (normalizedDeletionDirection(direction) == QStringLiteral("forward"))
    {
        deletionStart = nextNewlineStart >= 0 ? nextNewlineStart : previousNewlineStart;
    }
    else
    {
        deletionStart = previousNewlineStart >= 0 ? previousNewlineStart : nextNewlineStart;
    }

    if (deletionStart < 0)
    {
        return {};
    }

    QVariantMap focusRequest;
    focusRequest.insert(QStringLiteral("preferNearestTextBlock"), true);
    focusRequest.insert(QStringLiteral("sourceOffset"), deletionStart);

    QVariantMap deletionRange;
    deletionRange.insert(QStringLiteral("start"), deletionStart);
    deletionRange.insert(QStringLiteral("end"), deletionStart + 1);
    deletionRange.insert(QStringLiteral("focusRequest"), focusRequest);
    return deletionRange;
}

int ContentsStructuredDocumentMutationPolicy::nextEditableSourceOffsetAfterBlock(
    const QString& sourceText,
    const int blockEndOffset) const
{
    const QString normalizedText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int normalizedLength = static_cast<int>(normalizedText.size());
    const int boundedBlockEndOffset = std::clamp(blockEndOffset, 0, normalizedLength);
    if (boundedBlockEndOffset < normalizedLength
        && normalizedText.at(boundedBlockEndOffset) == QLatin1Char('\n'))
    {
        return boundedBlockEndOffset + 1;
    }
    return boundedBlockEndOffset;
}

bool ContentsStructuredDocumentMutationPolicy::supportsParagraphBoundaryOperations(
    const QVariantMap& blockData) const
{
    return isParagraphBoundaryType(canonicalParagraphBoundaryType(blockData));
}

QVariantMap ContentsStructuredDocumentMutationPolicy::buildParagraphMergePayload(
    const QVariantMap& previousBlockData,
    const QVariantMap& currentBlockData,
    const QString& sourceText) const
{
    if (!supportsParagraphBoundaryOperations(previousBlockData)
        || !supportsParagraphBoundaryOperations(currentBlockData))
    {
        return {};
    }

    const QString currentSourceText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int sourceLength = static_cast<int>(currentSourceText.size());

    const int previousContentStart =
        blockContentStart(m_collectionPolicy, previousBlockData, sourceLength);
    const int previousContentEnd =
        blockContentEnd(m_collectionPolicy, previousBlockData, previousContentStart, sourceLength);
    const int previousBlockStart =
        blockSourceStart(m_collectionPolicy, previousBlockData, previousContentStart, sourceLength);
    const int previousBlockEnd =
        blockSourceEnd(m_collectionPolicy, previousBlockData, previousContentEnd, sourceLength);

    const int currentContentStart =
        blockContentStart(m_collectionPolicy, currentBlockData, sourceLength);
    const int currentContentEnd =
        blockContentEnd(m_collectionPolicy, currentBlockData, currentContentStart, sourceLength);
    const int currentBlockStart =
        blockSourceStart(m_collectionPolicy, currentBlockData, currentContentStart, sourceLength);
    const int currentBlockEnd =
        blockSourceEnd(m_collectionPolicy, currentBlockData, currentContentEnd, sourceLength);

    if (previousBlockStart > previousContentStart
        || previousContentEnd > previousBlockEnd
        || currentBlockStart > currentContentStart
        || currentContentEnd > currentBlockEnd
        || previousBlockEnd > currentBlockStart
        || previousContentEnd > currentBlockEnd)
    {
        return {};
    }

    const QString previousClosingSuffix =
        currentSourceText.mid(previousContentEnd, previousBlockEnd - previousContentEnd);
    const QString currentContentText =
        currentSourceText.mid(currentContentStart, currentContentEnd - currentContentStart);

    QVariantMap payload;
    payload.insert(
        QStringLiteral("nextSourceText"),
        m_collectionPolicy->spliceSourceRange(
            currentSourceText,
            previousContentEnd,
            currentBlockEnd,
            currentContentText + previousClosingSuffix));
    payload.insert(
        QStringLiteral("focusRequest"),
        paragraphBoundaryFocusRequest(previousContentEnd));
    return payload;
}

QVariantMap ContentsStructuredDocumentMutationPolicy::buildParagraphSplitPayload(
    const QVariantMap& blockData,
    const QString& sourceText,
    const int sourceCursorOffset) const
{
    if (!supportsParagraphBoundaryOperations(blockData))
    {
        return {};
    }

    const QString currentSourceText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int sourceLength = static_cast<int>(currentSourceText.size());

    const int contentStart = blockContentStart(m_collectionPolicy, blockData, sourceLength);
    const int contentEnd =
        blockContentEnd(m_collectionPolicy, blockData, contentStart, sourceLength);
    const int blockStart =
        blockSourceStart(m_collectionPolicy, blockData, contentStart, sourceLength);
    const int blockEnd =
        blockSourceEnd(m_collectionPolicy, blockData, contentEnd, sourceLength);

    if (blockStart > contentStart || contentEnd > blockEnd)
    {
        return {};
    }

    const int boundedSplitOffset = std::clamp(sourceCursorOffset, contentStart, contentEnd);
    const bool explicitWrapper = blockStart < contentStart || blockEnd > contentEnd;

    QString nextSourceText;
    int focusSourceOffset = boundedSplitOffset + 1;
    if (!explicitWrapper)
    {
        nextSourceText = m_collectionPolicy->spliceSourceRange(
            currentSourceText,
            boundedSplitOffset,
            boundedSplitOffset,
            QStringLiteral("\n"));
    }
    else
    {
        const QString openingPrefix =
            currentSourceText.mid(blockStart, contentStart - blockStart);
        const QString closingSuffix =
            currentSourceText.mid(contentEnd, blockEnd - contentEnd);
        if (openingPrefix.isEmpty() || closingSuffix.isEmpty())
        {
            return {};
        }

        const QString rightContent =
            currentSourceText.mid(boundedSplitOffset, contentEnd - boundedSplitOffset);
        const QString splitReplacement =
            closingSuffix + QStringLiteral("\n") + openingPrefix + rightContent + closingSuffix;
        nextSourceText = m_collectionPolicy->spliceSourceRange(
            currentSourceText,
            boundedSplitOffset,
            blockEnd,
            splitReplacement);
        focusSourceOffset =
            boundedSplitOffset + static_cast<int>(closingSuffix.size()) + 1
            + static_cast<int>(openingPrefix.size());
    }

    QVariantMap payload;
    payload.insert(QStringLiteral("nextSourceText"), nextSourceText);
    payload.insert(
        QStringLiteral("focusRequest"),
        paragraphBoundaryFocusRequest(
            std::clamp(focusSourceOffset, 0, static_cast<int>(nextSourceText.size()))));
    return payload;
}

QVariantMap ContentsStructuredDocumentMutationPolicy::buildStructuredInsertionPayload(
    const QString& sourceText,
    const int insertionOffset,
    const QString& insertionSourceText,
    const int cursorSourceOffsetFromInsertionStart) const
{
    const QString currentSourceText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int currentSourceLength = static_cast<int>(currentSourceText.size());
    const int boundedInsertionOffset = std::clamp(insertionOffset, 0, currentSourceLength);
    const QString prefixNewline =
        boundedInsertionOffset > 0
            && currentSourceText.at(boundedInsertionOffset - 1) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString suffixNewline =
        boundedInsertionOffset < currentSourceLength
            && currentSourceText.at(boundedInsertionOffset) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString normalizedInsertionSourceText = prefixNewline + insertionSourceText + suffixNewline;

    QVariantMap payload;
    payload.insert(
        QStringLiteral("nextSourceText"),
        m_collectionPolicy->spliceSourceRange(
            currentSourceText,
            boundedInsertionOffset,
            boundedInsertionOffset,
            normalizedInsertionSourceText));
    payload.insert(
        QStringLiteral("sourceOffset"),
        boundedInsertionOffset + static_cast<int>(prefixNewline.size())
            + std::max(0, cursorSourceOffsetFromInsertionStart));
    payload.insert(QStringLiteral("insertedSourceText"), normalizedInsertionSourceText);
    return payload;
}

QVariantMap ContentsStructuredDocumentMutationPolicy::buildResourceInsertionPayload(
    const QString& sourceText,
    const int insertionOffset,
    const QVariant& tagTexts) const
{
    const QVariantList rawTagTexts = normalizedVariantList(tagTexts);
    QStringList normalizedTagTexts;
    normalizedTagTexts.reserve(rawTagTexts.size());
    for (const QVariant& tagTextValue : rawTagTexts)
    {
        const QString tagText = tagTextValue.toString().trimmed();
        if (!tagText.isEmpty())
        {
            normalizedTagTexts.push_back(tagText);
        }
    }

    if (normalizedTagTexts.isEmpty())
    {
        return {};
    }

    const QString currentSourceText = m_collectionPolicy->normalizeSourceText(sourceText);
    const int currentSourceLength = static_cast<int>(currentSourceText.size());
    const int boundedInsertionOffset = std::clamp(insertionOffset, 0, currentSourceLength);
    const QString prefixNewline =
        boundedInsertionOffset > 0
            && currentSourceText.at(boundedInsertionOffset - 1) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString blockSourceText = normalizedTagTexts.join(QLatin1Char('\n'));
    const QString suffixNewline =
        boundedInsertionOffset < currentSourceLength
            && currentSourceText.at(boundedInsertionOffset) != QLatin1Char('\n')
        ? QStringLiteral("\n")
        : QString();
    const QString insertionSourceText = prefixNewline + blockSourceText + suffixNewline;
    const QString nextSourceText = m_collectionPolicy->spliceSourceRange(
        currentSourceText,
        boundedInsertionOffset,
        boundedInsertionOffset,
        insertionSourceText);
    const int insertedBlockEndOffset =
        boundedInsertionOffset + static_cast<int>(prefixNewline.size())
        + static_cast<int>(blockSourceText.size());

    QVariantMap focusRequest;
    if (!suffixNewline.isEmpty())
    {
        focusRequest.insert(
            QStringLiteral("sourceOffset"),
            nextEditableSourceOffsetAfterBlock(nextSourceText, insertedBlockEndOffset));
    }
    else
    {
        focusRequest.insert(
            QStringLiteral("sourceOffset"),
            std::max(
                boundedInsertionOffset + static_cast<int>(prefixNewline.size()),
                insertedBlockEndOffset - 1));
    }

    QVariantMap payload;
    payload.insert(QStringLiteral("focusRequest"), focusRequest);
    payload.insert(QStringLiteral("insertedSourceText"), insertionSourceText);
    payload.insert(QStringLiteral("nextSourceText"), nextSourceText);
    return payload;
}

QString ContentsStructuredDocumentMutationPolicy::normalizedDeletionDirection(
    const QString& direction) const
{
    const QString normalizedDirection = direction.trimmed().toLower();
    return normalizedDirection == QStringLiteral("forward") ? QStringLiteral("forward")
                                                            : QStringLiteral("backward");
}
