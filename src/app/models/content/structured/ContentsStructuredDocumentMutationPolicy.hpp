#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantMap>

class ContentsStructuredDocumentCollectionPolicy;

class ContentsStructuredDocumentMutationPolicy : public QObject
{
    Q_OBJECT

public:
    explicit ContentsStructuredDocumentMutationPolicy(QObject* parent = nullptr);
    ~ContentsStructuredDocumentMutationPolicy() override;

    Q_INVOKABLE QVariantMap emptyTextBlockDeletionRange(
        const QVariantMap& blockData,
        const QString& direction,
        const QString& sourceText) const;
    Q_INVOKABLE int nextEditableSourceOffsetAfterBlock(
        const QString& sourceText,
        int blockEndOffset) const;
    Q_INVOKABLE bool supportsParagraphBoundaryOperations(
        const QVariantMap& blockData) const;
    Q_INVOKABLE QVariantMap buildParagraphMergePayload(
        const QVariantMap& previousBlockData,
        const QVariantMap& currentBlockData,
        const QString& sourceText) const;
    Q_INVOKABLE QVariantMap buildParagraphSplitPayload(
        const QVariantMap& blockData,
        const QString& sourceText,
        int sourceCursorOffset) const;
    Q_INVOKABLE QVariantMap buildStructuredInsertionPayload(
        const QString& sourceText,
        int insertionOffset,
        const QString& insertionSourceText,
        int cursorSourceOffsetFromInsertionStart) const;
    Q_INVOKABLE QVariantMap buildResourceInsertionPayload(
        const QString& sourceText,
        int insertionOffset,
        const QVariant& tagTexts) const;

private:
    QString normalizedDeletionDirection(const QString& direction) const;

    ContentsStructuredDocumentCollectionPolicy* m_collectionPolicy = nullptr;
};
