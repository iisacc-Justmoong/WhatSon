#pragma once

#include <QObject>
#include <QString>

class ContentsTextFormatRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString renderedHtml READ renderedHtml NOTIFY renderedHtmlChanged)

public:
    explicit ContentsTextFormatRenderer(QObject* parent = nullptr);
    ~ContentsTextFormatRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);
    QString renderedHtml() const;

    Q_INVOKABLE QString renderRichText(const QString& sourceText) const;
    Q_INVOKABLE QString normalizeInlineStyleAliasesForEditor(const QString& sourceText) const;
    Q_INVOKABLE QString normalizeEditorSurfaceTextToSource(const QString& surfaceText) const;
    Q_INVOKABLE QString applyPlainTextReplacementToSource(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& replacementText) const;
    Q_INVOKABLE QString applyInlineStyleToSelectionSource(
        const QString& surfaceText,
        int selectionStart,
        int selectionEnd,
        const QString& styleTag) const;

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void renderedHtmlChanged();

private:
    void refreshRenderedHtml();

    QString m_sourceText;
    QString m_renderedHtml;
};
