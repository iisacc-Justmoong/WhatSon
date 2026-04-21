#include "MobileHierarchyRouteCoordinator.hpp"

#include <QtGlobal>

namespace
{
    QString normalizedPath(const QString& value)
    {
        return value.trimmed();
    }
}

MobileHierarchyRouteCoordinator::MobileHierarchyRouteCoordinator(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyRouteCoordinator::~MobileHierarchyRouteCoordinator() = default;

QString MobileHierarchyRouteCoordinator::hierarchyRoutePath() const { return m_hierarchyRoutePath; }
void MobileHierarchyRouteCoordinator::setHierarchyRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_hierarchyRoutePath == normalized)
        return;
    m_hierarchyRoutePath = normalized;
    emit hierarchyRoutePathChanged();
}

QString MobileHierarchyRouteCoordinator::noteListRoutePath() const { return m_noteListRoutePath; }
void MobileHierarchyRouteCoordinator::setNoteListRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_noteListRoutePath == normalized)
        return;
    m_noteListRoutePath = normalized;
    emit noteListRoutePathChanged();
}

QString MobileHierarchyRouteCoordinator::editorRoutePath() const { return m_editorRoutePath; }
void MobileHierarchyRouteCoordinator::setEditorRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_editorRoutePath == normalized)
        return;
    m_editorRoutePath = normalized;
    emit editorRoutePathChanged();
}

QString MobileHierarchyRouteCoordinator::detailRoutePath() const { return m_detailRoutePath; }
void MobileHierarchyRouteCoordinator::setDetailRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_detailRoutePath == normalized)
        return;
    m_detailRoutePath = normalized;
    emit detailRoutePathChanged();
}

QString MobileHierarchyRouteCoordinator::lastObservedRoutePath() const { return m_lastObservedRoutePath; }
void MobileHierarchyRouteCoordinator::setLastObservedRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_lastObservedRoutePath == normalized)
        return;
    m_lastObservedRoutePath = normalized;
    emit lastObservedRoutePathChanged();
}

int MobileHierarchyRouteCoordinator::preservedNoteListSelectionIndex() const noexcept { return m_preservedNoteListSelectionIndex; }
void MobileHierarchyRouteCoordinator::setPreservedNoteListSelectionIndex(const int value)
{
    if (m_preservedNoteListSelectionIndex == value)
        return;
    m_preservedNoteListSelectionIndex = value;
    emit preservedNoteListSelectionIndexChanged();
}

bool MobileHierarchyRouteCoordinator::routeSelectionSyncSuppressed() const noexcept { return m_routeSelectionSyncSuppressed; }
void MobileHierarchyRouteCoordinator::setRouteSelectionSyncSuppressed(const bool value)
{
    if (m_routeSelectionSyncSuppressed == value)
        return;
    m_routeSelectionSyncSuppressed = value;
    emit routeSelectionSyncSuppressedChanged();
}

int MobileHierarchyRouteCoordinator::editorPopRepairRequestId() const noexcept { return m_editorPopRepairRequestId; }
void MobileHierarchyRouteCoordinator::setEditorPopRepairRequestId(const int value)
{
    if (m_editorPopRepairRequestId == value)
        return;
    m_editorPopRepairRequestId = value;
    emit editorPopRepairRequestIdChanged();
}

int MobileHierarchyRouteCoordinator::detailPopRepairRequestId() const noexcept { return m_detailPopRepairRequestId; }
void MobileHierarchyRouteCoordinator::setDetailPopRepairRequestId(const int value)
{
    if (m_detailPopRepairRequestId == value)
        return;
    m_detailPopRepairRequestId = value;
    emit detailPopRepairRequestIdChanged();
}

int MobileHierarchyRouteCoordinator::normalizedInteger(const QVariant& value, const int fallbackValue) const
{
    bool ok = false;
    const int converted = value.toInt(&ok);
    return ok ? converted : fallbackValue;
}

int MobileHierarchyRouteCoordinator::normalizedSelectionIndex(const QVariant& value) const
{
    return normalizedInteger(value, -1);
}

int MobileHierarchyRouteCoordinator::rememberSelectionIndex(const QVariant& explicitSelectionIndex, const int currentHierarchySelectionIndex)
{
    int nextSelectionIndex = normalizedSelectionIndex(explicitSelectionIndex);
    if (nextSelectionIndex < 0)
        nextSelectionIndex = currentHierarchySelectionIndex;
    setPreservedNoteListSelectionIndex(nextSelectionIndex);
    return nextSelectionIndex;
}

int MobileHierarchyRouteCoordinator::resolvedSelectionRestoreTarget(const QVariant& explicitSelectionIndex) const
{
    const int explicitIndex = normalizedSelectionIndex(explicitSelectionIndex);
    return explicitIndex >= 0 ? explicitIndex : m_preservedNoteListSelectionIndex;
}

int MobileHierarchyRouteCoordinator::routeStackDepth(const QVariant& depthValue) const
{
    return qMax(0, normalizedInteger(depthValue, 0));
}

QVariantMap MobileHierarchyRouteCoordinator::canonicalRoutePlan(const QString& targetPath, const bool hasRouter, const bool hasNoteListModel, const bool transitionActive, const QString& displayedPath) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("allowed"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("cancelTransition"), transitionActive);
    plan.insert(QStringLiteral("resetToRoot"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("suppressSelectionSync"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("restoreSelection"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("requestViewHook"), hasRouter && hasNoteListModel);

    QVariantList pushPaths;
    if (hasRouter && hasNoteListModel)
    {
        pushPaths.push_back(m_noteListRoutePath);
        if (normalizedPath(targetPath) == m_editorRoutePath || normalizedPath(targetPath) == m_detailRoutePath)
            pushPaths.push_back(m_editorRoutePath);
        if (normalizedPath(targetPath) == m_detailRoutePath)
            pushPaths.push_back(m_detailRoutePath);
    }
    plan.insert(QStringLiteral("pushPaths"), pushPaths);
    plan.insert(QStringLiteral("targetPath"), normalizedPath(targetPath));
    plan.insert(QStringLiteral("alreadyDisplayed"), normalizedPath(displayedPath) == normalizedPath(targetPath));
    return plan;
}

QVariantMap MobileHierarchyRouteCoordinator::directPushPlan(const QString& currentPath, const QString& displayedPath, const int depth, const QString& targetPath) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("useDirectPush"), false);
    plan.insert(QStringLiteral("requiresCanonicalRoute"), true);

    const QString normalizedTarget = normalizedPath(targetPath);
    const QString normalizedCurrent = normalizedPath(currentPath);
    const QString normalizedDisplayed = normalizedPath(displayedPath);

    if (normalizedTarget == m_noteListRoutePath)
    {
        const bool direct = normalizedCurrent == m_hierarchyRoutePath
            && normalizedDisplayed == m_hierarchyRoutePath
            && depth <= 1;
        plan.insert(QStringLiteral("useDirectPush"), direct);
        plan.insert(QStringLiteral("requiresCanonicalRoute"), !direct);
        return plan;
    }

    if (normalizedTarget == m_editorRoutePath)
    {
        const bool direct = normalizedCurrent == m_noteListRoutePath
            && normalizedDisplayed == m_noteListRoutePath
            && depth >= 2;
        plan.insert(QStringLiteral("useDirectPush"), direct);
        plan.insert(QStringLiteral("requiresCanonicalRoute"), !direct);
        return plan;
    }

    if (normalizedTarget == m_detailRoutePath)
    {
        const bool direct = normalizedCurrent == m_editorRoutePath
            && normalizedDisplayed == m_editorRoutePath
            && depth >= 3;
        plan.insert(QStringLiteral("useDirectPush"), direct);
        plan.insert(QStringLiteral("requiresCanonicalRoute"), !direct);
        return plan;
    }

    return plan;
}

QVariantMap MobileHierarchyRouteCoordinator::repairVerificationPlan(const int requestId, const bool editorRepair, const bool hasRouter, const bool hasNoteListModel, const QString& displayedPath, const QString& currentPath, const int depth, const int attemptsRemaining) const
{
    QVariantMap plan;
    const int expectedRequestId = editorRepair ? m_editorPopRepairRequestId : m_detailPopRepairRequestId;
    const QString targetPath = editorRepair ? m_noteListRoutePath : m_editorRoutePath;
    const int minimumDepth = editorRepair ? 2 : 3;

    plan.insert(QStringLiteral("valid"), requestId == expectedRequestId && hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("done"), false);
    plan.insert(QStringLiteral("retry"), false);
    plan.insert(QStringLiteral("fallbackTargetPath"), targetPath);

    if (!(requestId == expectedRequestId && hasRouter && hasNoteListModel))
        return plan;

    if (normalizedPath(displayedPath) == targetPath
        || (normalizedPath(currentPath) == targetPath && depth >= minimumDepth))
    {
        plan.insert(QStringLiteral("done"), true);
        return plan;
    }

    if (attemptsRemaining > 0)
    {
        plan.insert(QStringLiteral("retry"), true);
        plan.insert(QStringLiteral("nextAttemptsRemaining"), attemptsRemaining - 1);
        return plan;
    }

    plan.insert(QStringLiteral("fallbackToCanonicalRoute"), true);
    return plan;
}

QVariantMap MobileHierarchyRouteCoordinator::committedTransitionPlan(const QVariantMap& state, const bool hasNoteListModel, const QString& displayedPath) const
{
    QVariantMap plan;
    const QString operation = state.value(QStringLiteral("operation")).toString();
    const QString fromPath = normalizedPath(state.value(QStringLiteral("fromPath")).toString());

    plan.insert(QStringLiteral("requestViewHook"), true);
    plan.insert(QStringLiteral("rememberSelection"), operation == QStringLiteral("pop") && hasNoteListModel);
    plan.insert(QStringLiteral("scheduleRepair"), false);

    if (operation != QStringLiteral("pop") || !hasNoteListModel)
        return plan;

    if (fromPath == m_detailRoutePath)
    {
        plan.insert(QStringLiteral("cancelDetailRepair"), true);
        if (normalizedPath(displayedPath) != m_editorRoutePath)
        {
            plan.insert(QStringLiteral("scheduleRepair"), true);
            plan.insert(QStringLiteral("repairEditor"), false);
            plan.insert(QStringLiteral("repairRequestId"), m_detailPopRepairRequestId);
            plan.insert(QStringLiteral("attemptsRemaining"), 2);
        }
        return plan;
    }

    if (fromPath == m_editorRoutePath)
    {
        plan.insert(QStringLiteral("cancelEditorRepair"), true);
        if (normalizedPath(displayedPath) != m_noteListRoutePath)
        {
            plan.insert(QStringLiteral("scheduleRepair"), true);
            plan.insert(QStringLiteral("repairEditor"), true);
            plan.insert(QStringLiteral("repairRequestId"), m_editorPopRepairRequestId);
            plan.insert(QStringLiteral("attemptsRemaining"), 2);
        }
    }

    return plan;
}

QVariantMap MobileHierarchyRouteCoordinator::hierarchyRootPlan(const bool hasRouter, const bool transitionActive, const QString& displayedPath, const bool canGoBack) const
{
    QVariantMap plan;
    const bool alreadyRoot = normalizedPath(displayedPath) == m_hierarchyRoutePath && !canGoBack;
    plan.insert(QStringLiteral("allowed"), hasRouter);
    plan.insert(QStringLiteral("cancelTransition"), transitionActive);
    plan.insert(QStringLiteral("alreadyRoot"), alreadyRoot);
    plan.insert(QStringLiteral("setRoot"), hasRouter && !alreadyRoot);
    plan.insert(QStringLiteral("requestViewHook"), hasRouter && !alreadyRoot);
    return plan;
}

QVariantMap MobileHierarchyRouteCoordinator::routeSelectionSyncPlan(const bool hasRouter, const QString& displayedPath, const QString& routerCurrentPath, const int depth) const
{
    QVariantMap plan;
    const QString previousPath = m_lastObservedRoutePath;
    plan.insert(QStringLiteral("valid"), hasRouter);
    plan.insert(QStringLiteral("previousPath"), previousPath);
    plan.insert(QStringLiteral("nextPath"), normalizedPath(displayedPath));
    plan.insert(QStringLiteral("clearHierarchySelection"), false);

    if (!hasRouter)
        return plan;

    if (!m_routeSelectionSyncSuppressed
        && normalizedPath(displayedPath) == m_hierarchyRoutePath
        && previousPath != m_hierarchyRoutePath
        && normalizedPath(routerCurrentPath) == m_hierarchyRoutePath
        && depth <= 1)
    {
        plan.insert(QStringLiteral("clearHierarchySelection"), true);
    }
    return plan;
}
