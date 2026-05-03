#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class ContentsResourceTagController;

class ContentsInlineResourcePresentationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentEditor MEMBER m_contentEditor)
    Q_PROPERTY(QObject* editorViewport MEMBER m_editorViewport)
    Q_PROPERTY(QObject* bodyResourceRenderer MEMBER m_bodyResourceRenderer)
    Q_PROPERTY(QObject* resourceTagController MEMBER m_resourceTagController)
    Q_PROPERTY(bool showPrintEditorLayout MEMBER m_showPrintEditorLayout)
    Q_PROPERTY(qreal printPaperTextWidth MEMBER m_printPaperTextWidth)
    Q_PROPERTY(int editorHorizontalInset MEMBER m_editorHorizontalInset)
    Q_PROPERTY(int resourceEditorPlaceholderLineCount MEMBER m_resourceEditorPlaceholderLineCount)
    Q_PROPERTY(bool inlineHtmlImageRenderingEnabled MEMBER m_inlineHtmlImageRenderingEnabled)
    Q_PROPERTY(QObject* view MEMBER m_view)

public:
    explicit ContentsInlineResourcePresentationController(QObject* parent = nullptr);
    ~ContentsInlineResourcePresentationController() override;

    Q_INVOKABLE int inlineResourcePreviewWidth() const;
    Q_INVOKABLE QString resourceEntryOpenTarget(const QVariant& resourceEntry) const;
    Q_INVOKABLE QString paragraphHtml(const QString& innerHtml) const;
    Q_INVOKABLE QString inlineResourcePlaceholderHtml(int lineCount) const;
    Q_INVOKABLE QString resourcePlaceholderBlockHtml() const;
    Q_INVOKABLE QString resourceEntryFrameLabel(const QVariant& resourceEntry) const;
    Q_INVOKABLE QString resourceEntryFrameImageSource(const QVariant& resourceEntry) const;
    Q_INVOKABLE QString inlineResourceBlockHtml(const QVariant& resourceEntry) const;
    Q_INVOKABLE bool resourceEntryCanRenderInlineInHtmlProjection(const QVariant& resourceEntry) const;
    Q_INVOKABLE QString renderEditorSurfaceHtmlWithInlineResources(
        const QString& editorHtml,
        const QVariant& renderedResources = QVariant()) const;

private:
    QObject* m_contentEditor = nullptr;
    QObject* m_editorViewport = nullptr;
    QObject* m_bodyResourceRenderer = nullptr;
    QObject* m_resourceTagController = nullptr;
    bool m_showPrintEditorLayout = false;
    qreal m_printPaperTextWidth = 0.0;
    int m_editorHorizontalInset = 0;
    int m_resourceEditorPlaceholderLineCount = 1;
    bool m_inlineHtmlImageRenderingEnabled = false;
    QObject* m_view = nullptr;
};
