#pragma once

#include "agenda/ContentsAgendaBackend.hpp"
#include "callout/ContentsCalloutBackend.hpp"

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsStructuredBlockRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QVariantList renderedAgendas READ renderedAgendas NOTIFY renderedBlocksChanged)
    Q_PROPERTY(QVariantList renderedCallouts READ renderedCallouts NOTIFY renderedBlocksChanged)
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
    int agendaCount() const noexcept;
    int calloutCount() const noexcept;
    bool hasRenderedBlocks() const noexcept;

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void renderedBlocksChanged();

private:
    void refreshRenderedBlocks();

    ContentsAgendaBackend m_agendaBackend;
    ContentsCalloutBackend m_calloutBackend;
    QString m_sourceText;
    QVariantList m_renderedAgendas;
    QVariantList m_renderedCallouts;
};
