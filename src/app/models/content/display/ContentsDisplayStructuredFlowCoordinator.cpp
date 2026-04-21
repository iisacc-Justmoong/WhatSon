#include "app/models/content/display/ContentsDisplayStructuredFlowCoordinator.hpp"

#include <QtGlobal>

ContentsDisplayStructuredFlowCoordinator::ContentsDisplayStructuredFlowCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayStructuredFlowCoordinator::~ContentsDisplayStructuredFlowCoordinator() = default;

bool ContentsDisplayStructuredFlowCoordinator::structuredHostGeometryActive() const noexcept { return m_structuredHostGeometryActive; }
void ContentsDisplayStructuredFlowCoordinator::setStructuredHostGeometryActive(const bool active)
{
    if (m_structuredHostGeometryActive == active)
        return;
    m_structuredHostGeometryActive = active;
    emit structuredHostGeometryActiveChanged();
}

double ContentsDisplayStructuredFlowCoordinator::editorLineHeight() const noexcept { return m_editorLineHeight; }
void ContentsDisplayStructuredFlowCoordinator::setEditorLineHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorLineHeight, normalized))
        return;
    m_editorLineHeight = normalized;
    emit editorLineHeightChanged();
}

double ContentsDisplayStructuredFlowCoordinator::gutterViewportHeight() const noexcept { return m_gutterViewportHeight; }
void ContentsDisplayStructuredFlowCoordinator::setGutterViewportHeight(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_gutterViewportHeight, normalized))
        return;
    m_gutterViewportHeight = normalized;
    emit gutterViewportHeightChanged();
}

double ContentsDisplayStructuredFlowCoordinator::editorDocumentStartY() const noexcept { return m_editorDocumentStartY; }
void ContentsDisplayStructuredFlowCoordinator::setEditorDocumentStartY(const double value)
{
    const double normalized = qMax(0.0, value);
    if (qFuzzyCompare(m_editorDocumentStartY, normalized))
        return;
    m_editorDocumentStartY = normalized;
    emit editorDocumentStartYChanged();
}

double ContentsDisplayStructuredFlowCoordinator::editorContentOffsetY() const noexcept { return m_editorContentOffsetY; }
void ContentsDisplayStructuredFlowCoordinator::setEditorContentOffsetY(const double value)
{
    if (qFuzzyCompare(m_editorContentOffsetY, value))
        return;
    m_editorContentOffsetY = value;
    emit editorContentOffsetYChanged();
}

int ContentsDisplayStructuredFlowCoordinator::logicalLineCount() const noexcept { return m_logicalLineCount; }
void ContentsDisplayStructuredFlowCoordinator::setLogicalLineCount(const int value)
{
    const int normalized = qMax(1, value);
    if (m_logicalLineCount == normalized)
        return;
    m_logicalLineCount = normalized;
    emit logicalLineCountChanged();
}

QString ContentsDisplayStructuredFlowCoordinator::structuredGutterGeometrySignature() const
{
    return m_structuredGutterGeometrySignature;
}

void ContentsDisplayStructuredFlowCoordinator::setStructuredGutterGeometrySignature(const QString& signature)
{
    if (m_structuredGutterGeometrySignature == signature)
        return;
    m_structuredGutterGeometrySignature = signature;
    emit structuredGutterGeometrySignatureChanged();
}

QVariantList ContentsDisplayStructuredFlowCoordinator::normalizeStructuredLogicalLineEntries(const QVariant& rawEntries) const
{
    if (!m_structuredHostGeometryActive)
        return {};

    const QVariantList rawList = rawEntries.toList();
    if (!rawList.isEmpty())
        return rawList;

    QVariantList normalized;
    const QVariantMap rawMap = rawEntries.toMap();
    Q_UNUSED(rawMap)

    const int explicitLength = qFloor(rawEntries.value<QVariantList>().size());
    if (explicitLength > 0)
        return rawEntries.value<QVariantList>();

    if (rawEntries.canConvert<QVariantList>())
        return rawEntries.toList();

    return normalized;
}

QVariantMap ContentsDisplayStructuredFlowCoordinator::evaluateStructuredLayoutState(const QVariant& rawEntries) const
{
    const QVariantList entries = normalizeStructuredLogicalLineEntries(rawEntries);
    const QString nextSignature = buildStructuredGutterGeometrySignature(entries);

    QVariantMap result;
    result.insert(QStringLiteral("entries"), entries);
    result.insert(QStringLiteral("signature"), nextSignature);
    result.insert(QStringLiteral("geometryChanged"), m_structuredGutterGeometrySignature != nextSignature);
    result.insert(QStringLiteral("lineCount"), qMax(1, entries.size()));
    return result;
}

QVariantList ContentsDisplayStructuredFlowCoordinator::buildVisibleStructuredGutterLineEntries(const QVariantList& lineEntries, const int fallbackFirstVisibleLine) const
{
    QVariantList visibleLines;
    if (lineEntries.isEmpty())
    {
        QVariantMap singleLine;
        singleLine.insert(QStringLiteral("height"), qMax(1.0, m_editorLineHeight));
        singleLine.insert(QStringLiteral("lineNumber"), 1);
        singleLine.insert(QStringLiteral("y"), editorViewportYForDocumentY(0.0));
        visibleLines.push_back(singleLine);
        return visibleLines;
    }

    for (int lineIndex = 0; lineIndex < lineEntries.size(); ++lineIndex)
    {
        const QVariantMap entry = lineEntries.at(lineIndex).toMap();
        const int lineNumber = lineIndex + 1;
        const double lineContentY = qMax(
            0.0,
            entry.contains(QStringLiteral("gutterContentY"))
                ? entry.value(QStringLiteral("gutterContentY")).toDouble()
                : entry.value(QStringLiteral("contentY")).toDouble());
        const double gutterY = editorViewportYForDocumentY(lineContentY);
        const double gutterHeight = qMax(
            1.0,
            entry.contains(QStringLiteral("gutterContentHeight"))
                ? entry.value(QStringLiteral("gutterContentHeight")).toDouble()
                : m_editorLineHeight);
        const double visibleHeight = qMax(
            1.0,
            entry.contains(QStringLiteral("contentHeight"))
                ? entry.value(QStringLiteral("contentHeight")).toDouble()
                : m_editorLineHeight);
        if (gutterY > m_gutterViewportHeight)
            break;
        if (gutterY + visibleHeight < 0.0)
            continue;

        QVariantMap row;
        row.insert(QStringLiteral("height"), gutterHeight);
        row.insert(QStringLiteral("lineNumber"), lineNumber);
        row.insert(QStringLiteral("y"), gutterY);
        visibleLines.push_back(row);
    }

    if (visibleLines.isEmpty())
    {
        QVariantMap row;
        row.insert(QStringLiteral("lineNumber"), qMax(1, fallbackFirstVisibleLine));
        row.insert(QStringLiteral("y"), 0.0);
        row.insert(QStringLiteral("height"), qMax(1.0, m_editorLineHeight));
        visibleLines.push_back(row);
    }

    return visibleLines;
}

double ContentsDisplayStructuredFlowCoordinator::editorViewportYForDocumentY(const double documentY) const noexcept
{
    return m_editorDocumentStartY + documentY + m_editorContentOffsetY;
}

QString ContentsDisplayStructuredFlowCoordinator::buildStructuredGutterGeometrySignature(const QVariantList& lineEntries) const
{
    if (!m_structuredHostGeometryActive)
        return {};

    QStringList parts;
    parts.reserve(lineEntries.size());
    for (const QVariant& rawEntry : lineEntries)
    {
        const QVariantMap entry = rawEntry.toMap();
        const double contentY = qMax(0.0, entry.value(QStringLiteral("contentY")).toDouble());
        const double contentHeight = qMax(
            1.0,
            entry.contains(QStringLiteral("contentHeight"))
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
        parts.push_back(QString::number(contentY)
                        + QStringLiteral(":")
                        + QString::number(contentHeight)
                        + QStringLiteral(":")
                        + QString::number(gutterContentY)
                        + QStringLiteral(":")
                        + QString::number(gutterContentHeight));
    }
    return parts.join(QStringLiteral("|"));
}
