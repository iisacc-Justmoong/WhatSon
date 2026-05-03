#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsGutterLineNumberGeometry : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* editorGeometryHost READ editorGeometryHost WRITE setEditorGeometryHost NOTIFY editorGeometryHostChanged)
    Q_PROPERTY(QObject* mapTarget READ mapTarget WRITE setMapTarget NOTIFY mapTargetChanged)
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QVariantList displayBlocks READ displayBlocks WRITE setDisplayBlocks NOTIFY displayBlocksChanged)
    Q_PROPERTY(QVariantList documentBlocks READ documentBlocks WRITE setDocumentBlocks NOTIFY documentBlocksChanged)
    Q_PROPERTY(QVariantList renderedResources READ renderedResources WRITE setRenderedResources NOTIFY renderedResourcesChanged)
    Q_PROPERTY(QVariantList logicalLineStartOffsets READ logicalLineStartOffsets WRITE setLogicalLineStartOffsets NOTIFY logicalLineStartOffsetsChanged)
    Q_PROPERTY(QVariantList logicalToSourceOffsets READ logicalToSourceOffsets WRITE setLogicalToSourceOffsets NOTIFY logicalToSourceOffsetsChanged)
    Q_PROPERTY(int lineNumberCount READ lineNumberCount WRITE setLineNumberCount NOTIFY lineNumberCountChanged)
    Q_PROPERTY(int lineNumberBaseOffset READ lineNumberBaseOffset WRITE setLineNumberBaseOffset NOTIFY lineNumberBaseOffsetChanged)
    Q_PROPERTY(qreal fallbackLineHeight READ fallbackLineHeight WRITE setFallbackLineHeight NOTIFY fallbackLineHeightChanged)
    Q_PROPERTY(qreal fallbackTopInset READ fallbackTopInset WRITE setFallbackTopInset NOTIFY fallbackTopInsetChanged)
    Q_PROPERTY(qreal editorContentHeight READ editorContentHeight WRITE setEditorContentHeight NOTIFY editorContentHeightChanged)
    Q_PROPERTY(QVariantList lineNumberEntries READ lineNumberEntries NOTIFY lineNumberEntriesChanged)

public:
    struct LineSlot final
    {
        QString blockType;
        int sourceEnd = 0;
        int sourceStart = 0;
        bool resource = false;
    };

    explicit ContentsGutterLineNumberGeometry(QObject* parent = nullptr);
    ~ContentsGutterLineNumberGeometry() override;

    QObject* editorGeometryHost() const noexcept;
    void setEditorGeometryHost(QObject* value);
    QObject* mapTarget() const noexcept;
    void setMapTarget(QObject* value);
    QString sourceText() const;
    void setSourceText(const QString& value);
    QVariantList displayBlocks() const;
    void setDisplayBlocks(const QVariantList& value);
    QVariantList documentBlocks() const;
    void setDocumentBlocks(const QVariantList& value);
    QVariantList renderedResources() const;
    void setRenderedResources(const QVariantList& value);
    QVariantList logicalLineStartOffsets() const;
    void setLogicalLineStartOffsets(const QVariantList& value);
    QVariantList logicalToSourceOffsets() const;
    void setLogicalToSourceOffsets(const QVariantList& value);
    int lineNumberCount() const noexcept;
    void setLineNumberCount(int value);
    int lineNumberBaseOffset() const noexcept;
    void setLineNumberBaseOffset(int value);
    qreal fallbackLineHeight() const noexcept;
    void setFallbackLineHeight(qreal value);
    qreal fallbackTopInset() const noexcept;
    void setFallbackTopInset(qreal value);
    qreal editorContentHeight() const noexcept;
    void setEditorContentHeight(qreal value);
    QVariantList lineNumberEntries() const;

public slots:
    void refresh();

signals:
    void editorGeometryHostChanged();
    void mapTargetChanged();
    void sourceTextChanged();
    void displayBlocksChanged();
    void documentBlocksChanged();
    void renderedResourcesChanged();
    void logicalLineStartOffsetsChanged();
    void logicalToSourceOffsetsChanged();
    void lineNumberCountChanged();
    void lineNumberBaseOffsetChanged();
    void fallbackLineHeightChanged();
    void fallbackTopInsetChanged();
    void editorContentHeightChanged();
    void lineNumberEntriesChanged();

private:
    QVariantList effectiveBlockStream() const;
    QList<LineSlot> editorLineSlots() const;
    QList<int> sourceLineStartOffsets() const;
    QList<LineSlot> rawLineSlots() const;
    int logicalOffsetForSourceOffset(int sourceOffset) const noexcept;
    bool editorLineGeometryForSlot(const LineSlot& slot, int fallbackIndex, qreal* resolvedY, qreal* resolvedHeight) const;
    qreal fallbackYForIndex(int index) const noexcept;
    void rebuildLineNumberEntries();

    QObject* m_editorGeometryHost = nullptr;
    QObject* m_mapTarget = nullptr;
    QString m_sourceText;
    QVariantList m_displayBlocks;
    QVariantList m_documentBlocks;
    QVariantList m_renderedResources;
    QVariantList m_logicalLineStartOffsets;
    QVariantList m_logicalToSourceOffsets;
    int m_lineNumberCount = 1;
    int m_lineNumberBaseOffset = 1;
    qreal m_fallbackLineHeight = 1.0;
    qreal m_fallbackTopInset = 0.0;
    qreal m_editorContentHeight = 0.0;
    QVariantList m_lineNumberEntries;
};
