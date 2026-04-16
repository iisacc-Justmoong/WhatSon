#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ContentsStructuredDocumentCollectionPolicy;

class ContentsStructuredDocumentFocusPolicy : public QObject
{
    Q_OBJECT

public:
    explicit ContentsStructuredDocumentFocusPolicy(QObject* parent = nullptr);
    ~ContentsStructuredDocumentFocusPolicy() override;

    Q_INVOKABLE int focusTargetBlockIndex(
        const QVariant& rawBlocks,
        int activeBlockIndex,
        const QVariantMap& request) const;
    Q_INVOKABLE QVariantMap focusRequestAfterBlockDeletion(
        const QVariant& rawBlocks,
        int activeBlockIndex,
        const QVariantMap& blockData,
        const QString& nextSourceText) const;

private:
    QVariantList normalizedBlocks(const QVariant& rawBlocks) const;
    int normalizedFocusTaskOpenTagStart(const QVariantMap& request) const;
    int normalizedFocusTargetBlockIndex(const QVariantMap& request) const;
    int normalizedFocusSourceOffset(const QVariantMap& request) const;
    bool requestPrefersNearestTextBlock(const QVariantMap& request) const;
    bool blockContainsTaskOpenTagStart(const QVariantMap& blockEntry, int taskOpenTagStart) const;
    bool blockUsesExclusiveTrailingBoundary(const QVariantMap& blockEntry) const;
    bool blockContainsSourceOffset(const QVariantMap& blockEntry, int sourceOffset) const;
    bool blockTextEditable(const QVariantMap& blockEntry) const;
    int blockIndexForEntry(
        const QVariantList& blocks,
        int activeBlockIndex,
        const QVariantMap& blockData) const;
    int previousEditableBlockFocusSourceOffset(const QVariantList& blocks, int blockIndex) const;
    int nextEditableBlockFocusSourceOffset(const QVariantList& blocks, int blockIndex) const;

    ContentsStructuredDocumentCollectionPolicy* m_collectionPolicy = nullptr;
};
