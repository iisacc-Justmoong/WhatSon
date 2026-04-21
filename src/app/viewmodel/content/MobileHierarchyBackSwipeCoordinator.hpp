#pragma once

#include <QObject>
#include <QVariantMap>

class MobileHierarchyBackSwipeCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int backSwipeConsumedSessionId READ backSwipeConsumedSessionId WRITE setBackSwipeConsumedSessionId NOTIFY backSwipeConsumedSessionIdChanged)
    Q_PROPERTY(int backSwipeSessionId READ backSwipeSessionId WRITE setBackSwipeSessionId NOTIFY backSwipeSessionIdChanged)
    Q_PROPERTY(int backSwipeGeneratedSessionId READ backSwipeGeneratedSessionId WRITE setBackSwipeGeneratedSessionId NOTIFY backSwipeGeneratedSessionIdChanged)
    Q_PROPERTY(int backSwipeEdgeWidth READ backSwipeEdgeWidth WRITE setBackSwipeEdgeWidth NOTIFY backSwipeEdgeWidthChanged)

public:
    explicit MobileHierarchyBackSwipeCoordinator(QObject* parent = nullptr);
    ~MobileHierarchyBackSwipeCoordinator() override;

    int backSwipeConsumedSessionId() const noexcept;
    void setBackSwipeConsumedSessionId(int value);

    int backSwipeSessionId() const noexcept;
    void setBackSwipeSessionId(int value);

    int backSwipeGeneratedSessionId() const noexcept;
    void setBackSwipeGeneratedSessionId(int value);

    int backSwipeEdgeWidth() const noexcept;
    void setBackSwipeEdgeWidth(int value);

    Q_INVOKABLE int nextGeneratedSessionId();
    Q_INVOKABLE QVariantMap gestureEventData(double localX, double localY, double totalDeltaX, double totalDeltaY, int sessionId, double mappedGlobalX, double mappedGlobalY) const;
    Q_INVOKABLE QVariantMap beginGesturePlan(bool backNavigationAvailable, bool transitionActive, const QVariantMap& eventData, double edgeOriginX) const;
    Q_INVOKABLE QVariantMap cancelGesturePlan(const QVariantMap& eventData) const;
    Q_INVOKABLE QVariantMap finishGesturePlan(const QVariantMap& eventData, bool cancelled, bool transitionActive, double transitionProgress) const;
    Q_INVOKABLE QVariantMap updateGesturePlan(const QVariantMap& eventData, bool transitionActive, double viewportWidth, double cancelVerticalThreshold) const;
    Q_INVOKABLE void resetState();

signals:
    void backSwipeConsumedSessionIdChanged();
    void backSwipeSessionIdChanged();
    void backSwipeGeneratedSessionIdChanged();
    void backSwipeEdgeWidthChanged();

private:
    int m_backSwipeConsumedSessionId = -1;
    int m_backSwipeSessionId = -1;
    int m_backSwipeGeneratedSessionId = 0;
    int m_backSwipeEdgeWidth = 0;
};
