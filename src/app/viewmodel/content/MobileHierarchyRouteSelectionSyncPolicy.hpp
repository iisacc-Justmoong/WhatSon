#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class MobileHierarchyRouteSelectionSyncPolicy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hierarchyRoutePath READ hierarchyRoutePath WRITE setHierarchyRoutePath NOTIFY hierarchyRoutePathChanged)
    Q_PROPERTY(QString lastObservedRoutePath READ lastObservedRoutePath WRITE setLastObservedRoutePath NOTIFY lastObservedRoutePathChanged)
    Q_PROPERTY(bool routeSelectionSyncSuppressed READ routeSelectionSyncSuppressed WRITE setRouteSelectionSyncSuppressed NOTIFY routeSelectionSyncSuppressedChanged)

public:
    explicit MobileHierarchyRouteSelectionSyncPolicy(QObject* parent = nullptr);
    ~MobileHierarchyRouteSelectionSyncPolicy() override;

    QString hierarchyRoutePath() const;
    void setHierarchyRoutePath(const QString& value);

    QString lastObservedRoutePath() const;
    void setLastObservedRoutePath(const QString& value);

    bool routeSelectionSyncSuppressed() const noexcept;
    void setRouteSelectionSyncSuppressed(bool value);

    Q_INVOKABLE QVariantMap routeSelectionSyncPlan(bool hasRouter, const QString& displayedPath, const QString& routerCurrentPath, int depth) const;

signals:
    void hierarchyRoutePathChanged();
    void lastObservedRoutePathChanged();
    void routeSelectionSyncSuppressedChanged();

private:
    QString m_hierarchyRoutePath;
    QString m_lastObservedRoutePath;
    bool m_routeSelectionSyncSuppressed = false;
};
