#include "ContentsDisplayContextMenuCoordinator.hpp"

#include <QtGlobal>

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
