#include "app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"

#include "app/models/display/paper/ContentsA4PaperBackground.hpp"
#include "app/models/navigationbar/EditorViewState.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QString>

namespace
{
    constexpr qreal kEpsilon = 0.0001;
    constexpr qreal kDefaultPaperAspectRatio = ContentsA4PaperBackground::aspectRatioValue();

    bool nearlyEqual(const qreal lhs, const qreal rhs) noexcept
    {
        return std::fabs(lhs - rhs) <= kEpsilon;
    }

    qreal sanitizePositiveOrDefault(const qreal value, const qreal fallback) noexcept
    {
        if (!std::isfinite(value) || value <= 0.0)
        {
            return fallback;
        }
        return value;
    }
} // namespace

ContentsPagePrintLayoutRenderer::ContentsPagePrintLayoutRenderer(QObject* parent)
    : QObject(parent)
    , m_paperAspectRatio(ContentsA4PaperBackground::aspectRatioValue())
{
}

ContentsPagePrintLayoutRenderer::~ContentsPagePrintLayoutRenderer() = default;

int ContentsPagePrintLayoutRenderer::activeEditorViewMode() const noexcept
{
    return m_activeEditorViewMode;
}

void ContentsPagePrintLayoutRenderer::setActiveEditorViewMode(const int activeEditorViewMode)
{
    if (m_activeEditorViewMode == activeEditorViewMode)
    {
        return;
    }

    m_activeEditorViewMode = activeEditorViewMode;
    emit modeStateChanged();
    emit layoutStateChanged();
}

bool ContentsPagePrintLayoutRenderer::hasSelectedNote() const noexcept
{
    return m_hasSelectedNote;
}

void ContentsPagePrintLayoutRenderer::setHasSelectedNote(const bool hasSelectedNote)
{
    if (m_hasSelectedNote == hasSelectedNote)
    {
        return;
    }

    m_hasSelectedNote = hasSelectedNote;
    emit modeStateChanged();
    emit layoutStateChanged();
}

bool ContentsPagePrintLayoutRenderer::dedicatedResourceViewerVisible() const noexcept
{
    return m_dedicatedResourceViewerVisible;
}

void ContentsPagePrintLayoutRenderer::setDedicatedResourceViewerVisible(const bool dedicatedResourceViewerVisible)
{
    if (m_dedicatedResourceViewerVisible == dedicatedResourceViewerVisible)
    {
        return;
    }

    m_dedicatedResourceViewerVisible = dedicatedResourceViewerVisible;
    emit modeStateChanged();
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::editorViewportWidth() const noexcept
{
    return m_editorViewportWidth;
}

void ContentsPagePrintLayoutRenderer::setEditorViewportWidth(const qreal editorViewportWidth)
{
    const qreal normalizedWidth = sanitizeNonNegative(editorViewportWidth);
    if (nearlyEqual(m_editorViewportWidth, normalizedWidth))
    {
        return;
    }

    m_editorViewportWidth = normalizedWidth;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::editorViewportHeight() const noexcept
{
    return m_editorViewportHeight;
}

void ContentsPagePrintLayoutRenderer::setEditorViewportHeight(const qreal editorViewportHeight)
{
    const qreal normalizedHeight = sanitizeNonNegative(editorViewportHeight);
    if (nearlyEqual(m_editorViewportHeight, normalizedHeight))
    {
        return;
    }

    m_editorViewportHeight = normalizedHeight;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::editorContentHeight() const noexcept
{
    return m_editorContentHeight;
}

void ContentsPagePrintLayoutRenderer::setEditorContentHeight(const qreal editorContentHeight)
{
    const qreal normalizedHeight = sanitizeNonNegative(editorContentHeight);
    if (nearlyEqual(m_editorContentHeight, normalizedHeight))
    {
        return;
    }

    m_editorContentHeight = normalizedHeight;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::guideHorizontalInset() const noexcept
{
    return m_guideHorizontalInset;
}

void ContentsPagePrintLayoutRenderer::setGuideHorizontalInset(const qreal guideHorizontalInset)
{
    const qreal normalizedInset = sanitizeNonNegative(guideHorizontalInset);
    if (nearlyEqual(m_guideHorizontalInset, normalizedInset))
    {
        return;
    }

    m_guideHorizontalInset = normalizedInset;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::guideVerticalInset() const noexcept
{
    return m_guideVerticalInset;
}

void ContentsPagePrintLayoutRenderer::setGuideVerticalInset(const qreal guideVerticalInset)
{
    const qreal normalizedInset = sanitizeNonNegative(guideVerticalInset);
    if (nearlyEqual(m_guideVerticalInset, normalizedInset))
    {
        return;
    }

    m_guideVerticalInset = normalizedInset;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperAspectRatio() const noexcept
{
    return m_paperAspectRatio;
}

void ContentsPagePrintLayoutRenderer::setPaperAspectRatio(const qreal paperAspectRatio)
{
    const qreal normalizedAspectRatio = sanitizePositiveOrDefault(paperAspectRatio, kDefaultPaperAspectRatio);
    if (nearlyEqual(m_paperAspectRatio, normalizedAspectRatio))
    {
        return;
    }

    m_paperAspectRatio = normalizedAspectRatio;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperHorizontalMargin() const noexcept
{
    return m_paperHorizontalMargin;
}

void ContentsPagePrintLayoutRenderer::setPaperHorizontalMargin(const qreal paperHorizontalMargin)
{
    const qreal normalizedMargin = sanitizeNonNegative(paperHorizontalMargin);
    if (nearlyEqual(m_paperHorizontalMargin, normalizedMargin))
    {
        return;
    }

    m_paperHorizontalMargin = normalizedMargin;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperVerticalMargin() const noexcept
{
    return m_paperVerticalMargin;
}

void ContentsPagePrintLayoutRenderer::setPaperVerticalMargin(const qreal paperVerticalMargin)
{
    const qreal normalizedMargin = sanitizeNonNegative(paperVerticalMargin);
    if (nearlyEqual(m_paperVerticalMargin, normalizedMargin))
    {
        return;
    }

    m_paperVerticalMargin = normalizedMargin;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperMaxWidth() const noexcept
{
    return m_paperMaxWidth;
}

void ContentsPagePrintLayoutRenderer::setPaperMaxWidth(const qreal paperMaxWidth)
{
    const qreal normalizedMaxWidth = sanitizeNonNegative(paperMaxWidth);
    if (nearlyEqual(m_paperMaxWidth, normalizedMaxWidth))
    {
        return;
    }

    m_paperMaxWidth = normalizedMaxWidth;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperSeparatorThickness() const noexcept
{
    return m_paperSeparatorThickness;
}

void ContentsPagePrintLayoutRenderer::setPaperSeparatorThickness(const qreal paperSeparatorThickness)
{
    const qreal normalizedThickness = sanitizePositiveOrDefault(paperSeparatorThickness, 1.0);
    if (nearlyEqual(m_paperSeparatorThickness, normalizedThickness))
    {
        return;
    }

    m_paperSeparatorThickness = normalizedThickness;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperShadowOffsetX() const noexcept
{
    return m_paperShadowOffsetX;
}

void ContentsPagePrintLayoutRenderer::setPaperShadowOffsetX(const qreal paperShadowOffsetX)
{
    const qreal normalizedOffsetX = sanitizeNonNegative(paperShadowOffsetX);
    if (nearlyEqual(m_paperShadowOffsetX, normalizedOffsetX))
    {
        return;
    }

    m_paperShadowOffsetX = normalizedOffsetX;
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::paperShadowOffsetY() const noexcept
{
    return m_paperShadowOffsetY;
}

void ContentsPagePrintLayoutRenderer::setPaperShadowOffsetY(const qreal paperShadowOffsetY)
{
    const qreal normalizedOffsetY = sanitizeNonNegative(paperShadowOffsetY);
    if (nearlyEqual(m_paperShadowOffsetY, normalizedOffsetY))
    {
        return;
    }

    m_paperShadowOffsetY = normalizedOffsetY;
    emit layoutStateChanged();
}

int ContentsPagePrintLayoutRenderer::pageViewModeValue() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Page);
}

int ContentsPagePrintLayoutRenderer::printViewModeValue() const noexcept
{
    return WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Print);
}

bool ContentsPagePrintLayoutRenderer::showPageEditorLayout() const noexcept
{
    return hasSelectedNote()
        && !dedicatedResourceViewerVisible()
        && activeEditorViewMode() == pageViewModeValue();
}

bool ContentsPagePrintLayoutRenderer::showPrintModeActive() const noexcept
{
    return hasSelectedNote()
        && !dedicatedResourceViewerVisible()
        && activeEditorViewMode() == printViewModeValue();
}

bool ContentsPagePrintLayoutRenderer::showPrintEditorLayout() const noexcept
{
    return showPageEditorLayout() || showPrintModeActive();
}

bool ContentsPagePrintLayoutRenderer::showPrintMarginGuides() const noexcept
{
    return showPrintModeActive();
}

int ContentsPagePrintLayoutRenderer::documentPageCount() const noexcept
{
    if (!showPrintEditorLayout())
    {
        return 1;
    }

    const qreal pageTextHeight = std::max<qreal>(1.0, paperTextHeight());
    const qreal requiredHeight = std::max<qreal>(pageTextHeight, editorContentHeight());
    const qreal rawPageCount = std::ceil(requiredHeight / pageTextHeight);
    if (!std::isfinite(rawPageCount))
    {
        return 1;
    }

    const qreal maxIntAsReal = static_cast<qreal>(std::numeric_limits<int>::max());
    const qreal boundedPageCount = std::clamp(rawPageCount, 1.0, maxIntAsReal);
    return static_cast<int>(boundedPageCount);
}

qreal ContentsPagePrintLayoutRenderer::paperResolvedWidth() const noexcept
{
    const qreal availableWidth = std::max<qreal>(0.0, editorViewportWidth() - paperHorizontalMargin() * 2.0);
    return std::max<qreal>(0.0, std::min(paperMaxWidth(), availableWidth));
}

qreal ContentsPagePrintLayoutRenderer::paperResolvedHeight() const noexcept
{
    const qreal aspectRatio = paperAspectRatio();
    if (aspectRatio <= 0.0)
    {
        return 0.0;
    }

    return paperResolvedWidth() / aspectRatio;
}

qreal ContentsPagePrintLayoutRenderer::paperTextWidth() const noexcept
{
    return std::max<qreal>(0.0, paperResolvedWidth() - guideHorizontalInset() * 2.0);
}

qreal ContentsPagePrintLayoutRenderer::paperTextHeight() const noexcept
{
    return std::max<qreal>(1.0, paperResolvedHeight() - guideVerticalInset() * 2.0);
}

qreal ContentsPagePrintLayoutRenderer::paperDocumentHeight() const noexcept
{
    return guideVerticalInset() * 2.0 + static_cast<qreal>(documentPageCount()) * paperTextHeight();
}

qreal ContentsPagePrintLayoutRenderer::documentSurfaceHeight() const noexcept
{
    return std::max<qreal>(
        editorViewportHeight(),
        paperDocumentHeight() + paperVerticalMargin() * 2.0);
}

QColor ContentsPagePrintLayoutRenderer::canvasColor() const
{
    return ContentsA4PaperBackground::canvasColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperBorderColor() const
{
    return ContentsA4PaperBackground::paperBorderColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperColor() const
{
    return ContentsA4PaperBackground::paperColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperHighlightColor() const
{
    return ContentsA4PaperBackground::paperHighlightColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperShadeColor() const
{
    return ContentsA4PaperBackground::paperShadeColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperSeparatorColor() const
{
    return ContentsA4PaperBackground::paperSeparatorColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperShadowColor() const
{
    return ContentsA4PaperBackground::paperShadowColorValue();
}

QColor ContentsPagePrintLayoutRenderer::paperTextColor() const
{
    return ContentsA4PaperBackground::paperTextColorValue();
}

void ContentsPagePrintLayoutRenderer::requestRefresh()
{
    emit modeStateChanged();
    emit layoutStateChanged();
}

qreal ContentsPagePrintLayoutRenderer::sanitizeNonNegative(const qreal value) noexcept
{
    if (!std::isfinite(value) || value < 0.0)
    {
        return 0.0;
    }
    return value;
}
