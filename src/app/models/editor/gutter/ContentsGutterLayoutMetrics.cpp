#include "app/models/editor/gutter/ContentsGutterLayoutMetrics.hpp"

#include <algorithm>

ContentsGutterLayoutMetrics::ContentsGutterLayoutMetrics(QObject* parent)
    : QObject(parent)
{
}

ContentsGutterLayoutMetrics::~ContentsGutterLayoutMetrics() = default;

int ContentsGutterLayoutMetrics::gapNone() const noexcept
{
    return m_gapNone;
}

void ContentsGutterLayoutMetrics::setGapNone(const int value)
{
    if (m_gapNone == value)
    {
        return;
    }

    m_gapNone = value;
    emitChanged(&ContentsGutterLayoutMetrics::gapNoneChanged);
}

int ContentsGutterLayoutMetrics::gap2() const noexcept
{
    return m_gap2;
}

void ContentsGutterLayoutMetrics::setGap2(const int value)
{
    if (m_gap2 == value)
    {
        return;
    }

    m_gap2 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap2Changed);
}

int ContentsGutterLayoutMetrics::gap3() const noexcept
{
    return m_gap3;
}

void ContentsGutterLayoutMetrics::setGap3(const int value)
{
    if (m_gap3 == value)
    {
        return;
    }

    m_gap3 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap3Changed);
}

int ContentsGutterLayoutMetrics::gap5() const noexcept
{
    return m_gap5;
}

void ContentsGutterLayoutMetrics::setGap5(const int value)
{
    if (m_gap5 == value)
    {
        return;
    }

    m_gap5 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap5Changed);
}

int ContentsGutterLayoutMetrics::gap7() const noexcept
{
    return m_gap7;
}

void ContentsGutterLayoutMetrics::setGap7(const int value)
{
    if (m_gap7 == value)
    {
        return;
    }

    m_gap7 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap7Changed);
}

int ContentsGutterLayoutMetrics::gap14() const noexcept
{
    return m_gap14;
}

void ContentsGutterLayoutMetrics::setGap14(const int value)
{
    if (m_gap14 == value)
    {
        return;
    }

    m_gap14 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap14Changed);
}

int ContentsGutterLayoutMetrics::gap20() const noexcept
{
    return m_gap20;
}

void ContentsGutterLayoutMetrics::setGap20(const int value)
{
    if (m_gap20 == value)
    {
        return;
    }

    m_gap20 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap20Changed);
}

int ContentsGutterLayoutMetrics::gap24() const noexcept
{
    return m_gap24;
}

void ContentsGutterLayoutMetrics::setGap24(const int value)
{
    if (m_gap24 == value)
    {
        return;
    }

    m_gap24 = value;
    emitChanged(&ContentsGutterLayoutMetrics::gap24Changed);
}

int ContentsGutterLayoutMetrics::strokeThin() const noexcept
{
    return m_strokeThin;
}

void ContentsGutterLayoutMetrics::setStrokeThin(const int value)
{
    if (m_strokeThin == value)
    {
        return;
    }

    m_strokeThin = value;
    emitChanged(&ContentsGutterLayoutMetrics::strokeThinChanged);
}

int ContentsGutterLayoutMetrics::controlHeightMd() const noexcept
{
    return m_controlHeightMd;
}

void ContentsGutterLayoutMetrics::setControlHeightMd(const int value)
{
    if (m_controlHeightMd == value)
    {
        return;
    }

    m_controlHeightMd = value;
    emitChanged(&ContentsGutterLayoutMetrics::controlHeightMdChanged);
}

int ContentsGutterLayoutMetrics::controlHeightSm() const noexcept
{
    return m_controlHeightSm;
}

void ContentsGutterLayoutMetrics::setControlHeightSm(const int value)
{
    if (m_controlHeightSm == value)
    {
        return;
    }

    m_controlHeightSm = value;
    emitChanged(&ContentsGutterLayoutMetrics::controlHeightSmChanged);
}

int ContentsGutterLayoutMetrics::dialogMaxWidth() const noexcept
{
    return m_dialogMaxWidth;
}

void ContentsGutterLayoutMetrics::setDialogMaxWidth(const int value)
{
    if (m_dialogMaxWidth == value)
    {
        return;
    }

    m_dialogMaxWidth = value;
    emitChanged(&ContentsGutterLayoutMetrics::dialogMaxWidthChanged);
}

int ContentsGutterLayoutMetrics::gutterWidthOverride() const noexcept
{
    return m_gutterWidthOverride;
}

void ContentsGutterLayoutMetrics::setGutterWidthOverride(const int value)
{
    if (m_gutterWidthOverride == value)
    {
        return;
    }

    m_gutterWidthOverride = value;
    emitChanged(&ContentsGutterLayoutMetrics::gutterWidthOverrideChanged);
}

int ContentsGutterLayoutMetrics::inputWidthMd() const noexcept
{
    return m_inputWidthMd;
}

void ContentsGutterLayoutMetrics::setInputWidthMd(const int value)
{
    if (m_inputWidthMd == value)
    {
        return;
    }

    m_inputWidthMd = value;
    emitChanged(&ContentsGutterLayoutMetrics::inputWidthMdChanged);
}

int ContentsGutterLayoutMetrics::lineNumberColumnLeftOverride() const noexcept
{
    return m_lineNumberColumnLeftOverride;
}

void ContentsGutterLayoutMetrics::setLineNumberColumnLeftOverride(const int value)
{
    if (m_lineNumberColumnLeftOverride == value)
    {
        return;
    }

    m_lineNumberColumnLeftOverride = value;
    emitChanged(&ContentsGutterLayoutMetrics::lineNumberColumnLeftOverrideChanged);
}

int ContentsGutterLayoutMetrics::lineNumberColumnTextWidthOverride() const noexcept
{
    return m_lineNumberColumnTextWidthOverride;
}

void ContentsGutterLayoutMetrics::setLineNumberColumnTextWidthOverride(const int value)
{
    if (m_lineNumberColumnTextWidthOverride == value)
    {
        return;
    }

    m_lineNumberColumnTextWidthOverride = value;
    emitChanged(&ContentsGutterLayoutMetrics::lineNumberColumnTextWidthOverrideChanged);
}

int ContentsGutterLayoutMetrics::logicalLineCount() const noexcept
{
    return m_logicalLineCount;
}

void ContentsGutterLayoutMetrics::setLogicalLineCount(const int value)
{
    if (m_logicalLineCount == value)
    {
        return;
    }

    m_logicalLineCount = value;
    emitChanged(&ContentsGutterLayoutMetrics::logicalLineCountChanged);
}

int ContentsGutterLayoutMetrics::changedMarkerHeight() const noexcept
{
    return m_gap20 * 2;
}

int ContentsGutterLayoutMetrics::changedMarkerY() const noexcept
{
    return m_inputWidthMd + m_controlHeightMd + m_gap5;
}

int ContentsGutterLayoutMetrics::conflictMarkerHeight() const noexcept
{
    return m_gap24 * 5;
}

int ContentsGutterLayoutMetrics::conflictMarkerY() const noexcept
{
    return m_dialogMaxWidth + m_gap20 + m_gap3;
}

int ContentsGutterLayoutMetrics::defaultActiveLineNumber() const noexcept
{
    return std::max(m_gapNone, m_controlHeightMd - m_gap5);
}

int ContentsGutterLayoutMetrics::defaultGutterWidth() const noexcept
{
    return (m_gap24 * 3) + m_gap2;
}

int ContentsGutterLayoutMetrics::designLineNumberCount() const noexcept
{
    return std::max(minimumMetricUnit(), m_controlHeightMd + m_gap7);
}

int ContentsGutterLayoutMetrics::effectiveGutterWidth() const noexcept
{
    if (m_gutterWidthOverride > m_gapNone)
    {
        return m_gutterWidthOverride;
    }

    return defaultGutterWidth();
}

int ContentsGutterLayoutMetrics::effectiveLineNumberCount() const noexcept
{
    return std::max(minimumMetricUnit(), m_logicalLineCount);
}

int ContentsGutterLayoutMetrics::iconRailX() const noexcept
{
    return m_gap20 * 2;
}

int ContentsGutterLayoutMetrics::inactiveLineNumber() const noexcept
{
    return -minimumMetricUnit();
}

int ContentsGutterLayoutMetrics::lineNumberBaseOffset() const noexcept
{
    return minimumMetricUnit();
}

int ContentsGutterLayoutMetrics::lineNumberColumnLeft() const noexcept
{
    if (m_lineNumberColumnLeftOverride > m_gapNone)
    {
        return m_lineNumberColumnLeftOverride;
    }

    return m_gap14;
}

int ContentsGutterLayoutMetrics::lineNumberColumnTextWidth() const noexcept
{
    if (m_lineNumberColumnTextWidthOverride > m_gapNone)
    {
        return m_lineNumberColumnTextWidthOverride;
    }

    return m_controlHeightSm;
}

void ContentsGutterLayoutMetrics::requestMetricsRefresh()
{
    emit metricsChanged();
}

int ContentsGutterLayoutMetrics::minimumMetricUnit() const noexcept
{
    return std::max(1, m_strokeThin);
}

void ContentsGutterLayoutMetrics::emitChanged(void (ContentsGutterLayoutMetrics::*signal)())
{
    emit (this->*signal)();
    emit metricsChanged();
}
