#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsTextFormatRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString editorSurfaceHtml READ editorSurfaceHtml NOTIFY editorSurfaceHtmlChanged)
    Q_PROPERTY(QString renderedHtml READ renderedHtml NOTIFY renderedHtmlChanged)
    Q_PROPERTY(QVariantList htmlTokens READ htmlTokens NOTIFY htmlTokensChanged)
    Q_PROPERTY(QVariantList normalizedHtmlBlocks READ normalizedHtmlBlocks NOTIFY normalizedHtmlBlocksChanged)
    Q_PROPERTY(bool htmlOverlayVisible READ htmlOverlayVisible NOTIFY htmlOverlayVisibleChanged)
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged)

public:
    explicit ContentsTextFormatRenderer(QObject* parent = nullptr);
    ~ContentsTextFormatRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);
    QString editorSurfaceHtml() const;
    QString renderedHtml() const;
    QVariantList htmlTokens() const;
    QVariantList normalizedHtmlBlocks() const;
    bool htmlOverlayVisible() const noexcept;
    bool previewEnabled() const noexcept;
    void setPreviewEnabled(bool enabled);

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

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void editorSurfaceHtmlChanged();
    void renderedHtmlChanged();
    void htmlTokensChanged();
    void normalizedHtmlBlocksChanged();
    void htmlOverlayVisibleChanged();
    void previewEnabledChanged();

private:
    void refreshRenderedOutputs();

    QString m_sourceText;
    QString m_editorSurfaceHtml;
    QString m_renderedHtml;
    QVariantList m_htmlTokens;
    QVariantList m_normalizedHtmlBlocks;
    bool m_htmlOverlayVisible = false;
    bool m_previewEnabled = false;
};
