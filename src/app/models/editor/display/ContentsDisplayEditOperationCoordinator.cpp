#include "app/models/editor/display/ContentsDisplayEditOperationCoordinator.hpp"

ContentsDisplayEditOperationCoordinator::ContentsDisplayEditOperationCoordinator(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplayEditOperationCoordinator::~ContentsDisplayEditOperationCoordinator() = default;

QVariantMap ContentsDisplayEditOperationCoordinator::structuredShortcutPlan(
    const QString& shortcutKind,
    const bool showStructuredDocumentFlow,
    const bool canUseStructuredShortcut) const
{
    QVariantMap plan;
    const QString normalizedShortcutKind = shortcutKind.trimmed();
    plan.insert(QStringLiteral("handled"), false);
    plan.insert(QStringLiteral("shortcutKind"), normalizedShortcutKind);
    if (!showStructuredDocumentFlow || !canUseStructuredShortcut || normalizedShortcutKind.isEmpty())
        return plan;
    plan.insert(QStringLiteral("handled"), true);
    return plan;
}

QVariantMap ContentsDisplayEditOperationCoordinator::structuredEndEditPlan(
    const bool showStructuredDocumentFlow,
    const bool canRequestStructuredEndEdit,
    const int logicalTextLength) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("handled"), false);
    plan.insert(QStringLiteral("targetOffset"), qMax(0, logicalTextLength));
    if (!showStructuredDocumentFlow || !canRequestStructuredEndEdit)
        return plan;
    plan.insert(QStringLiteral("handled"), true);
    return plan;
}

QVariantMap ContentsDisplayEditOperationCoordinator::focusStructuredSourceOffsetPlan(
    const bool showStructuredDocumentFlow,
    const bool canRequestStructuredFocus,
    const int sourceOffset) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("handled"), false);
    plan.insert(QStringLiteral("targetOffset"), qMax(0, sourceOffset));
    if (!showStructuredDocumentFlow || !canRequestStructuredFocus || sourceOffset < 0)
        return plan;
    plan.insert(QStringLiteral("handled"), true);
    return plan;
}
