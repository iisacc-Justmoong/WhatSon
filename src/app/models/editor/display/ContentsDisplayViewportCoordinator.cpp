#include "app/models/editor/display/ContentsDisplayViewportCoordinator.hpp"

#include <QVariant>
#include <QtGlobal>

#include <cmath>

namespace
{
QString normalizeOptionalReason(const QString& reason)
{
    const QString trimmed = reason.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("note-entry-finalize") : trimmed;
}

QVariantMap normalizeCursorRect(const QVariantMap& rawRect, const double fallbackHeight)
{
    QVariantMap rect;
    rect.insert(
        QStringLiteral("height"),
        qMax(1.0, rawRect.value(QStringLiteral("height")).toDouble() > 0.0
                       ? rawRect.value(QStringLiteral("height")).toDouble()
                       : fallbackHeight));
    rect.insert(
        QStringLiteral("y"),
        qMax(0.0, rawRect.value(QStringLiteral("y")).toDouble()));
    rect.insert(
        QStringLiteral("width"),
        qMax(0.0, rawRect.value(QStringLiteral("width")).toDouble()));
    return rect;
}
}

ContentsDisplayViewportCoordinator::ContentsDisplayViewportCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayViewportCoordinator::~ContentsDisplayViewportCoordinator() = default;

bool ContentsDisplayViewportCoordinator::structuredHostGeometryActive() const noexcept { return m_structuredHostGeometryActive; }
void ContentsDisplayViewportCoordinator::setStructuredHostGeometryActive(const bool active)
{
    if (m_structuredHostGeometryActive == active)
        return;
    m_structuredHostGeometryActive = active;
    emit structuredHostGeometryActiveChanged();
}

bool ContentsDisplayViewportCoordinator::showPrintEditorLayout() const noexcept { return m_showPrintEditorLayout; }
void ContentsDisplayViewportCoordinator::setShowPrintEditorLayout(const bool active)
{
    if (m_showPrintEditorLayout == active)
        return;
    m_showPrintEditorLayout = active;
    emit showPrintEditorLayoutChanged();
}

bool ContentsDisplayViewportCoordinator::editorInputFocused() const noexcept { return m_editorInputFocused; }
void ContentsDisplayViewportCoordinator::setEditorInputFocused(const bool focused)
{
    if (m_editorInputFocused == focused)
        return;
    m_editorInputFocused = focused;
    emit editorInputFocusedChanged();
}

double ContentsDisplayViewportCoordinator::editorLineHeight() const noexcept { return m_editorLineHeight; }
void ContentsDisplayViewportCoordinator::setEditorLineHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorLineHeight, normalized))
        return;
    m_editorLineHeight = normalized;
    emit editorLineHeightChanged();
}

double ContentsDisplayViewportCoordinator::editorSurfaceHeight() const noexcept { return m_editorSurfaceHeight; }
void ContentsDisplayViewportCoordinator::setEditorSurfaceHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorSurfaceHeight, normalized))
        return;
    m_editorSurfaceHeight = normalized;
    emit editorSurfaceHeightChanged();
}

double ContentsDisplayViewportCoordinator::editorDocumentStartY() const noexcept { return m_editorDocumentStartY; }
void ContentsDisplayViewportCoordinator::setEditorDocumentStartY(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorDocumentStartY, normalized))
        return;
    m_editorDocumentStartY = normalized;
    emit editorDocumentStartYChanged();
}

double ContentsDisplayViewportCoordinator::editorViewportHeight() const noexcept { return m_editorViewportHeight; }
void ContentsDisplayViewportCoordinator::setEditorViewportHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorViewportHeight, normalized))
        return;
    m_editorViewportHeight = normalized;
    emit editorViewportHeightChanged();
}

double ContentsDisplayViewportCoordinator::editorContentOffsetY() const noexcept { return m_editorContentOffsetY; }
void ContentsDisplayViewportCoordinator::setEditorContentOffsetY(const double value)
{
    if (qFuzzyCompare(m_editorContentOffsetY, value))
        return;
    m_editorContentOffsetY = value;
    emit editorContentOffsetYChanged();
}

double ContentsDisplayViewportCoordinator::minimapResolvedTrackHeight() const noexcept { return m_minimapResolvedTrackHeight; }
void ContentsDisplayViewportCoordinator::setMinimapResolvedTrackHeight(const double value)
{
    const double normalized = qMax(1.0, value);
    if (qFuzzyCompare(m_minimapResolvedTrackHeight, normalized))
        return;
    m_minimapResolvedTrackHeight = normalized;
    emit minimapResolvedTrackHeightChanged();
}

int ContentsDisplayViewportCoordinator::logicalLineCount() const noexcept { return m_logicalLineCount; }
void ContentsDisplayViewportCoordinator::setLogicalLineCount(const int value)
{
    const int normalized = qMax(1, value);
    if (m_logicalLineCount == normalized)
        return;
    m_logicalLineCount = normalized;
    emit logicalLineCountChanged();
}

int ContentsDisplayViewportCoordinator::currentCursorLineNumber() const noexcept { return m_currentCursorLineNumber; }
void ContentsDisplayViewportCoordinator::setCurrentCursorLineNumber(const int value)
{
    const int normalized = qMax(1, value);
    if (m_currentCursorLineNumber == normalized)
        return;
    m_currentCursorLineNumber = normalized;
    emit currentCursorLineNumberChanged();
}

int ContentsDisplayViewportCoordinator::currentCursorOffset() const noexcept { return m_currentCursorOffset; }
void ContentsDisplayViewportCoordinator::setCurrentCursorOffset(const int value)
{
    const int normalized = qMax(0, value);
    if (m_currentCursorOffset == normalized)
        return;
    m_currentCursorOffset = normalized;
    emit currentCursorOffsetChanged();
}

int ContentsDisplayViewportCoordinator::logicalTextLength() const noexcept { return m_logicalTextLength; }
void ContentsDisplayViewportCoordinator::setLogicalTextLength(const int value)
{
    const int normalized = qMax(0, value);
    if (m_logicalTextLength == normalized)
        return;
    m_logicalTextLength = normalized;
    emit logicalTextLengthChanged();
}

QString ContentsDisplayViewportCoordinator::normalizedNoteId(const QString& noteId) const
{
    return noteId.trimmed();
}

bool ContentsDisplayViewportCoordinator::hasPendingNoteEntryGutterRefresh(const QString& pendingNoteId, const QString& noteId) const
{
    const QString normalizedPending = normalizedNoteId(pendingNoteId);
    if (normalizedPending.isEmpty())
        return false;
    if (noteId.isNull())
        return true;
    const QString normalizedNote = normalizedNoteId(noteId);
    return !normalizedNote.isEmpty() && normalizedPending == normalizedNote;
}

QVariantMap ContentsDisplayViewportCoordinator::finalizePendingNoteEntryGutterRefresh(
    const QString& pendingNoteId,
    const QString& selectedNoteId,
    const bool selectedNoteBodyLoading,
    const QString& reason,
    const bool refreshStructuredLayout) const
{
    QVariantMap plan;
    const QString normalizedSelected = normalizedNoteId(selectedNoteId);
    if (!hasPendingNoteEntryGutterRefresh(pendingNoteId, normalizedSelected))
        return plan;
    if (normalizedSelected.isEmpty() || selectedNoteBodyLoading)
        return plan;

    plan.insert(QStringLiteral("clearPendingNoteId"), true);
    plan.insert(QStringLiteral("refreshStructuredLayoutNow"), refreshStructuredLayout && m_structuredHostGeometryActive);
    plan.insert(QStringLiteral("commitGutterRefresh"), true);
    plan.insert(QStringLiteral("scheduleViewportGutterRefresh"), true);
    plan.insert(QStringLiteral("scheduleMinimapSnapshotRefresh"), true);
    plan.insert(QStringLiteral("scheduleGutterRefresh"), true);
    plan.insert(QStringLiteral("gutterPassCount"), 4);
    plan.insert(QStringLiteral("gutterReason"), normalizeOptionalReason(reason));
    return plan;
}

QVariantMap ContentsDisplayViewportCoordinator::buildLogicalLineMetricsFromStructuredEntries(const QVariantList& lineEntries) const
{
    QVariantList nextLineStartOffsets;
    nextLineStartOffsets.push_back(0);
    int logicalLength = 0;

    for (int lineIndex = 0; lineIndex < lineEntries.size(); ++lineIndex)
    {
        const QVariantMap entry = lineEntries.at(lineIndex).toMap();
        const int lineCharCount = qMax(0, entry.value(QStringLiteral("charCount")).toInt());
        if (lineIndex > 0)
            nextLineStartOffsets.push_back(logicalLength);
        logicalLength += lineCharCount;
        if (lineIndex + 1 < lineEntries.size())
            logicalLength += 1;
    }

    QVariantMap metrics;
    metrics.insert(QStringLiteral("logicalTextLength"), logicalLength);
    metrics.insert(QStringLiteral("lineStartOffsets"), nextLineStartOffsets);
    metrics.insert(QStringLiteral("lineCount"), qMax(1, lineEntries.size()));
    return metrics;
}

QVariantMap ContentsDisplayViewportCoordinator::buildLogicalLineMetricsFromText(const QString& text) const
{
    const QString normalizedText = QString(text).replace(QStringLiteral("\r\n"), QStringLiteral("\n")).replace(QStringLiteral("\r"), QStringLiteral("\n"));
    QVariantList nextLineStartOffsets;
    nextLineStartOffsets.push_back(0);
    for (int characterIndex = 0; characterIndex < normalizedText.size(); ++characterIndex)
    {
        if (normalizedText.at(characterIndex) == QChar('\n'))
            nextLineStartOffsets.push_back(characterIndex + 1);
    }

    QVariantMap metrics;
    metrics.insert(QStringLiteral("logicalTextLength"), normalizedText.size());
    metrics.insert(QStringLiteral("lineStartOffsets"), nextLineStartOffsets);
    metrics.insert(QStringLiteral("lineCount"), qMax(1, nextLineStartOffsets.size()));
    return metrics;
}

QString ContentsDisplayViewportCoordinator::structuredGutterGeometrySignature(const QVariantList& lineEntries) const
{
    if (!m_structuredHostGeometryActive)
        return QString();

    QStringList parts;
    parts.reserve(lineEntries.size());
    for (const QVariant& rawEntry : lineEntries)
    {
        const QVariantMap entry = rawEntry.toMap();
        const double contentY = qMax(0.0, entry.value(QStringLiteral("contentY")).toDouble());
        const double contentHeight = qMax(1.0, entry.value(QStringLiteral("contentHeight")).toDouble() > 0.0
                                                   ? entry.value(QStringLiteral("contentHeight")).toDouble()
                                                   : m_editorLineHeight);
        const double gutterContentY = qMax(
            0.0,
            entry.contains(QStringLiteral("gutterContentY"))
                ? entry.value(QStringLiteral("gutterContentY")).toDouble()
                : contentY);
        const double gutterContentHeight = qMax(
            1.0,
            entry.contains(QStringLiteral("gutterContentHeight"))
                ? entry.value(QStringLiteral("gutterContentHeight")).toDouble()
                : m_editorLineHeight);
        parts.push_back(
            QString::number(contentY)
            + QStringLiteral(":")
            + QString::number(contentHeight)
            + QStringLiteral(":")
            + QString::number(gutterContentY)
            + QStringLiteral(":")
            + QString::number(gutterContentHeight));
    }

    return parts.join(QStringLiteral("|"));
}

QVariantMap ContentsDisplayViewportCoordinator::consumeStructuredGutterGeometryChange(const QString& previousSignature, const QVariantList& lineEntries) const
{
    const QString nextSignature = structuredGutterGeometrySignature(lineEntries);
    QVariantMap result;
    result.insert(QStringLiteral("signature"), nextSignature);
    result.insert(QStringLiteral("changed"), previousSignature != nextSignature);
    return result;
}

int ContentsDisplayViewportCoordinator::logicalLineStartOffsetAt(
    const int lineIndex,
    const QVariantList& lineStartOffsets) const
{
    const int safeLineCount = qMax(1, m_logicalLineCount);
    const int safeIndex = qMax(0, qMin(safeLineCount - 1, lineIndex));
    if (safeIndex < lineStartOffsets.size())
        return qMax(0, lineStartOffsets.at(safeIndex).toInt());
    return 0;
}

int ContentsDisplayViewportCoordinator::logicalLineCharacterCountAt(
    const int lineIndex,
    const QVariantList& lineStartOffsets) const
{
    const int safeLineCount = qMax(1, m_logicalLineCount);
    const int safeIndex = qMax(0, qMin(safeLineCount - 1, lineIndex));
    const int startOffset = logicalLineStartOffsetAt(safeIndex, lineStartOffsets);
    const bool hasNextLine = safeIndex + 1 < safeLineCount;
    const int nextOffset = hasNextLine
        ? logicalLineStartOffsetAt(safeIndex + 1, lineStartOffsets)
        : qMax(0, m_logicalTextLength);
    return qMax(0, nextOffset - startOffset - (hasNextLine ? 1 : 0));
}

int ContentsDisplayViewportCoordinator::logicalLineNumberForOffset(
    const int offset,
    const QVariantList& lineStartOffsets) const
{
    const int safeLineCount = qMax(1, m_logicalLineCount);
    const int safeOffset = qMax(0, qMin(qMax(0, m_logicalTextLength), offset));
    int low = 0;
    int high = safeLineCount - 1;
    int best = 0;

    while (low <= high)
    {
        const int middle = (low + high) / 2;
        const int middleOffset = logicalLineStartOffsetAt(middle, lineStartOffsets);
        if (middleOffset <= safeOffset)
        {
            best = middle;
            low = middle + 1;
        }
        else
        {
            high = middle - 1;
        }
    }

    return best + 1;
}

double ContentsDisplayViewportCoordinator::minimapBarWidth(
    const int characterCount,
    const double resolvedTrackWidth) const
{
    const double safeTrackWidth = qMax(0.0, resolvedTrackWidth);
    const double maxWidth = qMax(6.0, safeTrackWidth - 1.0);
    const int safeCount = qMax(0, characterCount);
    if (safeCount <= 0)
        return qMax(2.0, maxWidth * 0.08);

    const double widthRatio = clampUnit(
        0.08 + std::log(static_cast<double>(safeCount) + 1.0) / std::log(160.0));
    return qMax(4.0, maxWidth * widthRatio);
}

double ContentsDisplayViewportCoordinator::minimapLineBarWidth(
    const double contentWidth,
    const double contentAvailableWidth,
    const int fallbackCharacterCount,
    const double resolvedTrackWidth) const
{
    const double safeTrackWidth = qMax(0.0, resolvedTrackWidth);
    const double maxWidth = qMax(6.0, safeTrackWidth - 1.0);
    const double safeContentWidth = qMax(0.0, contentWidth);
    const double safeContentAvailableWidth = qMax(safeContentWidth, qMax(0.0, contentAvailableWidth));
    if (safeContentWidth > 0.0 && safeContentAvailableWidth > 0.0)
        return qMax(2.0, maxWidth * clampUnit(safeContentWidth / safeContentAvailableWidth));
    return minimapBarWidth(fallbackCharacterCount, resolvedTrackWidth);
}

double ContentsDisplayViewportCoordinator::minimapTrackHeightForContentHeight(
    const double segmentHeight,
    const double contentHeight) const
{
    const double safeContentHeight = qMax(1.0, contentHeight);
    const double safeSegmentHeight = qMax(0.0, segmentHeight);
    return qMax(1.0, (safeSegmentHeight / safeContentHeight) * qMax(1.0, m_minimapResolvedTrackHeight));
}

double ContentsDisplayViewportCoordinator::minimapTrackYForContentY(
    const double contentY,
    const double contentHeight) const
{
    const double safeContentHeight = qMax(1.0, contentHeight);
    const double safeContentY = qMax(0.0, qMin(safeContentHeight, contentY));
    return (safeContentY / safeContentHeight) * qMax(1.0, m_minimapResolvedTrackHeight);
}

double ContentsDisplayViewportCoordinator::minimapViewportHeight(
    const bool flickableAvailable,
    const double contentHeight,
    const double viewportMinHeight) const
{
    const double trackHeight = qMax(1.0, m_minimapResolvedTrackHeight);
    if (!flickableAvailable)
        return trackHeight;

    const double safeContentHeight = qMax(1.0, contentHeight);
    const double viewportHeight = qMax(0.0, m_editorViewportHeight);
    if (safeContentHeight <= viewportHeight)
        return trackHeight;

    const double proportionalHeight = qMax(1.0, (viewportHeight / safeContentHeight) * trackHeight);
    return qMin(trackHeight, qMax(qMax(0.0, viewportMinHeight), proportionalHeight));
}

double ContentsDisplayViewportCoordinator::minimapViewportY(
    const bool flickableAvailable,
    const double flickableContentY,
    const double contentHeight,
    const double viewportHeight) const
{
    if (!flickableAvailable)
        return 0.0;

    const double safeContentHeight = qMax(1.0, contentHeight);
    const double editorViewportHeight = qMax(0.0, m_editorViewportHeight);
    const double maxContentY = qMax(0.0, safeContentHeight - editorViewportHeight);
    if (maxContentY <= 0.0)
        return 0.0;

    const double contentY = qMax(0.0, qMin(maxContentY, flickableContentY));
    const double maxTrackY = qMax(0.0, qMax(1.0, m_minimapResolvedTrackHeight) - qMax(0.0, viewportHeight));
    return maxTrackY * (contentY / maxContentY);
}

QVariantMap ContentsDisplayViewportCoordinator::minimapScrollPlan(const double localY, const double contentHeight) const
{
    QVariantMap plan;
    const double normalizedContentHeight = qMax(1.0, contentHeight);
    const double viewportHeight = qMax(0.0, m_editorViewportHeight);
    const double maxContentY = qMax(0.0, normalizedContentHeight - viewportHeight);
    if (maxContentY <= 0.0)
    {
        plan.insert(QStringLiteral("apply"), true);
        plan.insert(QStringLiteral("contentY"), 0.0);
        return plan;
    }

    const double trackRatio = clampUnit((localY <= 0.0 ? 0.0 : localY) / qMax(1.0, m_minimapResolvedTrackHeight));
    const double documentY = normalizedContentHeight * trackRatio;
    const double nextContentY = qMax(0.0, qMin(maxContentY, documentY - viewportHeight / 2.0));
    plan.insert(QStringLiteral("apply"), true);
    plan.insert(QStringLiteral("contentY"), nextContentY);
    return plan;
}

QVariantMap ContentsDisplayViewportCoordinator::typingViewportCorrectionPlan(
    const bool forceAnchor,
    const double flickableHeight,
    const double flickableContentHeight,
    const double currentContentY,
    const QVariantMap& cursorRect) const
{
    QVariantMap plan;
    if (m_showPrintEditorLayout || !m_editorInputFocused)
        return plan;

    const double viewportHeight = qMax(0.0, flickableHeight > 0.0 ? flickableHeight : m_editorViewportHeight);
    if (viewportHeight <= 0.0)
        return plan;

    const double contentHeight = qMax(viewportHeight, flickableContentHeight);
    const double maxContentY = qMax(0.0, contentHeight - viewportHeight);
    if (maxContentY <= 0.0)
        return plan;

    const QVariantMap normalizedRect = normalizeCursorRect(cursorRect, qMax(1.0, m_editorLineHeight));
    const double cursorHeight = normalizedRect.value(QStringLiteral("height")).toDouble();
    const double cursorTopViewportY = editorViewportYForDocumentY(normalizedRect.value(QStringLiteral("y")).toDouble());
    const double cursorBottomViewportY = cursorTopViewportY + cursorHeight;
    const double cursorCenterViewportY = cursorTopViewportY + cursorHeight / 2.0;
    const double bandTop = typingViewportBandTop(cursorHeight);
    const double bandBottom = typingViewportBandBottom(cursorHeight);
    const double anchorCenter = typingViewportAnchorCenter(cursorHeight);

    double deltaY = 0.0;
    if (forceAnchor)
        deltaY = cursorCenterViewportY - anchorCenter;
    else if (cursorBottomViewportY > bandBottom)
        deltaY = cursorBottomViewportY - bandBottom;
    else if (cursorTopViewportY < bandTop)
        deltaY = cursorTopViewportY - bandTop;
    else
        return plan;

    const double normalizedCurrentContentY = qMax(0.0, currentContentY);
    const double nextContentY = qMax(0.0, qMin(maxContentY, normalizedCurrentContentY + deltaY));
    if (qAbs(nextContentY - normalizedCurrentContentY) < 0.5)
        return plan;

    plan.insert(QStringLiteral("apply"), true);
    plan.insert(QStringLiteral("contentY"), nextContentY);
    return plan;
}

double ContentsDisplayViewportCoordinator::clampUnit(const double value) noexcept
{
    return qMax(0.0, qMin(1.0, value));
}

double ContentsDisplayViewportCoordinator::typingViewportBandTop(const double cursorHeight) const noexcept
{
    const double safeCursorHeight = qMax(1.0, cursorHeight > 0.0 ? cursorHeight : m_editorLineHeight);
    const double surfaceHeight = qMax(safeCursorHeight, m_editorSurfaceHeight);
    return m_editorDocumentStartY + qMax(safeCursorHeight, static_cast<double>(qRound(surfaceHeight * 0.18)));
}

double ContentsDisplayViewportCoordinator::typingViewportBandBottom(const double cursorHeight) const noexcept
{
    const double safeCursorHeight = qMax(1.0, cursorHeight > 0.0 ? cursorHeight : m_editorLineHeight);
    const double surfaceHeight = qMax(safeCursorHeight, m_editorSurfaceHeight);
    return m_editorDocumentStartY + qMax(safeCursorHeight * 2.0, static_cast<double>(qRound(surfaceHeight * 0.56)));
}

double ContentsDisplayViewportCoordinator::typingViewportAnchorCenter(const double cursorHeight) const noexcept
{
    const double safeCursorHeight = qMax(1.0, cursorHeight > 0.0 ? cursorHeight : m_editorLineHeight);
    const double surfaceHeight = qMax(safeCursorHeight, m_editorSurfaceHeight);
    return m_editorDocumentStartY + qMax(safeCursorHeight / 2.0, static_cast<double>(qRound(surfaceHeight * 0.5)));
}

double ContentsDisplayViewportCoordinator::editorViewportYForDocumentY(const double documentY) const noexcept
{
    return m_editorDocumentStartY + documentY + m_editorContentOffsetY;
}
