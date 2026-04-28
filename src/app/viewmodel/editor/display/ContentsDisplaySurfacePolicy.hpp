#pragma once

#include <QObject>
#include <QString>

class ContentsDisplaySurfacePolicy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasSelectedNote READ hasSelectedNote WRITE setHasSelectedNote NOTIFY inputsChanged)
    Q_PROPERTY(bool resourceViewerRequested READ resourceViewerRequested WRITE setResourceViewerRequested NOTIFY inputsChanged)
    Q_PROPERTY(bool formattedPreviewRequested READ formattedPreviewRequested WRITE setFormattedPreviewRequested NOTIFY inputsChanged)
    Q_PROPERTY(bool structuredDocumentSurfaceRequested READ structuredDocumentSurfaceRequested NOTIFY outputsChanged)
    Q_PROPERTY(bool structuredDocumentFlowVisible READ structuredDocumentFlowVisible NOTIFY outputsChanged)
    Q_PROPERTY(bool dedicatedResourceViewerVisible READ dedicatedResourceViewerVisible NOTIFY outputsChanged)
    Q_PROPERTY(bool formattedTextRendererVisible READ formattedTextRendererVisible NOTIFY outputsChanged)
    Q_PROPERTY(bool documentPresentationProjectionEnabled READ documentPresentationProjectionEnabled NOTIFY outputsChanged)
    Q_PROPERTY(bool inlineHtmlImageRenderingEnabled READ inlineHtmlImageRenderingEnabled NOTIFY outputsChanged)
    Q_PROPERTY(bool resourceBlocksRenderedInlineByHtmlProjection READ resourceBlocksRenderedInlineByHtmlProjection NOTIFY outputsChanged)
    Q_PROPERTY(bool nativeInputPriority READ nativeInputPriority NOTIFY outputsChanged)
    Q_PROPERTY(QString activeSurfaceKind READ activeSurfaceKind NOTIFY outputsChanged)

public:
    explicit ContentsDisplaySurfacePolicy(QObject* parent = nullptr);
    ~ContentsDisplaySurfacePolicy() override;

    [[nodiscard]] bool hasSelectedNote() const noexcept;
    [[nodiscard]] bool resourceViewerRequested() const noexcept;
    [[nodiscard]] bool formattedPreviewRequested() const noexcept;

    [[nodiscard]] bool structuredDocumentSurfaceRequested() const noexcept;
    [[nodiscard]] bool structuredDocumentFlowVisible() const noexcept;
    [[nodiscard]] bool dedicatedResourceViewerVisible() const noexcept;
    [[nodiscard]] bool formattedTextRendererVisible() const noexcept;
    [[nodiscard]] bool documentPresentationProjectionEnabled() const noexcept;
    [[nodiscard]] bool inlineHtmlImageRenderingEnabled() const noexcept;
    [[nodiscard]] bool resourceBlocksRenderedInlineByHtmlProjection() const noexcept;
    [[nodiscard]] bool nativeInputPriority() const noexcept;
    [[nodiscard]] QString activeSurfaceKind() const;

public slots:
    void setHasSelectedNote(bool selected);
    void setResourceViewerRequested(bool requested);
    void setFormattedPreviewRequested(bool requested);

signals:
    void inputsChanged();
    void outputsChanged();

private:
    void emitPolicyChanged();
    [[nodiscard]] bool structuredSurfaceActive() const noexcept;

    bool m_hasSelectedNote = false;
    bool m_resourceViewerRequested = false;
    bool m_formattedPreviewRequested = false;
};
