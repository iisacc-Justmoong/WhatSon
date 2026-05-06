#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsLogicalTextBridge;
class ContentsTextFormatRenderer;

class ContentsEditorPresentationProjection : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged)
    Q_PROPERTY(bool paperPaletteEnabled READ paperPaletteEnabled WRITE setPaperPaletteEnabled NOTIFY paperPaletteEnabledChanged)
    Q_PROPERTY(QString editorSurfaceHtml READ editorSurfaceHtml NOTIFY editorSurfaceHtmlChanged)
    Q_PROPERTY(QString renderedHtml READ renderedHtml NOTIFY renderedHtmlChanged)
    Q_PROPERTY(QVariantList htmlTokens READ htmlTokens NOTIFY htmlTokensChanged)
    Q_PROPERTY(QVariantList normalizedHtmlBlocks READ normalizedHtmlBlocks NOTIFY normalizedHtmlBlocksChanged)
    Q_PROPERTY(QString logicalText READ logicalText NOTIFY logicalTextChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(int sourceCursorPosition READ sourceCursorPosition WRITE setSourceCursorPosition NOTIFY sourceCursorPositionChanged)
    Q_PROPERTY(int logicalCursorPosition READ logicalCursorPosition NOTIFY logicalCursorPositionChanged)

public:
    explicit ContentsEditorPresentationProjection(QObject* parent = nullptr);
    ~ContentsEditorPresentationProjection() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    bool previewEnabled() const noexcept;
    void setPreviewEnabled(bool enabled);
    bool paperPaletteEnabled() const noexcept;
    void setPaperPaletteEnabled(bool enabled);

    QString editorSurfaceHtml() const;
    QString renderedHtml() const;
    QVariantList htmlTokens() const;
    QVariantList normalizedHtmlBlocks() const;
    QString logicalText() const;
    int logicalLineCount() const noexcept;
    int sourceCursorPosition() const noexcept;
    void setSourceCursorPosition(int position);
    int logicalCursorPosition() const;

    Q_INVOKABLE int logicalLengthForSourceText(const QString& text) const;
    Q_INVOKABLE int logicalOffsetForSourceOffset(int sourceOffset) const;
    Q_INVOKABLE int logicalOffsetForSourceOffsetWithAffinity(int sourceOffset, bool preferAfter) const noexcept;
    Q_INVOKABLE int sourceOffsetForLogicalOffset(int logicalOffset) const noexcept;
    Q_INVOKABLE int sourceOffsetForVisibleLogicalOffset(int logicalOffset, int visibleLength) const noexcept;

signals:
    void sourceTextChanged();
    void previewEnabledChanged();
    void paperPaletteEnabledChanged();
    void editorSurfaceHtmlChanged();
    void renderedHtmlChanged();
    void htmlTokensChanged();
    void normalizedHtmlBlocksChanged();
    void logicalTextChanged();
    void logicalLineCountChanged();
    void sourceCursorPositionChanged();
    void logicalCursorPositionChanged();

private:
    void connectSignals();

    ContentsTextFormatRenderer* m_textFormatRenderer = nullptr;
    ContentsLogicalTextBridge* m_logicalTextBridge = nullptr;
    int m_sourceCursorPosition = 0;
};
