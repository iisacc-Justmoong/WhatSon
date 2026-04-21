#include "MobileHierarchyBackSwipeCoordinator.hpp"

#include <QtGlobal>

MobileHierarchyBackSwipeCoordinator::MobileHierarchyBackSwipeCoordinator(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyBackSwipeCoordinator::~MobileHierarchyBackSwipeCoordinator() = default;

int MobileHierarchyBackSwipeCoordinator::backSwipeConsumedSessionId() const noexcept { return m_backSwipeConsumedSessionId; }
void MobileHierarchyBackSwipeCoordinator::setBackSwipeConsumedSessionId(const int value)
{
    if (m_backSwipeConsumedSessionId == value)
        return;
    m_backSwipeConsumedSessionId = value;
    emit backSwipeConsumedSessionIdChanged();
}

int MobileHierarchyBackSwipeCoordinator::backSwipeSessionId() const noexcept { return m_backSwipeSessionId; }
void MobileHierarchyBackSwipeCoordinator::setBackSwipeSessionId(const int value)
{
    if (m_backSwipeSessionId == value)
        return;
    m_backSwipeSessionId = value;
    emit backSwipeSessionIdChanged();
}

int MobileHierarchyBackSwipeCoordinator::backSwipeGeneratedSessionId() const noexcept { return m_backSwipeGeneratedSessionId; }
void MobileHierarchyBackSwipeCoordinator::setBackSwipeGeneratedSessionId(const int value)
{
    if (m_backSwipeGeneratedSessionId == value)
        return;
    m_backSwipeGeneratedSessionId = value;
    emit backSwipeGeneratedSessionIdChanged();
}

int MobileHierarchyBackSwipeCoordinator::backSwipeEdgeWidth() const noexcept { return m_backSwipeEdgeWidth; }
void MobileHierarchyBackSwipeCoordinator::setBackSwipeEdgeWidth(const int value)
{
    const int normalized = qMax(0, value);
    if (m_backSwipeEdgeWidth == normalized)
        return;
    m_backSwipeEdgeWidth = normalized;
    emit backSwipeEdgeWidthChanged();
}

int MobileHierarchyBackSwipeCoordinator::nextGeneratedSessionId()
{
    setBackSwipeGeneratedSessionId(m_backSwipeGeneratedSessionId + 1);
    return m_backSwipeGeneratedSessionId;
}

QVariantMap MobileHierarchyBackSwipeCoordinator::gestureEventData(
    const double localX,
    const double localY,
    const double totalDeltaX,
    const double totalDeltaY,
    const int sessionId,
    const double mappedGlobalX,
    const double mappedGlobalY) const
{
    QVariantMap eventData;
    eventData.insert(QStringLiteral("globalX"), mappedGlobalX);
    eventData.insert(QStringLiteral("globalY"), mappedGlobalY);
    eventData.insert(QStringLiteral("localX"), localX);
    eventData.insert(QStringLiteral("localY"), localY);
    eventData.insert(QStringLiteral("sessionId"), sessionId);
    eventData.insert(QStringLiteral("startGlobalX"), mappedGlobalX - totalDeltaX);
    eventData.insert(QStringLiteral("startGlobalY"), mappedGlobalY - totalDeltaY);
    eventData.insert(QStringLiteral("totalDeltaX"), totalDeltaX);
    eventData.insert(QStringLiteral("totalDeltaY"), totalDeltaY);
    eventData.insert(QStringLiteral("velocityX"), 0.0);
    eventData.insert(QStringLiteral("velocityY"), 0.0);
    return eventData;
}

QVariantMap MobileHierarchyBackSwipeCoordinator::beginGesturePlan(
    const bool backNavigationAvailable,
    const bool transitionActive,
    const QVariantMap& eventData,
    const double edgeOriginX) const
{
    QVariantMap plan;
    const int sessionId = eventData.value(QStringLiteral("sessionId")).toInt();
    const double startGlobalX = eventData.value(QStringLiteral("startGlobalX")).toDouble();
    const bool withinEdge = startGlobalX >= edgeOriginX && startGlobalX <= edgeOriginX + m_backSwipeEdgeWidth;
    const bool allowed = backNavigationAvailable
        && !transitionActive
        && sessionId >= 0
        && sessionId != m_backSwipeConsumedSessionId
        && qIsFinite(startGlobalX)
        && withinEdge;
    plan.insert(QStringLiteral("begin"), allowed);
    plan.insert(QStringLiteral("sessionId"), sessionId);
    return plan;
}

QVariantMap MobileHierarchyBackSwipeCoordinator::cancelGesturePlan(const QVariantMap& eventData) const
{
    QVariantMap plan;
    const int sessionId = eventData.value(QStringLiteral("sessionId")).toInt();
    const bool valid = m_backSwipeSessionId >= 0 && (sessionId < 0 || sessionId == m_backSwipeSessionId);
    plan.insert(QStringLiteral("cancel"), valid);
    plan.insert(QStringLiteral("sessionId"), sessionId);
    return plan;
}

QVariantMap MobileHierarchyBackSwipeCoordinator::finishGesturePlan(const QVariantMap& eventData, const bool cancelled, const bool transitionActive, const double transitionProgress) const
{
    QVariantMap plan;
    const int sessionId = eventData.value(QStringLiteral("sessionId")).toInt();
    const bool valid = m_backSwipeSessionId >= 0 && (sessionId < 0 || sessionId == m_backSwipeSessionId);
    plan.insert(QStringLiteral("valid"), valid);
    plan.insert(QStringLiteral("cancelled"), cancelled);
    plan.insert(QStringLiteral("sessionId"), sessionId);
    if (!valid || !transitionActive)
        return plan;
    const double velocityX = eventData.value(QStringLiteral("velocityX")).toDouble();
    const double velocityY = eventData.value(QStringLiteral("velocityY")).toDouble();
    const bool shouldCommit = transitionProgress >= 0.5 || velocityX > qAbs(velocityY);
    plan.insert(QStringLiteral("shouldCommit"), !cancelled && shouldCommit);
    return plan;
}

QVariantMap MobileHierarchyBackSwipeCoordinator::updateGesturePlan(const QVariantMap& eventData, const bool transitionActive, const double viewportWidth, const double cancelVerticalThreshold) const
{
    QVariantMap plan;
    const int sessionId = eventData.value(QStringLiteral("sessionId")).toInt();
    const double totalDeltaX = eventData.value(QStringLiteral("totalDeltaX")).toDouble();
    const double totalDeltaY = eventData.value(QStringLiteral("totalDeltaY")).toDouble();
    const double absoluteDeltaX = qAbs(totalDeltaX);
    const double absoluteDeltaY = qAbs(totalDeltaY);

    const bool valid = sessionId >= 0
        && sessionId != m_backSwipeConsumedSessionId
        && m_backSwipeSessionId >= 0
        && sessionId == m_backSwipeSessionId
        && transitionActive;
    plan.insert(QStringLiteral("valid"), valid);
    if (!valid)
        return plan;

    if (absoluteDeltaY > absoluteDeltaX && absoluteDeltaY >= cancelVerticalThreshold)
    {
        plan.insert(QStringLiteral("cancel"), true);
        return plan;
    }

    const double safeViewportWidth = qMax(1.0, viewportWidth);
    const double progress = qBound(0.0, totalDeltaX / safeViewportWidth, 1.0);
    plan.insert(QStringLiteral("update"), true);
    plan.insert(QStringLiteral("progress"), progress);
    plan.insert(QStringLiteral("velocityX"), eventData.value(QStringLiteral("velocityX")).toDouble());
    plan.insert(QStringLiteral("velocityY"), eventData.value(QStringLiteral("velocityY")).toDouble());
    return plan;
}

void MobileHierarchyBackSwipeCoordinator::resetState()
{
    setBackSwipeConsumedSessionId(-1);
    setBackSwipeSessionId(-1);
}
