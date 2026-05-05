#include "app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    qreal normalizedRatio(const QVariant& value) noexcept
    {
        bool ok = false;
        const qreal ratio = value.toDouble(&ok);
        if (!ok || !std::isfinite(ratio))
        {
            return 1.0;
        }
        return std::max<qreal>(0.0, std::min<qreal>(1.0, ratio));
    }
} // namespace

ContentsEditorVisualLineMetrics::ContentsEditorVisualLineMetrics(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorVisualLineMetrics::~ContentsEditorVisualLineMetrics() = default;

int ContentsEditorVisualLineMetrics::measuredVisualLineCount() const noexcept
{
    return m_measuredVisualLineCount;
}

void ContentsEditorVisualLineMetrics::setMeasuredVisualLineCount(const int value)
{
    const int nextValue = std::max(1, value);
    if (m_measuredVisualLineCount == nextValue)
    {
        return;
    }
    m_measuredVisualLineCount = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::measuredVisualLineCountChanged);
}

QVariantList ContentsEditorVisualLineMetrics::measuredLineWidthRatios() const
{
    return m_measuredLineWidthRatios;
}

void ContentsEditorVisualLineMetrics::setMeasuredLineWidthRatios(const QVariantList& value)
{
    if (m_measuredLineWidthRatios == value)
    {
        return;
    }
    m_measuredLineWidthRatios = value;
    emitInputChanged(&ContentsEditorVisualLineMetrics::measuredLineWidthRatiosChanged);
}

int ContentsEditorVisualLineMetrics::visualLineCount() const
{
    return std::max(1, std::max(m_measuredVisualLineCount, static_cast<int>(m_measuredLineWidthRatios.size())));
}

QVariantList ContentsEditorVisualLineMetrics::visualLineWidthRatios() const
{
    const int rowCount = visualLineCount();
    QVariantList ratios;
    ratios.reserve(rowCount);

    for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        ratios.push_back(
            rowIndex < m_measuredLineWidthRatios.size()
                ? normalizedRatio(m_measuredLineWidthRatios.at(rowIndex))
                : 1.0);
    }

    return ratios;
}

void ContentsEditorVisualLineMetrics::requestMetricsRefresh()
{
    emit metricsChanged();
}

void ContentsEditorVisualLineMetrics::emitInputChanged(void (ContentsEditorVisualLineMetrics::*signal)())
{
    emit (this->*signal)();
    emit metricsChanged();
}
