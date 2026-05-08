#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

class ContentsStructuredBlockRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QVariantList renderedDocumentBlocks READ renderedDocumentBlocks NOTIFY renderedBlocksChanged)
    Q_PROPERTY(QVariantMap structuredParseVerification READ structuredParseVerification NOTIFY structuredParseVerificationChanged)
    Q_PROPERTY(QString correctedSourceText READ correctedSourceText NOTIFY correctedSourceTextChanged)
    Q_PROPERTY(bool correctionSuggested READ correctionSuggested NOTIFY correctionSuggestedChanged)
    Q_PROPERTY(bool backgroundRefreshEnabled READ backgroundRefreshEnabled WRITE setBackgroundRefreshEnabled NOTIFY backgroundRefreshEnabledChanged)
    Q_PROPERTY(bool renderPending READ renderPending NOTIFY renderPendingChanged)
    Q_PROPERTY(QVariantMap lastRenderProfile READ lastRenderProfile NOTIFY lastRenderProfileChanged)
    Q_PROPERTY(bool hasRenderedBlocks READ hasRenderedBlocks NOTIFY renderedBlocksChanged)
    Q_PROPERTY(bool hasNonResourceRenderedBlocks READ hasNonResourceRenderedBlocks NOTIFY renderedBlocksChanged)

public:
    explicit ContentsStructuredBlockRenderer(QObject* parent = nullptr);
    ~ContentsStructuredBlockRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    QVariantList renderedDocumentBlocks() const;
    QVariantMap structuredParseVerification() const;
    QString correctedSourceText() const;
    bool correctionSuggested() const noexcept;
    bool backgroundRefreshEnabled() const noexcept;
    void setBackgroundRefreshEnabled(bool enabled);
    bool renderPending() const noexcept;
    QVariantMap lastRenderProfile() const;
    bool hasRenderedBlocks() const noexcept;
    bool hasNonResourceRenderedBlocks() const noexcept;

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void renderedBlocksChanged();
    void structuredParseVerificationChanged();
    void correctedSourceTextChanged();
    void correctionSuggestedChanged();
    void backgroundRefreshEnabledChanged();
    void renderPendingChanged();
    void lastRenderProfileChanged();
    void structuredParseVerificationReported(const QVariantMap& verification);
    void structuredCorrectionSuggested(
        const QString& sourceText,
        const QString& correctedSourceText,
        const QVariantMap& verification);
    void renderProfileReported(const QVariantMap& profile);

private:
    struct RenderResult;

    void applyRenderResult(const RenderResult& result);
    void dispatchAsyncRender();
    void handleAsyncRenderFinished(const RenderResult& result);
    void publishPlaceholderDocumentBlocks();
    void refreshRenderedBlocks();
    void updateStructuredParseVerification(const QVariantMap& verification);
    void updateCorrectedSourceText(const QString& correctedSourceText);
    void updateRenderPending(bool pending);
    void updateLastRenderProfile(const QVariantMap& profile);
    bool shouldRenderInBackground() const noexcept;

    QString m_sourceText;
    QVariantList m_renderedDocumentBlocks;
    QVariantMap m_structuredParseVerification;
    QString m_correctedSourceText;
    QVariantMap m_lastRenderProfile;
    QString m_lastCorrectionSuggestionSourceText;
    QString m_lastCorrectionSuggestionCorrectedText;
    bool m_backgroundRefreshEnabled = false;
    bool m_renderPending = false;
    bool m_renderRequestInFlight = false;
    quint64 m_activeRenderSequence = 0;
    quint64 m_nextRenderSequence = 1;
};
