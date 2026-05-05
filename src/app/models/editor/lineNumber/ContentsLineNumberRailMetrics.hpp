#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantList>

class QQuickItem;

class ContentsLineNumberRailMetrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString logicalText READ logicalText WRITE setLogicalText NOTIFY logicalTextChanged)
    Q_PROPERTY(QVariantList normalizedHtmlBlocks READ normalizedHtmlBlocks WRITE setNormalizedHtmlBlocks NOTIFY normalizedHtmlBlocksChanged)
    Q_PROPERTY(QVariantList logicalToSourceOffsets READ logicalToSourceOffsets WRITE setLogicalToSourceOffsets NOTIFY logicalToSourceOffsetsChanged)
    Q_PROPERTY(QObject* textGeometryItem READ textGeometryItem WRITE setTextGeometryItem NOTIFY textGeometryItemChanged)
    Q_PROPERTY(QObject* resourceGeometryItem READ resourceGeometryItem WRITE setResourceGeometryItem NOTIFY resourceGeometryItemChanged)
    Q_PROPERTY(QObject* targetItem READ targetItem WRITE setTargetItem NOTIFY targetItemChanged)
    Q_PROPERTY(qreal textLineHeight READ textLineHeight WRITE setTextLineHeight NOTIFY textLineHeightChanged)
    Q_PROPERTY(qreal geometryWidth READ geometryWidth WRITE setGeometryWidth NOTIFY geometryWidthChanged)
    Q_PROPERTY(qreal displayContentHeight READ displayContentHeight WRITE setDisplayContentHeight NOTIFY displayContentHeightChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)

public:
    explicit ContentsLineNumberRailMetrics(QObject* parent = nullptr);
    ~ContentsLineNumberRailMetrics() override;

    QString sourceText() const;
    void setSourceText(const QString& value);
    QString logicalText() const;
    void setLogicalText(const QString& value);
    QVariantList normalizedHtmlBlocks() const;
    void setNormalizedHtmlBlocks(const QVariantList& value);
    QVariantList logicalToSourceOffsets() const;
    void setLogicalToSourceOffsets(const QVariantList& value);
    QObject* textGeometryItem() const noexcept;
    void setTextGeometryItem(QObject* value);
    QObject* resourceGeometryItem() const noexcept;
    void setResourceGeometryItem(QObject* value);
    QObject* targetItem() const noexcept;
    void setTargetItem(QObject* value);
    qreal textLineHeight() const noexcept;
    void setTextLineHeight(qreal value);
    qreal geometryWidth() const noexcept;
    void setGeometryWidth(qreal value);
    qreal displayContentHeight() const noexcept;
    void setDisplayContentHeight(qreal value);
    QVariantList rows() const;

    Q_INVOKABLE QVariantList logicalLineRanges() const;

public slots:
    void requestRowsRefresh();

signals:
    void sourceTextChanged();
    void logicalTextChanged();
    void normalizedHtmlBlocksChanged();
    void logicalToSourceOffsetsChanged();
    void textGeometryItemChanged();
    void resourceGeometryItemChanged();
    void targetItemChanged();
    void textLineHeightChanged();
    void geometryWidthChanged();
    void displayContentHeightChanged();
    void rowsChanged();

private:
    void emitInputChanged(void (ContentsLineNumberRailMetrics::*signal)());

    QString m_sourceText;
    QString m_logicalText;
    QVariantList m_normalizedHtmlBlocks;
    QVariantList m_logicalToSourceOffsets;
    QPointer<QObject> m_textGeometryItem;
    QPointer<QObject> m_resourceGeometryItem;
    QPointer<QObject> m_targetItem;
    qreal m_textLineHeight = 1.0;
    qreal m_geometryWidth = 0.0;
    qreal m_displayContentHeight = 0.0;
};
