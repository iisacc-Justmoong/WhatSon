#pragma once

#include "app/models/editor/geometry/IContentsEditorGeometryProvider.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

class ContentsEditorGeometryProvider : public QObject, public IContentsEditorGeometryProvider
{
    Q_OBJECT
    Q_INTERFACES(IContentsEditorGeometryProvider)

    Q_PROPERTY(QObject* textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)
    Q_PROPERTY(QObject* resourceItem READ resourceItem WRITE setResourceItem NOTIFY resourceItemChanged)
    Q_PROPERTY(QObject* targetItem READ targetItem WRITE setTargetItem NOTIFY targetItemChanged)
    Q_PROPERTY(QObject* visualItem READ visualItem WRITE setVisualItem NOTIFY visualItemChanged)
    Q_PROPERTY(QString renderedHtml READ renderedHtml WRITE setRenderedHtml NOTIFY renderedHtmlChanged)
    Q_PROPERTY(QVariantList resourceVisualHeights READ resourceVisualHeights WRITE setResourceVisualHeights NOTIFY resourceVisualHeightsChanged)
    Q_PROPERTY(QVariantList lineNumberRanges READ lineNumberRanges WRITE setLineNumberRanges NOTIFY lineNumberRangesChanged)
    Q_PROPERTY(int logicalLength READ logicalLength WRITE setLogicalLength NOTIFY logicalLengthChanged)
    Q_PROPERTY(qreal fallbackLineHeight READ fallbackLineHeight WRITE setFallbackLineHeight NOTIFY fallbackLineHeightChanged)
    Q_PROPERTY(qreal fallbackWidth READ fallbackWidth WRITE setFallbackWidth NOTIFY fallbackWidthChanged)
    Q_PROPERTY(int visualTextLineCount READ visualTextLineCount WRITE setVisualTextLineCount NOTIFY visualTextLineCountChanged)
    Q_PROPERTY(qreal visualTextContentHeight READ visualTextContentHeight WRITE setVisualTextContentHeight NOTIFY visualTextContentHeightChanged)
    Q_PROPERTY(qreal visualTextWidth READ visualTextWidth WRITE setVisualTextWidth NOTIFY visualTextWidthChanged)
    Q_PROPERTY(qreal visualLineHeight READ visualLineHeight WRITE setVisualLineHeight NOTIFY visualLineHeightChanged)
    Q_PROPERTY(qreal visualStrokeThin READ visualStrokeThin WRITE setVisualStrokeThin NOTIFY visualStrokeThinChanged)
    Q_PROPERTY(int visualLineCount READ visualLineCount NOTIFY geometryChanged)
    Q_PROPERTY(QVariantList visualLineWidthRatios READ visualLineWidthRatios NOTIFY geometryChanged)
    Q_PROPERTY(QVariantList lineNumberGeometryRows READ lineNumberGeometryRows NOTIFY geometryChanged)

public:
    explicit ContentsEditorGeometryProvider(QObject* parent = nullptr);
    ~ContentsEditorGeometryProvider() override;

    QObject* textItem() const noexcept;
    void setTextItem(QObject* value);
    QObject* resourceItem() const noexcept;
    void setResourceItem(QObject* value);
    QObject* targetItem() const noexcept;
    void setTargetItem(QObject* value);
    QObject* visualItem() const noexcept;
    void setVisualItem(QObject* value);
    QString renderedHtml() const;
    void setRenderedHtml(const QString& value);
    QVariantList resourceVisualHeights() const;
    void setResourceVisualHeights(const QVariantList& value);
    QVariantList lineNumberRanges() const;
    void setLineNumberRanges(const QVariantList& value);
    int logicalLength() const noexcept;
    void setLogicalLength(int value);
    qreal fallbackLineHeight() const noexcept;
    void setFallbackLineHeight(qreal value);
    qreal fallbackWidth() const noexcept;
    void setFallbackWidth(qreal value);
    int visualTextLineCount() const noexcept;
    void setVisualTextLineCount(int value);
    qreal visualTextContentHeight() const noexcept;
    void setVisualTextContentHeight(qreal value);
    qreal visualTextWidth() const noexcept;
    void setVisualTextWidth(qreal value);
    qreal visualLineHeight() const noexcept;
    void setVisualLineHeight(qreal value);
    qreal visualStrokeThin() const noexcept;
    void setVisualStrokeThin(qreal value);
    int visualLineCount() const;
    QVariantList visualLineWidthRatios() const;
    QVariantList lineNumberGeometryRows() const;

    ContentsEditorGeometryMeasurement measureTextRange(
        int logicalStart,
        int logicalEnd,
        int logicalLength,
        qreal fallbackLineHeight,
        qreal fallbackWidth) const override;

    ContentsEditorGeometryMeasurement measureResourceRange(
        int logicalStart,
        int logicalEnd,
        int logicalLength,
        qreal fallbackLineHeight,
        qreal fallbackWidth) const override;

    qreal resourceContentHeight(qreal fallback) const override;

signals:
    void textItemChanged();
    void resourceItemChanged();
    void targetItemChanged();
    void visualItemChanged();
    void renderedHtmlChanged();
    void resourceVisualHeightsChanged();
    void lineNumberRangesChanged();
    void logicalLengthChanged();
    void fallbackLineHeightChanged();
    void fallbackWidthChanged();
    void visualTextLineCountChanged();
    void visualTextContentHeightChanged();
    void visualTextWidthChanged();
    void visualLineHeightChanged();
    void visualStrokeThinChanged();
    void geometryChanged();

private:
    ContentsEditorGeometryMeasurement measureRange(
        QObject* geometryObject,
        int logicalStart,
        int logicalEnd,
        int logicalLength,
        qreal fallbackLineHeight,
        qreal fallbackWidth) const;

    void emitGeometryInputChanged(void (ContentsEditorGeometryProvider::*signal)());

    QPointer<QObject> m_textItem;
    QPointer<QObject> m_resourceItem;
    QPointer<QObject> m_targetItem;
    QPointer<QObject> m_visualItem;
    QString m_renderedHtml;
    QVariantList m_resourceVisualHeights;
    QVariantList m_lineNumberRanges;
    int m_logicalLength = 0;
    qreal m_fallbackLineHeight = 1.0;
    qreal m_fallbackWidth = 1.0;
    int m_visualTextLineCount = 0;
    qreal m_visualTextContentHeight = 0.0;
    qreal m_visualTextWidth = 0.0;
    qreal m_visualLineHeight = 1.0;
    qreal m_visualStrokeThin = 1.0;
};
