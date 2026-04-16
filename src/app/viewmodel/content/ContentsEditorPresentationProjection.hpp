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
    Q_PROPERTY(QString editorSurfaceHtml READ editorSurfaceHtml NOTIFY editorSurfaceHtmlChanged)
    Q_PROPERTY(QString richTextSurfaceHtml READ richTextSurfaceHtml WRITE setRichTextSurfaceHtml NOTIFY richTextSurfaceHtmlChanged)
    Q_PROPERTY(QString renderedHtml READ renderedHtml NOTIFY renderedHtmlChanged)
    Q_PROPERTY(QString logicalText READ logicalText NOTIFY logicalTextChanged)
    Q_PROPERTY(QVariantList logicalLineStartOffsets READ logicalLineStartOffsets NOTIFY logicalLineStartOffsetsChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount NOTIFY logicalLineCountChanged)

public:
    explicit ContentsEditorPresentationProjection(QObject* parent = nullptr);
    ~ContentsEditorPresentationProjection() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);

    bool previewEnabled() const noexcept;
    void setPreviewEnabled(bool enabled);

    QString editorSurfaceHtml() const;
    QString richTextSurfaceHtml() const;
    QString renderedHtml() const;
    QString logicalText() const;
    QVariantList logicalLineStartOffsets() const;
    int logicalLineCount() const noexcept;

    Q_INVOKABLE QString renderRichText(const QString& sourceText) const;
    Q_INVOKABLE QString normalizeInlineStyleAliasesForEditor(const QString& sourceText) const;
    Q_INVOKABLE QString plainTextFromEditorSurfaceHtml(const QString& richTextHtml) const;
    Q_INVOKABLE QString applyPlainTextReplacementToSource(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& replacementText) const;
    Q_INVOKABLE QString applyInlineStyleToLogicalSelectionSource(
        const QString& sourceText,
        int selectionStart,
        int selectionEnd,
        const QString& styleTag) const;
    Q_INVOKABLE int logicalLineNumberForOffset(int offset) const noexcept;
    Q_INVOKABLE int logicalLineStartOffsetAt(int index) const noexcept;
    Q_INVOKABLE int logicalLineCharacterCountAt(int index) const noexcept;
    Q_INVOKABLE int logicalLengthForSourceText(const QString& text) const;
    Q_INVOKABLE QVariantList logicalToSourceOffsets() const;
    Q_INVOKABLE int sourceOffsetForLogicalOffset(int logicalOffset) const noexcept;
    Q_INVOKABLE void adoptIncrementalState(
        const QString& sourceText,
        const QString& logicalText,
        const QVariantList& logicalLineStartOffsets,
        const QVariantList& logicalToSourceOffsets);
    Q_INVOKABLE void setRichTextSurfaceHtml(const QString& richTextSurfaceHtml);
    Q_INVOKABLE void clearRichTextSurfaceHtml();

signals:
    void sourceTextChanged();
    void previewEnabledChanged();
    void editorSurfaceHtmlChanged();
    void richTextSurfaceHtmlChanged();
    void renderedHtmlChanged();
    void logicalTextChanged();
    void logicalLineStartOffsetsChanged();
    void logicalLineCountChanged();

private:
    void connectSignals();

    ContentsTextFormatRenderer* m_textFormatRenderer = nullptr;
    ContentsLogicalTextBridge* m_logicalTextBridge = nullptr;
    QString m_richTextSurfaceHtmlOverride;
    bool m_hasRichTextSurfaceHtmlOverride = false;
};
