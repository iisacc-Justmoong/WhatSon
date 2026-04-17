#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ContentsStructuredDocumentCollectionPolicy;
class ContentsStructuredDocumentFocusPolicy;
class ContentsStructuredDocumentMutationPolicy;

class ContentsStructuredDocumentHost : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant documentBlocks READ documentBlocks WRITE setDocumentBlocks NOTIFY documentBlocksChanged)
    Q_PROPERTY(QVariant renderedResources READ renderedResources WRITE setRenderedResources NOTIFY renderedResourcesChanged)
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(int activeBlockIndex READ activeBlockIndex WRITE setActiveBlockIndex NOTIFY activeBlockIndexChanged)
    Q_PROPERTY(
        int activeBlockCursorRevision READ activeBlockCursorRevision WRITE setActiveBlockCursorRevision
            NOTIFY activeBlockCursorRevisionChanged)
    Q_PROPERTY(
        QVariantMap pendingFocusRequest READ pendingFocusRequest WRITE setPendingFocusRequest
            NOTIFY pendingFocusRequestChanged)
    Q_PROPERTY(
        int pendingFocusBlockIndex READ pendingFocusBlockIndex WRITE setPendingFocusBlockIndex
            NOTIFY pendingFocusBlockIndexChanged)
    Q_PROPERTY(
        bool pendingFocusApplyQueued READ pendingFocusApplyQueued WRITE setPendingFocusApplyQueued
            NOTIFY pendingFocusApplyQueuedChanged)
    Q_PROPERTY(
        QVariantList cachedLogicalLineEntries READ cachedLogicalLineEntries WRITE setCachedLogicalLineEntries
            NOTIFY cachedLogicalLineEntriesChanged)
    Q_PROPERTY(
        QVariantList cachedBlockLayoutSummaries READ cachedBlockLayoutSummaries WRITE
            setCachedBlockLayoutSummaries NOTIFY cachedBlockLayoutSummariesChanged)
    Q_PROPERTY(
        bool layoutCacheRefreshQueued READ layoutCacheRefreshQueued WRITE setLayoutCacheRefreshQueued
            NOTIFY layoutCacheRefreshQueuedChanged)
    Q_PROPERTY(qreal viewportContentY READ viewportContentY WRITE setViewportContentY NOTIFY viewportContentYChanged)
    Q_PROPERTY(qreal viewportHeight READ viewportHeight WRITE setViewportHeight NOTIFY viewportHeightChanged)
    Q_PROPERTY(QObject* collectionPolicy READ collectionPolicy CONSTANT)
    Q_PROPERTY(QObject* focusPolicy READ focusPolicy CONSTANT)
    Q_PROPERTY(QObject* mutationPolicy READ mutationPolicy CONSTANT)

public:
    explicit ContentsStructuredDocumentHost(QObject* parent = nullptr);
    ~ContentsStructuredDocumentHost() override;

    QVariant documentBlocks() const;
    void setDocumentBlocks(const QVariant& documentBlocks);

    QVariant renderedResources() const;
    void setRenderedResources(const QVariant& renderedResources);

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    int activeBlockIndex() const noexcept;
    void setActiveBlockIndex(int activeBlockIndex);

    int activeBlockCursorRevision() const noexcept;
    void setActiveBlockCursorRevision(int activeBlockCursorRevision);

    QVariantMap pendingFocusRequest() const;
    void setPendingFocusRequest(const QVariantMap& pendingFocusRequest);

    int pendingFocusBlockIndex() const noexcept;
    void setPendingFocusBlockIndex(int pendingFocusBlockIndex);

    bool pendingFocusApplyQueued() const noexcept;
    void setPendingFocusApplyQueued(bool pendingFocusApplyQueued);

    QVariantList cachedLogicalLineEntries() const;
    void setCachedLogicalLineEntries(const QVariantList& cachedLogicalLineEntries);

    QVariantList cachedBlockLayoutSummaries() const;
    void setCachedBlockLayoutSummaries(const QVariantList& cachedBlockLayoutSummaries);

    bool layoutCacheRefreshQueued() const noexcept;
    void setLayoutCacheRefreshQueued(bool layoutCacheRefreshQueued);

    qreal viewportContentY() const noexcept;
    void setViewportContentY(qreal viewportContentY);

    qreal viewportHeight() const noexcept;
    void setViewportHeight(qreal viewportHeight);

    QObject* collectionPolicy() const noexcept;
    QObject* focusPolicy() const noexcept;
    QObject* mutationPolicy() const noexcept;

    Q_INVOKABLE int resolvedInteractiveBlockIndex(int focusedBlockIndex) const noexcept;
    Q_INVOKABLE int shortcutInsertionSourceOffset(
        int focusedBlockIndex,
        const QVariant& delegateInsertionOffset = QVariant()) const;
    Q_INVOKABLE void noteActiveBlockInteraction(int blockIndex);
    Q_INVOKABLE void clearPendingFocusRequest();

signals:
    void documentBlocksChanged();
    void renderedResourcesChanged();
    void sourceTextChanged();
    void activeBlockIndexChanged();
    void activeBlockCursorRevisionChanged();
    void pendingFocusRequestChanged();
    void pendingFocusBlockIndexChanged();
    void pendingFocusApplyQueuedChanged();
    void cachedLogicalLineEntriesChanged();
    void cachedBlockLayoutSummariesChanged();
    void layoutCacheRefreshQueuedChanged();
    void viewportContentYChanged();
    void viewportHeightChanged();

private:
    QVariant m_documentBlocks;
    QVariant m_renderedResources;
    QString m_sourceText;
    int m_activeBlockIndex = -1;
    int m_activeBlockCursorRevision = 0;
    QVariantMap m_pendingFocusRequest;
    int m_pendingFocusBlockIndex = -1;
    bool m_pendingFocusApplyQueued = false;
    QVariantList m_cachedLogicalLineEntries;
    QVariantList m_cachedBlockLayoutSummaries;
    bool m_layoutCacheRefreshQueued = false;
    qreal m_viewportContentY = 0.0;
    qreal m_viewportHeight = 0.0;
    ContentsStructuredDocumentCollectionPolicy* m_collectionPolicy = nullptr;
    ContentsStructuredDocumentFocusPolicy* m_focusPolicy = nullptr;
    ContentsStructuredDocumentMutationPolicy* m_mutationPolicy = nullptr;
};
