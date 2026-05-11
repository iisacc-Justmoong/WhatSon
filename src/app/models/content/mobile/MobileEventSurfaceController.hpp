#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class MobileEventSurfaceController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int generatedSessionId READ generatedSessionId WRITE setGeneratedSessionId NOTIFY generatedSessionIdChanged)
    Q_PROPERTY(int activeSessionId READ activeSessionId NOTIFY activeSessionIdChanged)
    Q_PROPERTY(QString activeEventKind READ activeEventKind NOTIFY activeEventKindChanged)
    Q_PROPERTY(QString activeDirection READ activeDirection NOTIFY activeDirectionChanged)
    Q_PROPERTY(int activeFingerCount READ activeFingerCount NOTIFY activeFingerCountChanged)
    Q_PROPERTY(double tapMoveThreshold READ tapMoveThreshold WRITE setTapMoveThreshold NOTIFY tapMoveThresholdChanged)
    Q_PROPERTY(double scrollDistanceThreshold READ scrollDistanceThreshold WRITE setScrollDistanceThreshold NOTIFY scrollDistanceThresholdChanged)
    Q_PROPERTY(double gestureDistanceThreshold READ gestureDistanceThreshold WRITE setGestureDistanceThreshold NOTIFY gestureDistanceThresholdChanged)

public:
    explicit MobileEventSurfaceController(QObject* parent = nullptr);
    ~MobileEventSurfaceController() override;

    int generatedSessionId() const noexcept;
    void setGeneratedSessionId(int value);

    int activeSessionId() const noexcept;
    QString activeEventKind() const;
    QString activeDirection() const;
    int activeFingerCount() const noexcept;

    double tapMoveThreshold() const noexcept;
    void setTapMoveThreshold(double value);

    double scrollDistanceThreshold() const noexcept;
    void setScrollDistanceThreshold(double value);

    double gestureDistanceThreshold() const noexcept;
    void setGestureDistanceThreshold(double value);

    Q_INVOKABLE int nextSessionId();
    Q_INVOKABLE QVariantMap beginTouchSequence(int sessionId, const QVariantList& points);
    Q_INVOKABLE QVariantMap updateTouchSequence(int sessionId, const QVariantList& points);
    Q_INVOKABLE QVariantMap endTouchSequence(int sessionId, const QVariantList& points, bool cancelled);
    Q_INVOKABLE QVariantMap cancelTouchSequence(int sessionId);
    Q_INVOKABLE void resetState();

signals:
    void generatedSessionIdChanged();
    void activeSessionIdChanged();
    void activeEventKindChanged();
    void activeDirectionChanged();
    void activeFingerCountChanged();
    void tapMoveThresholdChanged();
    void scrollDistanceThresholdChanged();
    void gestureDistanceThresholdChanged();
    void touchRecognized(const QVariantMap& eventData);
    void scrollRecognized(const QVariantMap& eventData);
    void gestureRecognized(const QVariantMap& eventData);
    void classificationChanged(const QVariantMap& eventData);

private:
    struct TouchPoint
    {
        int id = -1;
        QPointF position;
    };

    static QVector<TouchPoint> pointsFromVariantList(const QVariantList& points);
    static QPointF centroidForPoints(const QVector<TouchPoint>& points);
    static QString directionForDelta(double deltaX, double deltaY, double threshold);

    QVariantMap eventData(
        const QString& phase,
        const QString& kind,
        const QString& direction,
        const QVector<TouchPoint>& points,
        bool valid) const;
    bool hasActiveSession(int sessionId) const noexcept;
    void setActiveSessionId(int value);
    void setActiveEventKind(const QString& value);
    void setActiveDirection(const QString& value);
    void setActiveFingerCount(int value);
    void emitRecognitionSignal(const QString& previousKind, const QString& nextKind, const QVariantMap& data);
    void clearActiveSequence();

    int m_generatedSessionId = 0;
    int m_activeSessionId = -1;
    QString m_activeEventKind = QStringLiteral("idle");
    QString m_activeDirection = QStringLiteral("none");
    int m_activeFingerCount = 0;
    double m_tapMoveThreshold = 8.0;
    double m_scrollDistanceThreshold = 12.0;
    double m_gestureDistanceThreshold = 16.0;
    QVector<TouchPoint> m_startPoints;
    QVector<TouchPoint> m_currentPoints;
};
