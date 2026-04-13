#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

class ContentsStructuredBlockRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QVariantList renderedAgendas READ renderedAgendas NOTIFY renderedBlocksChanged)
    Q_PROPERTY(QVariantList renderedCallouts READ renderedCallouts NOTIFY renderedBlocksChanged)
    Q_PROPERTY(QVariantList renderedDocumentBlocks READ renderedDocumentBlocks NOTIFY renderedBlocksChanged)
    Q_PROPERTY(QVariantMap agendaParseVerification READ agendaParseVerification NOTIFY agendaParseVerificationChanged)
    Q_PROPERTY(QVariantMap calloutParseVerification READ calloutParseVerification NOTIFY calloutParseVerificationChanged)
    Q_PROPERTY(QVariantMap structuredParseVerification READ structuredParseVerification NOTIFY structuredParseVerificationChanged)
    Q_PROPERTY(QString correctedSourceText READ correctedSourceText NOTIFY correctedSourceTextChanged)
    Q_PROPERTY(bool correctionSuggested READ correctionSuggested NOTIFY correctionSuggestedChanged)
    Q_PROPERTY(bool backgroundRefreshEnabled READ backgroundRefreshEnabled WRITE setBackgroundRefreshEnabled NOTIFY backgroundRefreshEnabledChanged)
    Q_PROPERTY(bool renderPending READ renderPending NOTIFY renderPendingChanged)
    Q_PROPERTY(int agendaCount READ agendaCount NOTIFY renderedBlocksChanged)
    Q_PROPERTY(int calloutCount READ calloutCount NOTIFY renderedBlocksChanged)
    Q_PROPERTY(bool hasRenderedBlocks READ hasRenderedBlocks NOTIFY renderedBlocksChanged)
    Q_PROPERTY(bool hasNonResourceRenderedBlocks READ hasNonResourceRenderedBlocks NOTIFY renderedBlocksChanged)

public:
    explicit ContentsStructuredBlockRenderer(QObject* parent = nullptr);
    ~ContentsStructuredBlockRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    QVariantList renderedAgendas() const;
    QVariantList renderedCallouts() const;
    QVariantList renderedDocumentBlocks() const;
    QVariantMap agendaParseVerification() const;
    QVariantMap calloutParseVerification() const;
    QVariantMap structuredParseVerification() const;
    QString correctedSourceText() const;
    bool correctionSuggested() const noexcept;
    bool backgroundRefreshEnabled() const noexcept;
    void setBackgroundRefreshEnabled(bool enabled);
    bool renderPending() const noexcept;
    int agendaCount() const noexcept;
    int calloutCount() const noexcept;
    bool hasRenderedBlocks() const noexcept;
    bool hasNonResourceRenderedBlocks() const noexcept;

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void renderedBlocksChanged();
    void agendaParseVerificationChanged();
    void calloutParseVerificationChanged();
    void structuredParseVerificationChanged();
    void correctedSourceTextChanged();
    void correctionSuggestedChanged();
    void backgroundRefreshEnabledChanged();
    void renderPendingChanged();
    void agendaParseVerificationReported(const QVariantMap& verification);
    void calloutParseVerificationReported(const QVariantMap& verification);
    void structuredParseVerificationReported(const QVariantMap& verification);
    void structuredCorrectionSuggested(
        const QString& sourceText,
        const QString& correctedSourceText,
        const QVariantMap& verification);

private:
    struct RenderResult;

    void applyRenderResult(const RenderResult& result);
    void dispatchAsyncRender();
    void handleAsyncRenderFinished(const RenderResult& result);
    void publishPlaceholderDocumentBlocks();
    void refreshRenderedBlocks();
    void updateAgendaParseVerification(const QVariantMap& verification);
    void updateCalloutParseVerification(const QVariantMap& verification);
    void updateStructuredParseVerification(const QVariantMap& verification);
    void updateCorrectedSourceText(const QString& correctedSourceText);
    void updateRenderPending(bool pending);
    bool shouldRenderInBackground() const noexcept;

    QString m_sourceText;
    QVariantList m_renderedAgendas;
    QVariantList m_renderedCallouts;
    QVariantList m_renderedDocumentBlocks;
    QVariantMap m_agendaParseVerification;
    QVariantMap m_calloutParseVerification;
    QVariantMap m_structuredParseVerification;
    QString m_correctedSourceText;
    QString m_lastCorrectionSuggestionSourceText;
    QString m_lastCorrectionSuggestionCorrectedText;
    bool m_backgroundRefreshEnabled = false;
    bool m_renderPending = false;
    bool m_renderRequestInFlight = false;
    quint64 m_activeRenderSequence = 0;
    quint64 m_nextRenderSequence = 1;
};
