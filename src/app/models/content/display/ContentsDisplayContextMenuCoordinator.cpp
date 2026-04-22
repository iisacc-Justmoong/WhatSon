#include "app/models/content/display/ContentsDisplayContextMenuCoordinator.hpp"

#include <QtGlobal>
#include <cmath>

namespace
{
    QString normalizedString(const QVariant& value)
    {
        return value.toString().trimmed();
    }

    QVariantMap safeMap(const QVariantMap& value)
    {
        return value;
    }
}

ContentsDisplayContextMenuCoordinator::ContentsDisplayContextMenuCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayContextMenuCoordinator::~ContentsDisplayContextMenuCoordinator() = default;

bool ContentsDisplayContextMenuCoordinator::structuredDocumentFlowVisible() const noexcept { return m_structuredDocumentFlowVisible; }
void ContentsDisplayContextMenuCoordinator::setStructuredDocumentFlowVisible(const bool value)
{
    if (m_structuredDocumentFlowVisible == value)
        return;
    m_structuredDocumentFlowVisible = value;
    emit structuredDocumentFlowVisibleChanged();
}

int ContentsDisplayContextMenuCoordinator::structuredContextMenuBlockIndex() const noexcept { return m_structuredContextMenuBlockIndex; }
void ContentsDisplayContextMenuCoordinator::setStructuredContextMenuBlockIndex(const int value)
{
    const int normalized = qMax(0, value);
    if (m_structuredContextMenuBlockIndex == normalized)
        return;
    m_structuredContextMenuBlockIndex = normalized;
    emit structuredContextMenuBlockIndexChanged();
}

QVariantMap ContentsDisplayContextMenuCoordinator::structuredContextMenuSelectionSnapshot() const
{
    return m_structuredContextMenuSelectionSnapshot;
}
void ContentsDisplayContextMenuCoordinator::setStructuredContextMenuSelectionSnapshot(const QVariantMap& value)
{
    if (m_structuredContextMenuSelectionSnapshot == value)
        return;
    m_structuredContextMenuSelectionSnapshot = value;
    emit structuredContextMenuSelectionSnapshotChanged();
}

QVariantMap ContentsDisplayContextMenuCoordinator::normalizeStructuredSelectionSnapshot(const QVariantMap& snapshot) const
{
    const QVariantMap safeSnapshot = safeMap(snapshot);
    QVariantMap normalized;
    normalized.insert(QStringLiteral("cursorPosition"), safeSnapshot.value(QStringLiteral("cursorPosition")).toDouble());
    normalized.insert(QStringLiteral("selectedText"), safeSnapshot.value(QStringLiteral("selectedText")).toString());
    normalized.insert(QStringLiteral("selectionEnd"), safeSnapshot.value(QStringLiteral("selectionEnd")).toDouble());
    normalized.insert(QStringLiteral("selectionStart"), safeSnapshot.value(QStringLiteral("selectionStart")).toDouble());
    return normalized;
}

bool ContentsDisplayContextMenuCoordinator::structuredSelectionValid() const
{
    const double selectionStart = m_structuredContextMenuSelectionSnapshot.value(QStringLiteral("selectionStart")).toDouble(std::numeric_limits<double>::quiet_NaN());
    const double selectionEnd = m_structuredContextMenuSelectionSnapshot.value(QStringLiteral("selectionEnd")).toDouble(std::numeric_limits<double>::quiet_NaN());
    return m_structuredContextMenuBlockIndex >= 0
        && std::isfinite(selectionStart)
        && std::isfinite(selectionEnd)
        && selectionEnd > selectionStart;
}

QString ContentsDisplayContextMenuCoordinator::inlineStyleTagForEvent(const QString& eventName) const
{
    const QString normalizedEventName = eventName.trimmed();
    if (normalizedEventName == QStringLiteral("editor.format.plain"))
        return QStringLiteral("plain");
    if (normalizedEventName == QStringLiteral("editor.format.bold"))
        return QStringLiteral("bold");
    if (normalizedEventName == QStringLiteral("editor.format.italic"))
        return QStringLiteral("italic");
    if (normalizedEventName == QStringLiteral("editor.format.underline"))
        return QStringLiteral("underline");
    if (normalizedEventName == QStringLiteral("editor.format.strikethrough"))
        return QStringLiteral("strikethrough");
    if (normalizedEventName == QStringLiteral("editor.format.highlight"))
        return QStringLiteral("highlight");
    return QString();
}

QVariantMap ContentsDisplayContextMenuCoordinator::primeStructuredSelectionSnapshotPlan(const QVariantMap& targetState) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("accepted"), false);
    plan.insert(QStringLiteral("resetSnapshot"), true);
    plan.insert(QStringLiteral("blockIndex"), -1);
    plan.insert(QStringLiteral("selectionSnapshot"), QVariantMap());

    if (!m_structuredDocumentFlowVisible)
        return plan;
    if (!targetState.value(QStringLiteral("valid")).toBool())
        return plan;

    const double blockIndex = targetState.value(QStringLiteral("blockIndex")).toDouble(std::numeric_limits<double>::quiet_NaN());
    if (!std::isfinite(blockIndex) || blockIndex < 0)
        return plan;

    const QVariantMap selectionSnapshot = normalizeStructuredSelectionSnapshot(
        targetState.value(QStringLiteral("selectionSnapshot")).toMap());
    const double selectionStart = selectionSnapshot.value(QStringLiteral("selectionStart")).toDouble(std::numeric_limits<double>::quiet_NaN());
    const double selectionEnd = selectionSnapshot.value(QStringLiteral("selectionEnd")).toDouble(std::numeric_limits<double>::quiet_NaN());
    if (!std::isfinite(selectionStart) || !std::isfinite(selectionEnd) || selectionEnd <= selectionStart)
        return plan;

    plan.insert(QStringLiteral("accepted"), true);
    plan.insert(QStringLiteral("resetSnapshot"), false);
    plan.insert(QStringLiteral("blockIndex"), static_cast<int>(std::floor(blockIndex)));
    plan.insert(QStringLiteral("selectionSnapshot"), selectionSnapshot);
    return plan;
}

QVariantMap ContentsDisplayContextMenuCoordinator::openSelectionContextMenuPlan(
    const bool structuredSelectionValid,
    const bool hasContextMenu,
    const double localX,
    const double localY) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("delegateToEditorSelectionController"), !m_structuredDocumentFlowVisible);
    plan.insert(QStringLiteral("openStructuredContextMenu"), false);
    plan.insert(QStringLiteral("requireStructuredSelectionPrime"), false);
    plan.insert(QStringLiteral("closeBeforeOpen"), false);
    plan.insert(QStringLiteral("openX"), qMax(0.0, localX));
    plan.insert(QStringLiteral("openY"), qMax(0.0, localY));

    if (!m_structuredDocumentFlowVisible)
        return plan;

    plan.insert(QStringLiteral("requireStructuredSelectionPrime"), !structuredSelectionValid);
    if (!hasContextMenu)
        return plan;

    plan.insert(QStringLiteral("openStructuredContextMenu"), structuredSelectionValid);
    plan.insert(QStringLiteral("closeBeforeOpen"), structuredSelectionValid);
    return plan;
}

QVariantMap ContentsDisplayContextMenuCoordinator::handleStructuredSelectionEventPlan(
    const QString& inlineStyleTag,
    const bool structuredSelectionValid,
    const bool canApplyInlineFormat) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("handled"), false);
    plan.insert(QStringLiteral("delegateToSelectionController"), !m_structuredDocumentFlowVisible);
    plan.insert(QStringLiteral("requireStructuredSelectionPrime"), false);
    plan.insert(QStringLiteral("applyStructuredInlineFormat"), false);
    plan.insert(QStringLiteral("inlineStyleTag"), inlineStyleTag);
    plan.insert(QStringLiteral("blockIndex"), m_structuredContextMenuBlockIndex);
    plan.insert(QStringLiteral("selectionSnapshot"), m_structuredContextMenuSelectionSnapshot);

    if (!m_structuredDocumentFlowVisible || inlineStyleTag.isEmpty() || !canApplyInlineFormat)
        return plan;

    plan.insert(QStringLiteral("requireStructuredSelectionPrime"), !structuredSelectionValid);
    plan.insert(QStringLiteral("applyStructuredInlineFormat"), structuredSelectionValid);
    return plan;
}
