#include "app/models/content/mobile/MobileEventSurfaceController.hpp"

#include <QtGlobal>

#include <cmath>

namespace
{
    double finiteOrZero(const double value)
    {
        return qIsFinite(value) ? value : 0.0;
    }

    double nonNegativeFiniteOrDefault(const double value, const double fallbackValue)
    {
        return qIsFinite(value) ? qMax(0.0, value) : fallbackValue;
    }
} // namespace

MobileEventSurfaceController::MobileEventSurfaceController(QObject* parent)
    : QObject(parent)
{
}

MobileEventSurfaceController::~MobileEventSurfaceController() = default;

int MobileEventSurfaceController::generatedSessionId() const noexcept { return m_generatedSessionId; }
void MobileEventSurfaceController::setGeneratedSessionId(const int value)
{
    if (m_generatedSessionId == value)
        return;
    m_generatedSessionId = value;
    emit generatedSessionIdChanged();
}

int MobileEventSurfaceController::activeSessionId() const noexcept { return m_activeSessionId; }
QString MobileEventSurfaceController::activeEventKind() const { return m_activeEventKind; }
QString MobileEventSurfaceController::activeDirection() const { return m_activeDirection; }
int MobileEventSurfaceController::activeFingerCount() const noexcept { return m_activeFingerCount; }

double MobileEventSurfaceController::tapMoveThreshold() const noexcept { return m_tapMoveThreshold; }
void MobileEventSurfaceController::setTapMoveThreshold(const double value)
{
    const double normalized = nonNegativeFiniteOrDefault(value, m_tapMoveThreshold);
    if (qFuzzyCompare(m_tapMoveThreshold, normalized))
        return;
    m_tapMoveThreshold = normalized;
    emit tapMoveThresholdChanged();
}

double MobileEventSurfaceController::scrollDistanceThreshold() const noexcept { return m_scrollDistanceThreshold; }
void MobileEventSurfaceController::setScrollDistanceThreshold(const double value)
{
    const double normalized = nonNegativeFiniteOrDefault(value, m_scrollDistanceThreshold);
    if (qFuzzyCompare(m_scrollDistanceThreshold, normalized))
        return;
    m_scrollDistanceThreshold = normalized;
    emit scrollDistanceThresholdChanged();
}

double MobileEventSurfaceController::gestureDistanceThreshold() const noexcept { return m_gestureDistanceThreshold; }
void MobileEventSurfaceController::setGestureDistanceThreshold(const double value)
{
    const double normalized = nonNegativeFiniteOrDefault(value, m_gestureDistanceThreshold);
    if (qFuzzyCompare(m_gestureDistanceThreshold, normalized))
        return;
    m_gestureDistanceThreshold = normalized;
    emit gestureDistanceThresholdChanged();
}

int MobileEventSurfaceController::nextSessionId()
{
    setGeneratedSessionId(m_generatedSessionId + 1);
    return m_generatedSessionId;
}

QVariantMap MobileEventSurfaceController::beginTouchSequence(const int sessionId, const QVariantList& points)
{
    const QVector<TouchPoint> normalizedPoints = pointsFromVariantList(points);
    if (sessionId < 0 || normalizedPoints.isEmpty())
    {
        return eventData(
            QStringLiteral("begin"),
            QStringLiteral("invalid"),
            QStringLiteral("none"),
            normalizedPoints,
            false);
    }

    m_startPoints = normalizedPoints;
    m_currentPoints = normalizedPoints;
    setActiveSessionId(sessionId);
    setActiveEventKind(QStringLiteral("pending"));
    setActiveDirection(QStringLiteral("none"));
    setActiveFingerCount(normalizedPoints.size());

    const QVariantMap data = eventData(
        QStringLiteral("begin"),
        m_activeEventKind,
        m_activeDirection,
        m_currentPoints,
        true);
    emit classificationChanged(data);
    return data;
}

QVariantMap MobileEventSurfaceController::updateTouchSequence(const int sessionId, const QVariantList& points)
{
    const QVector<TouchPoint> normalizedPoints = pointsFromVariantList(points);
    if (!hasActiveSession(sessionId) || normalizedPoints.isEmpty())
    {
        return eventData(
            QStringLiteral("update"),
            QStringLiteral("invalid"),
            QStringLiteral("none"),
            normalizedPoints,
            false);
    }

    m_currentPoints = normalizedPoints;
    setActiveFingerCount(normalizedPoints.size());

    const QPointF startCentroid = centroidForPoints(m_startPoints);
    const QPointF currentCentroid = centroidForPoints(m_currentPoints);
    const double deltaX = currentCentroid.x() - startCentroid.x();
    const double deltaY = currentCentroid.y() - startCentroid.y();
    const double distance = std::hypot(deltaX, deltaY);

    QString nextKind = m_activeEventKind;
    const int fingerCount = m_currentPoints.size();
    if (fingerCount >= 2 && (distance >= m_gestureDistanceThreshold || m_activeEventKind == QStringLiteral("gesture")))
    {
        nextKind = QStringLiteral("gesture");
    }
    else if (fingerCount == 1 && (distance >= m_scrollDistanceThreshold || m_activeEventKind == QStringLiteral("scroll")))
    {
        nextKind = QStringLiteral("scroll");
    }

    const QString nextDirection = directionForDelta(
        deltaX,
        deltaY,
        nextKind == QStringLiteral("pending") ? m_tapMoveThreshold : 0.0);
    const QString previousKind = m_activeEventKind;
    setActiveEventKind(nextKind);
    setActiveDirection(nextDirection);

    const QVariantMap data = eventData(
        QStringLiteral("update"),
        m_activeEventKind,
        m_activeDirection,
        m_currentPoints,
        true);
    emitRecognitionSignal(previousKind, m_activeEventKind, data);
    emit classificationChanged(data);
    return data;
}

QVariantMap MobileEventSurfaceController::endTouchSequence(const int sessionId, const QVariantList& points, const bool cancelled)
{
    const QVector<TouchPoint> normalizedPoints = pointsFromVariantList(points);
    if (cancelled)
        return cancelTouchSequence(sessionId);
    if (!hasActiveSession(sessionId))
    {
        return eventData(
            QStringLiteral("end"),
            QStringLiteral("invalid"),
            QStringLiteral("none"),
            normalizedPoints,
            false);
    }

    if (!normalizedPoints.isEmpty())
        m_currentPoints = normalizedPoints;

    QString finalKind = m_activeEventKind;
    if (finalKind == QStringLiteral("pending"))
        finalKind = QStringLiteral("touch");

    const QPointF startCentroid = centroidForPoints(m_startPoints);
    const QPointF currentCentroid = centroidForPoints(m_currentPoints);
    const QString finalDirection = finalKind == QStringLiteral("touch")
        ? QStringLiteral("none")
        : directionForDelta(currentCentroid.x() - startCentroid.x(), currentCentroid.y() - startCentroid.y(), 0.0);

    setActiveEventKind(finalKind);
    setActiveDirection(finalDirection);
    const QVariantMap data = eventData(
        QStringLiteral("end"),
        finalKind,
        finalDirection,
        m_currentPoints,
        true);
    if (finalKind == QStringLiteral("touch"))
        emit touchRecognized(data);
    emit classificationChanged(data);
    clearActiveSequence();
    return data;
}

QVariantMap MobileEventSurfaceController::cancelTouchSequence(const int sessionId)
{
    if (!hasActiveSession(sessionId))
    {
        return eventData(
            QStringLiteral("cancel"),
            QStringLiteral("invalid"),
            QStringLiteral("none"),
            {},
            false);
    }

    const QVariantMap data = eventData(
        QStringLiteral("cancel"),
        QStringLiteral("cancelled"),
        QStringLiteral("none"),
        m_currentPoints,
        true);
    emit classificationChanged(data);
    clearActiveSequence();
    return data;
}

void MobileEventSurfaceController::resetState()
{
    clearActiveSequence();
}

QVector<MobileEventSurfaceController::TouchPoint> MobileEventSurfaceController::pointsFromVariantList(
    const QVariantList& points)
{
    QVector<TouchPoint> normalizedPoints;
    normalizedPoints.reserve(points.size());
    for (int index = 0; index < points.size(); ++index)
    {
        const QVariantMap pointMap = points.at(index).toMap();
        TouchPoint point;
        point.id = pointMap.value(QStringLiteral("id"), index).toInt();
        point.position = QPointF(
            finiteOrZero(pointMap.value(QStringLiteral("x")).toDouble()),
            finiteOrZero(pointMap.value(QStringLiteral("y")).toDouble()));
        normalizedPoints.push_back(point);
    }
    return normalizedPoints;
}

QPointF MobileEventSurfaceController::centroidForPoints(const QVector<TouchPoint>& points)
{
    if (points.isEmpty())
        return {};

    QPointF centroid;
    for (const TouchPoint& point : points)
    {
        centroid += point.position;
    }
    return centroid / static_cast<double>(points.size());
}

QString MobileEventSurfaceController::directionForDelta(
    const double deltaX,
    const double deltaY,
    const double threshold)
{
    const double absoluteDeltaX = qAbs(deltaX);
    const double absoluteDeltaY = qAbs(deltaY);
    if (absoluteDeltaX < threshold && absoluteDeltaY < threshold)
        return QStringLiteral("none");
    if (absoluteDeltaX >= absoluteDeltaY)
        return deltaX >= 0.0 ? QStringLiteral("right") : QStringLiteral("left");
    return deltaY >= 0.0 ? QStringLiteral("down") : QStringLiteral("up");
}

QVariantMap MobileEventSurfaceController::eventData(
    const QString& phase,
    const QString& kind,
    const QString& direction,
    const QVector<TouchPoint>& points,
    const bool valid) const
{
    const QPointF startCentroid = centroidForPoints(m_startPoints);
    const QPointF currentCentroid = points.isEmpty() ? centroidForPoints(m_currentPoints) : centroidForPoints(points);
    QVariantMap data;
    data.insert(QStringLiteral("valid"), valid);
    data.insert(QStringLiteral("phase"), phase);
    data.insert(QStringLiteral("kind"), kind);
    data.insert(QStringLiteral("direction"), direction);
    data.insert(QStringLiteral("fingerCount"), points.isEmpty() ? m_activeFingerCount : points.size());
    data.insert(QStringLiteral("sessionId"), m_activeSessionId);
    data.insert(QStringLiteral("deltaX"), currentCentroid.x() - startCentroid.x());
    data.insert(QStringLiteral("deltaY"), currentCentroid.y() - startCentroid.y());
    data.insert(QStringLiteral("centroidX"), currentCentroid.x());
    data.insert(QStringLiteral("centroidY"), currentCentroid.y());
    return data;
}

bool MobileEventSurfaceController::hasActiveSession(const int sessionId) const noexcept
{
    return m_activeSessionId >= 0 && sessionId == m_activeSessionId;
}

void MobileEventSurfaceController::setActiveSessionId(const int value)
{
    if (m_activeSessionId == value)
        return;
    m_activeSessionId = value;
    emit activeSessionIdChanged();
}

void MobileEventSurfaceController::setActiveEventKind(const QString& value)
{
    if (m_activeEventKind == value)
        return;
    m_activeEventKind = value;
    emit activeEventKindChanged();
}

void MobileEventSurfaceController::setActiveDirection(const QString& value)
{
    if (m_activeDirection == value)
        return;
    m_activeDirection = value;
    emit activeDirectionChanged();
}

void MobileEventSurfaceController::setActiveFingerCount(const int value)
{
    const int normalized = qMax(0, value);
    if (m_activeFingerCount == normalized)
        return;
    m_activeFingerCount = normalized;
    emit activeFingerCountChanged();
}

void MobileEventSurfaceController::emitRecognitionSignal(
    const QString& previousKind,
    const QString& nextKind,
    const QVariantMap& data)
{
    if (previousKind == nextKind)
        return;
    if (nextKind == QStringLiteral("scroll"))
    {
        emit scrollRecognized(data);
        return;
    }
    if (nextKind == QStringLiteral("gesture"))
        emit gestureRecognized(data);
}

void MobileEventSurfaceController::clearActiveSequence()
{
    m_startPoints.clear();
    m_currentPoints.clear();
    setActiveSessionId(-1);
    setActiveEventKind(QStringLiteral("idle"));
    setActiveDirection(QStringLiteral("none"));
    setActiveFingerCount(0);
}
