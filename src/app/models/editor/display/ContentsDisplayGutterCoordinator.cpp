#include "app/models/editor/display/ContentsDisplayGutterCoordinator.hpp"

#include <QtGlobal>

#include <algorithm>

namespace
{
    double inferredViewportBottom(const QVariantList& lineYs, const QVariantList& lineHeights)
    {
        double bottom = 0.0;
        for (int index = 0; index < lineYs.size(); ++index)
        {
            const double resolvedLineY = lineYs.at(index).toDouble();
            const double resolvedLineHeight = index < lineHeights.size()
                ? qMax(1.0, lineHeights.at(index).toDouble())
                : 1.0;
            bottom = std::max(bottom, resolvedLineY + resolvedLineHeight);
        }
        return bottom;
    }
}

ContentsDisplayGutterCoordinator::ContentsDisplayGutterCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayGutterCoordinator::~ContentsDisplayGutterCoordinator() = default;

bool ContentsDisplayGutterCoordinator::structuredHostGeometryActive() const noexcept { return m_structuredHostGeometryActive; }
void ContentsDisplayGutterCoordinator::setStructuredHostGeometryActive(const bool value)
{
    if (m_structuredHostGeometryActive == value)
        return;
    m_structuredHostGeometryActive = value;
    emit structuredHostGeometryActiveChanged();
}

int ContentsDisplayGutterCoordinator::logicalLineCount() const noexcept { return m_logicalLineCount; }
void ContentsDisplayGutterCoordinator::setLogicalLineCount(const int value)
{
    const int normalized = qMax(1, value);
    if (m_logicalLineCount == normalized)
        return;
    m_logicalLineCount = normalized;
    emit logicalLineCountChanged();
}

double ContentsDisplayGutterCoordinator::gutterViewportHeight() const noexcept { return m_gutterViewportHeight; }
void ContentsDisplayGutterCoordinator::setGutterViewportHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_gutterViewportHeight, normalized))
        return;
    m_gutterViewportHeight = normalized;
    emit gutterViewportHeightChanged();
}

QVariantList ContentsDisplayGutterCoordinator::buildVisiblePlainGutterLineEntries(
    const int firstVisibleLine,
    const QVariantList& gutterLineYs,
    const QVariantList& gutterLineHeights) const
{
    QVariantList visibleLines;
    const int safeFirstVisibleLine = qMax(1, qMin(m_logicalLineCount, firstVisibleLine));
    const double effectiveViewportHeight = m_gutterViewportHeight > 0.0
        ? m_gutterViewportHeight
        : inferredViewportBottom(gutterLineYs, gutterLineHeights);

    for (int lineNumber = safeFirstVisibleLine; lineNumber <= m_logicalLineCount; ++lineNumber)
    {
        const int index = lineNumber - safeFirstVisibleLine;
        const double resolvedLineY = index < gutterLineYs.size()
            ? gutterLineYs.at(index).toDouble()
            : 0.0;
        const double resolvedLineHeight = index < gutterLineHeights.size()
            ? qMax(1.0, gutterLineHeights.at(index).toDouble())
            : 1.0;

        if (resolvedLineY > effectiveViewportHeight)
            break;
        if (resolvedLineY + resolvedLineHeight < 0.0)
            continue;

        QVariantMap lineEntry;
        lineEntry.insert(QStringLiteral("lineNumber"), lineNumber);
        lineEntry.insert(QStringLiteral("y"), resolvedLineY);
        visibleLines.push_back(lineEntry);
    }

    if (visibleLines.isEmpty())
    {
        QVariantMap fallbackEntry;
        fallbackEntry.insert(QStringLiteral("lineNumber"), safeFirstVisibleLine);
        fallbackEntry.insert(
            QStringLiteral("y"),
            !gutterLineYs.isEmpty()
                ? gutterLineYs.first().toDouble()
                : 0.0);
        visibleLines.push_back(fallbackEntry);
    }

    return visibleLines;
}
