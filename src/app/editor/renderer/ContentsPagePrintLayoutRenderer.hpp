#pragma once

#include <QColor>
#include <QObject>

class ContentsPagePrintLayoutRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeEditorViewMode READ activeEditorViewMode WRITE setActiveEditorViewMode NOTIFY modeStateChanged)
    Q_PROPERTY(bool hasSelectedNote READ hasSelectedNote WRITE setHasSelectedNote NOTIFY modeStateChanged)
    Q_PROPERTY(bool dedicatedResourceViewerVisible READ dedicatedResourceViewerVisible WRITE setDedicatedResourceViewerVisible NOTIFY modeStateChanged)

    Q_PROPERTY(qreal editorViewportWidth READ editorViewportWidth WRITE setEditorViewportWidth NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal editorViewportHeight READ editorViewportHeight WRITE setEditorViewportHeight NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal editorContentHeight READ editorContentHeight WRITE setEditorContentHeight NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal guideHorizontalInset READ guideHorizontalInset WRITE setGuideHorizontalInset NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal guideVerticalInset READ guideVerticalInset WRITE setGuideVerticalInset NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperAspectRatio READ paperAspectRatio WRITE setPaperAspectRatio NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperHorizontalMargin READ paperHorizontalMargin WRITE setPaperHorizontalMargin NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperVerticalMargin READ paperVerticalMargin WRITE setPaperVerticalMargin NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperMaxWidth READ paperMaxWidth WRITE setPaperMaxWidth NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperSeparatorThickness READ paperSeparatorThickness WRITE setPaperSeparatorThickness NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperShadowOffsetX READ paperShadowOffsetX WRITE setPaperShadowOffsetX NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperShadowOffsetY READ paperShadowOffsetY WRITE setPaperShadowOffsetY NOTIFY layoutStateChanged)

    Q_PROPERTY(int pageViewModeValue READ pageViewModeValue CONSTANT)
    Q_PROPERTY(int printViewModeValue READ printViewModeValue CONSTANT)

    Q_PROPERTY(bool showPageEditorLayout READ showPageEditorLayout NOTIFY modeStateChanged)
    Q_PROPERTY(bool showPrintModeActive READ showPrintModeActive NOTIFY modeStateChanged)
    Q_PROPERTY(bool showPrintEditorLayout READ showPrintEditorLayout NOTIFY modeStateChanged)
    Q_PROPERTY(bool showPrintMarginGuides READ showPrintMarginGuides NOTIFY modeStateChanged)

    Q_PROPERTY(int documentPageCount READ documentPageCount NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperResolvedWidth READ paperResolvedWidth NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperResolvedHeight READ paperResolvedHeight NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperTextWidth READ paperTextWidth NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperTextHeight READ paperTextHeight NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal paperDocumentHeight READ paperDocumentHeight NOTIFY layoutStateChanged)
    Q_PROPERTY(qreal documentSurfaceHeight READ documentSurfaceHeight NOTIFY layoutStateChanged)

    Q_PROPERTY(QColor canvasColor READ canvasColor CONSTANT)
    Q_PROPERTY(QColor paperBorderColor READ paperBorderColor CONSTANT)
    Q_PROPERTY(QColor paperColor READ paperColor CONSTANT)
    Q_PROPERTY(QColor paperHighlightColor READ paperHighlightColor CONSTANT)
    Q_PROPERTY(QColor paperShadeColor READ paperShadeColor CONSTANT)
    Q_PROPERTY(QColor paperSeparatorColor READ paperSeparatorColor CONSTANT)
    Q_PROPERTY(QColor paperShadowColor READ paperShadowColor CONSTANT)
    Q_PROPERTY(QColor paperTextColor READ paperTextColor CONSTANT)

public:
    explicit ContentsPagePrintLayoutRenderer(QObject* parent = nullptr);
    ~ContentsPagePrintLayoutRenderer() override;

    int activeEditorViewMode() const noexcept;
    void setActiveEditorViewMode(int activeEditorViewMode);

    bool hasSelectedNote() const noexcept;
    void setHasSelectedNote(bool hasSelectedNote);

    bool dedicatedResourceViewerVisible() const noexcept;
    void setDedicatedResourceViewerVisible(bool dedicatedResourceViewerVisible);

    qreal editorViewportWidth() const noexcept;
    void setEditorViewportWidth(qreal editorViewportWidth);

    qreal editorViewportHeight() const noexcept;
    void setEditorViewportHeight(qreal editorViewportHeight);

    qreal editorContentHeight() const noexcept;
    void setEditorContentHeight(qreal editorContentHeight);

    qreal guideHorizontalInset() const noexcept;
    void setGuideHorizontalInset(qreal guideHorizontalInset);

    qreal guideVerticalInset() const noexcept;
    void setGuideVerticalInset(qreal guideVerticalInset);

    qreal paperAspectRatio() const noexcept;
    void setPaperAspectRatio(qreal paperAspectRatio);

    qreal paperHorizontalMargin() const noexcept;
    void setPaperHorizontalMargin(qreal paperHorizontalMargin);

    qreal paperVerticalMargin() const noexcept;
    void setPaperVerticalMargin(qreal paperVerticalMargin);

    qreal paperMaxWidth() const noexcept;
    void setPaperMaxWidth(qreal paperMaxWidth);

    qreal paperSeparatorThickness() const noexcept;
    void setPaperSeparatorThickness(qreal paperSeparatorThickness);

    qreal paperShadowOffsetX() const noexcept;
    void setPaperShadowOffsetX(qreal paperShadowOffsetX);

    qreal paperShadowOffsetY() const noexcept;
    void setPaperShadowOffsetY(qreal paperShadowOffsetY);

    int pageViewModeValue() const noexcept;
    int printViewModeValue() const noexcept;

    bool showPageEditorLayout() const noexcept;
    bool showPrintModeActive() const noexcept;
    bool showPrintEditorLayout() const noexcept;
    bool showPrintMarginGuides() const noexcept;

    int documentPageCount() const noexcept;
    qreal paperResolvedWidth() const noexcept;
    qreal paperResolvedHeight() const noexcept;
    qreal paperTextWidth() const noexcept;
    qreal paperTextHeight() const noexcept;
    qreal paperDocumentHeight() const noexcept;
    qreal documentSurfaceHeight() const noexcept;

    QColor canvasColor() const;
    QColor paperBorderColor() const;
    QColor paperColor() const;
    QColor paperHighlightColor() const;
    QColor paperShadeColor() const;
    QColor paperSeparatorColor() const;
    QColor paperShadowColor() const;
    QColor paperTextColor() const;

public slots:
    void requestRefresh();

signals:
    void modeStateChanged();
    void layoutStateChanged();

private:
    static qreal sanitizeNonNegative(qreal value) noexcept;

    int m_activeEditorViewMode = 0;
    bool m_hasSelectedNote = false;
    bool m_dedicatedResourceViewerVisible = false;

    qreal m_editorViewportWidth = 0.0;
    qreal m_editorViewportHeight = 0.0;
    qreal m_editorContentHeight = 0.0;

    qreal m_guideHorizontalInset = 24.0;
    qreal m_guideVerticalInset = 24.0;
    qreal m_paperAspectRatio = 210.0 / 297.0;
    qreal m_paperHorizontalMargin = 12.0;
    qreal m_paperVerticalMargin = 4.0;
    qreal m_paperMaxWidth = 880.0;
    qreal m_paperSeparatorThickness = 1.0;
    qreal m_paperShadowOffsetX = 1.0;
    qreal m_paperShadowOffsetY = 2.0;
};
