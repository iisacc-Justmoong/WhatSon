#include "ContentsStructuredDocumentMutationPolicy.hpp"

#include "ContentsStructuredDocumentCollectionPolicy.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QJSValue>
#include <QMetaType>
#include <QSequentialIterable>
#include <QStringList>

#include <algorithm>

namespace
{
    QVariantList normalizedVariantList(QVariant value)
    {
        if (!value.isValid())
        {
            return {};
        }

        if (value.metaType().id() == QMetaType::QJSValue)
        {
            value = value.value<QJSValue>().toVariant();
        }

        if (value.metaType().id() == QMetaType::QVariantList)
        {
            return value.toList();
        }

        if (value.metaType().id() == QMetaType::QStringList)
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
