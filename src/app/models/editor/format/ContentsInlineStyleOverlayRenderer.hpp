#pragma once

#include <QObject>
#include <QString>

class ContentsTextFormatRenderer;

class ContentsInlineStyleOverlayRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString editorSurfaceHtml READ editorSurfaceHtml NOTIFY editorSurfaceHtmlChanged)
    Q_PROPERTY(bool htmlOverlayVisible READ htmlOverlayVisible NOTIFY htmlOverlayVisibleChanged)
    Q_PROPERTY(bool paperPaletteEnabled READ paperPaletteEnabled WRITE setPaperPaletteEnabled NOTIFY paperPaletteEnabledChanged)

public:
    explicit ContentsInlineStyleOverlayRenderer(QObject* parent = nullptr);
    ~ContentsInlineStyleOverlayRenderer() override;

    QString sourceText() const;
    void setSourceText(const QString& sourceText);
    QString editorSurfaceHtml() const;
    bool htmlOverlayVisible() const noexcept;
    bool paperPaletteEnabled() const noexcept;
    void setPaperPaletteEnabled(bool enabled);

public slots:
    void requestRenderRefresh();

signals:
    void sourceTextChanged();
    void editorSurfaceHtmlChanged();
    void htmlOverlayVisibleChanged();
    void paperPaletteEnabledChanged();

private:
    ContentsTextFormatRenderer* m_delegateRenderer = nullptr;
};
