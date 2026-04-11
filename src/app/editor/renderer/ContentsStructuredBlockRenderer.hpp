#pragma once

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"

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
    Q_PROPERTY(QVariantMap agendaParseVerification READ agendaParseVerification NOTIFY agendaParseVerificationChanged)
    Q_PROPERTY(QVariantMap calloutParseVerification READ calloutParseVerification NOTIFY calloutParseVerificationChanged)
    Q_PROPERTY(QVariantMap structuredParseVerification READ structuredParseVerification NOTIFY structuredParseVerificationChanged)
    Q_PROPERTY(int agendaCount READ agendaCount NOTIFY renderedBlocksChanged)
    Q_PROPERTY(int calloutCount READ calloutCount NOTIFY renderedBlocksChanged)
    Q_PROPERTY(bool hasRenderedBlocks READ hasRenderedBlocks NOTIFY renderedBlocksChanged)

public:
    explicit ContentsStructuredBlockRenderer(QObject* parent = nullptr);
    ~ContentsStructuredBlockRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    QVariantList renderedAgendas() const;
    QVariantList renderedCallouts() const;
    QVariantMap agendaParseVerification() const;
    QVariantMap calloutParseVerification() const;
    QVariantMap structuredParseVerification() const;
    int agendaCount() const noexcept;
    int calloutCount() const noexcept;
    bool hasRenderedBlocks() const noexcept;

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void renderedBlocksChanged();
    void agendaParseVerificationChanged();
    void calloutParseVerificationChanged();
    void structuredParseVerificationChanged();
    void agendaParseVerificationReported(const QVariantMap& verification);
    void calloutParseVerificationReported(const QVariantMap& verification);
    void structuredParseVerificationReported(const QVariantMap& verification);

private:
    void refreshRenderedBlocks();
    void refreshStructuredParseVerification();
    void handleAgendaParseVerificationReported(const QVariantMap& verification);
    void handleCalloutParseVerificationReported(const QVariantMap& verification);

    ContentsAgendaBackend m_agendaBackend;
    ContentsCalloutBackend m_calloutBackend;
    QString m_sourceText;
    QVariantList m_renderedAgendas;
    QVariantList m_renderedCallouts;
    QVariantMap m_agendaParseVerification;
    QVariantMap m_calloutParseVerification;
    QVariantMap m_structuredParseVerification;
};
