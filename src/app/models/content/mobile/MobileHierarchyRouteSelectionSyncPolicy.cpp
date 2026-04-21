#include "app/models/content/mobile/MobileHierarchyRouteSelectionSyncPolicy.hpp"

namespace
{
    QString normalizedPath(const QString& value)
    {
        return value.trimmed();
    }
}

MobileHierarchyRouteSelectionSyncPolicy::MobileHierarchyRouteSelectionSyncPolicy(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyRouteSelectionSyncPolicy::~MobileHierarchyRouteSelectionSyncPolicy() = default;

QString MobileHierarchyRouteSelectionSyncPolicy::hierarchyRoutePath() const { return m_hierarchyRoutePath; }
void MobileHierarchyRouteSelectionSyncPolicy::setHierarchyRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_hierarchyRoutePath == normalized)
        return;
    m_hierarchyRoutePath = normalized;
    emit hierarchyRoutePathChanged();
}

QString MobileHierarchyRouteSelectionSyncPolicy::lastObservedRoutePath() const { return m_lastObservedRoutePath; }
void MobileHierarchyRouteSelectionSyncPolicy::setLastObservedRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_lastObservedRoutePath == normalized)
        return;
    m_lastObservedRoutePath = normalized;
    emit lastObservedRoutePathChanged();
}

bool MobileHierarchyRouteSelectionSyncPolicy::routeSelectionSyncSuppressed() const noexcept { return m_routeSelectionSyncSuppressed; }
void MobileHierarchyRouteSelectionSyncPolicy::setRouteSelectionSyncSuppressed(const bool value)
{
    if (m_routeSelectionSyncSuppressed == value)
        return;
    m_routeSelectionSyncSuppressed = value;
    emit routeSelectionSyncSuppressedChanged();
}

QVariantMap MobileHierarchyRouteSelectionSyncPolicy::routeSelectionSyncPlan(const bool hasRouter, const QString& displayedPath, const QString& routerCurrentPath, const int depth) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("valid"), hasRouter);
    plan.insert(QStringLiteral("previousPath"), m_lastObservedRoutePath);
    plan.insert(QStringLiteral("nextPath"), normalizedPath(displayedPath));
    plan.insert(QStringLiteral("clearHierarchySelection"), false);

    if (!hasRouter)
        return plan;

    if (!m_routeSelectionSyncSuppressed
        && normalizedPath(displayedPath) == m_hierarchyRoutePath
        && m_lastObservedRoutePath != m_hierarchyRoutePath
        && normalizedPath(routerCurrentPath) == m_hierarchyRoutePath
        && depth <= 1)
    {
        plan.insert(QStringLiteral("clearHierarchySelection"), true);
    }
    return plan;
}
