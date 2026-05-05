#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantList>

class ContentsEditorVisualLineMetrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)
    Q_PROPERTY(int textLineCount READ textLineCount WRITE setTextLineCount NOTIFY textLineCountChanged)
    Q_PROPERTY(qreal textContentHeight READ textContentHeight WRITE setTextContentHeight NOTIFY textContentHeightChanged)
    Q_PROPERTY(qreal textWidth READ textWidth WRITE setTextWidth NOTIFY textWidthChanged)
    Q_PROPERTY(qreal fallbackWidth READ fallbackWidth WRITE setFallbackWidth NOTIFY fallbackWidthChanged)
    Q_PROPERTY(qreal lineHeight READ lineHeight WRITE setLineHeight NOTIFY lineHeightChanged)
    Q_PROPERTY(qreal strokeThin READ strokeThin WRITE setStrokeThin NOTIFY strokeThinChanged)
    Q_PROPERTY(int visualLineCount READ visualLineCount NOTIFY metricsChanged)
    Q_PROPERTY(QVariantList visualLineWidthRatios READ visualLineWidthRatios NOTIFY metricsChanged)

public:
    explicit ContentsEditorVisualLineMetrics(QObject* parent = nullptr);
    ~ContentsEditorVisualLineMetrics() override;

    QObject* textItem() const noexcept;
    void setTextItem(QObject* value);
    int textLineCount() const noexcept;
    void setTextLineCount(int value);
    qreal textContentHeight() const noexcept;
    void setTextContentHeight(qreal value);
    qreal textWidth() const noexcept;
    void setTextWidth(qreal value);
    qreal fallbackWidth() const noexcept;
    void setFallbackWidth(qreal value);
    qreal lineHeight() const noexcept;
    void setLineHeight(qreal value);
    qreal strokeThin() const noexcept;
    void setStrokeThin(qreal value);

    int visualLineCount() const;
    QVariantList visualLineWidthRatios() const;

public slots:
    void requestMetricsRefresh();

signals:
    void textItemChanged();
    void textLineCountChanged();
    void textContentHeightChanged();
    void textWidthChanged();
    void fallbackWidthChanged();
    void lineHeightChanged();
    void strokeThinChanged();
    void metricsChanged();

private:
    void emitInputChanged(void (ContentsEditorVisualLineMetrics::*signal)());
    qreal resolvedTextWidth() const noexcept;
    qreal resolvedLineHeight() const noexcept;

    QPointer<QObject> m_textItem;
    int m_textLineCount = 0;
    qreal m_textContentHeight = 0.0;
    qreal m_textWidth = 0.0;
    qreal m_fallbackWidth = 1.0;
    qreal m_lineHeight = 1.0;
    qreal m_strokeThin = 1.0;
};
