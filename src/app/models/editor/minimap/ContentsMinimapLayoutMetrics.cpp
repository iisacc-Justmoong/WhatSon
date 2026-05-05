#include "app/models/editor/minimap/ContentsMinimapLayoutMetrics.hpp"

#include <algorithm>

ContentsMinimapLayoutMetrics::ContentsMinimapLayoutMetrics(QObject* parent)
    : QObject(parent)
{
}

ContentsMinimapLayoutMetrics::~ContentsMinimapLayoutMetrics() = default;

int ContentsMinimapLayoutMetrics::gapNone() const noexcept
{
    return m_gapNone;
}

void ContentsMinimapLayoutMetrics::setGapNone(const int value)
{
    if (m_gapNone == value)
    {
        return;
    }

    m_gapNone = value;
    emitChanged(&ContentsMinimapLayoutMetrics::gapNoneChanged);
}

int ContentsMinimapLayoutMetrics::gap8() const noexcept
{
    return m_gap8;
}

void ContentsMinimapLayoutMetrics::setGap8(const int value)
{
    if (m_gap8 == value)
    {
        return;
    }

    m_gap8 = value;
    emitChanged(&ContentsMinimapLayoutMetrics::gap8Changed);
}

int ContentsMinimapLayoutMetrics::gap12() const noexcept
{
    return m_gap12;
}

void ContentsMinimapLayoutMetrics::setGap12(const int value)
{
    if (m_gap12 == value)
    {
        return;
    }

    m_gap12 = value;
    emitChanged(&ContentsMinimapLayoutMetrics::gap12Changed);
}

int ContentsMinimapLayoutMetrics::gap20() const noexcept
{
    return m_gap20;
}

void ContentsMinimapLayoutMetrics::setGap20(const int value)
{
    if (m_gap20 == value)
    {
        return;
    }

    m_gap20 = value;
    emitChanged(&ContentsMinimapLayoutMetrics::gap20Changed);
}

int ContentsMinimapLayoutMetrics::gap24() const noexcept
{
    return m_gap24;
}

void ContentsMinimapLayoutMetrics::setGap24(const int value)
{
    if (m_gap24 == value)
    {
        return;
    }

    m_gap24 = value;
    emitChanged(&ContentsMinimapLayoutMetrics::gap24Changed);
}

int ContentsMinimapLayoutMetrics::strokeThin() const noexcept
{
    return m_strokeThin;
}

void ContentsMinimapLayoutMetrics::setStrokeThin(const int value)
{
    if (m_strokeThin == value)
    {
        return;
    }

    m_strokeThin = value;
    emitChanged(&ContentsMinimapLayoutMetrics::strokeThinChanged);
}

int ContentsMinimapLayoutMetrics::buttonMinWidth() const noexcept
{
    return m_buttonMinWidth;
}

void ContentsMinimapLayoutMetrics::setButtonMinWidth(const int value)
{
    if (m_buttonMinWidth == value)
    {
        return;
    }

    m_buttonMinWidth = value;
    emitChanged(&ContentsMinimapLayoutMetrics::buttonMinWidthChanged);
}

int ContentsMinimapLayoutMetrics::visualLineCount() const noexcept
{
    return m_visualLineCount;
}

void ContentsMinimapLayoutMetrics::setVisualLineCount(const int value)
{
    if (m_visualLineCount == value)
    {
        return;
    }

    m_visualLineCount = value;
    emitChanged(&ContentsMinimapLayoutMetrics::visualLineCountChanged);
}

bool ContentsMinimapLayoutMetrics::minimapVisible() const noexcept
{
    return m_minimapVisible;
}

void ContentsMinimapLayoutMetrics::setMinimapVisible(const bool value)
{
    if (m_minimapVisible == value)
    {
        return;
    }

    m_minimapVisible = value;
    emitChanged(&ContentsMinimapLayoutMetrics::minimapVisibleChanged);
}

int ContentsMinimapLayoutMetrics::defaultMinimapWidth() const noexcept
{
    return m_buttonMinWidth;
}

int ContentsMinimapLayoutMetrics::designRowCount() const noexcept
{
    return std::max(minimumMetricUnit(), m_gap24 + m_gap20 + m_gap12 + m_gap8);
}

int ContentsMinimapLayoutMetrics::effectiveMinimapWidth() const noexcept
{
    if (!m_minimapVisible)
    {
        return m_gapNone;
    }

    return defaultMinimapWidth();
}

int ContentsMinimapLayoutMetrics::effectiveRowCount() const noexcept
{
    return std::max(minimumMetricUnit(), m_visualLineCount);
}

void ContentsMinimapLayoutMetrics::requestMetricsRefresh()
{
    emit metricsChanged();
}

int ContentsMinimapLayoutMetrics::minimumMetricUnit() const noexcept
{
    return std::max(1, m_strokeThin);
}

void ContentsMinimapLayoutMetrics::emitChanged(void (ContentsMinimapLayoutMetrics::*signal)())
{
    emit (this->*signal)();
    emit metricsChanged();
}
