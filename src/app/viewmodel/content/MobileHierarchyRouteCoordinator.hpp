#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class MobileHierarchyRouteCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hierarchyRoutePath READ hierarchyRoutePath WRITE setHierarchyRoutePath NOTIFY hierarchyRoutePathChanged)
    Q_PROPERTY(QString noteListRoutePath READ noteListRoutePath WRITE setNoteListRoutePath NOTIFY noteListRoutePathChanged)
    Q_PROPERTY(QString editorRoutePath READ editorRoutePath WRITE setEditorRoutePath NOTIFY editorRoutePathChanged)
    Q_PROPERTY(QString detailRoutePath READ detailRoutePath WRITE setDetailRoutePath NOTIFY detailRoutePathChanged)
    Q_PROPERTY(QString lastObservedRoutePath READ lastObservedRoutePath WRITE setLastObservedRoutePath NOTIFY lastObservedRoutePathChanged)
    Q_PROPERTY(int preservedNoteListSelectionIndex READ preservedNoteListSelectionIndex WRITE setPreservedNoteListSelectionIndex NOTIFY preservedNoteListSelectionIndexChanged)
    Q_PROPERTY(bool routeSelectionSyncSuppressed READ routeSelectionSyncSuppressed WRITE setRouteSelectionSyncSuppressed NOTIFY routeSelectionSyncSuppressedChanged)
    Q_PROPERTY(int editorPopRepairRequestId READ editorPopRepairRequestId WRITE setEditorPopRepairRequestId NOTIFY editorPopRepairRequestIdChanged)
    Q_PROPERTY(int detailPopRepairRequestId READ detailPopRepairRequestId WRITE setDetailPopRepairRequestId NOTIFY detailPopRepairRequestIdChanged)

public:
    explicit MobileHierarchyRouteCoordinator(QObject* parent = nullptr);
    ~MobileHierarchyRouteCoordinator() override;

    QString hierarchyRoutePath() const;
    void setHierarchyRoutePath(const QString& value);

    QString noteListRoutePath() const;
    void setNoteListRoutePath(const QString& value);

    QString editorRoutePath() const;
    void setEditorRoutePath(const QString& value);

    QString detailRoutePath() const;
    void setDetailRoutePath(const QString& value);

    QString lastObservedRoutePath() const;
    void setLastObservedRoutePath(const QString& value);

    int preservedNoteListSelectionIndex() const noexcept;
    void setPreservedNoteListSelectionIndex(int value);

    bool routeSelectionSyncSuppressed() const noexcept;
    void setRouteSelectionSyncSuppressed(bool value);

    int editorPopRepairRequestId() const noexcept;
    void setEditorPopRepairRequestId(int value);

    int detailPopRepairRequestId() const noexcept;
    void setDetailPopRepairRequestId(int value);

    Q_INVOKABLE int normalizedInteger(const QVariant& value, int fallbackValue) const;
    Q_INVOKABLE int normalizedSelectionIndex(const QVariant& value) const;
    Q_INVOKABLE int rememberSelectionIndex(const QVariant& explicitSelectionIndex, int currentHierarchySelectionIndex);
    Q_INVOKABLE int resolvedSelectionRestoreTarget(const QVariant& explicitSelectionIndex) const;
    Q_INVOKABLE int routeStackDepth(const QVariant& depthValue) const;
    Q_INVOKABLE QVariantMap canonicalRoutePlan(const QString& targetPath, bool hasRouter, bool hasNoteListModel, bool transitionActive, const QString& displayedPath) const;
    Q_INVOKABLE QVariantMap directPushPlan(const QString& currentPath, const QString& displayedPath, int depth, const QString& targetPath) const;
    Q_INVOKABLE QVariantMap repairVerificationPlan(int requestId, bool editorRepair, bool hasRouter, bool hasNoteListModel, const QString& displayedPath, const QString& currentPath, int depth, int attemptsRemaining) const;
    Q_INVOKABLE QVariantMap committedTransitionPlan(const QVariantMap& state, bool hasNoteListModel, const QString& displayedPath) const;
    Q_INVOKABLE QVariantMap hierarchyRootPlan(bool hasRouter, bool transitionActive, const QString& displayedPath, bool canGoBack) const;
    Q_INVOKABLE QVariantMap routeSelectionSyncPlan(bool hasRouter, const QString& displayedPath, const QString& routerCurrentPath, int depth) const;

signals:
    void hierarchyRoutePathChanged();
    void noteListRoutePathChanged();
    void editorRoutePathChanged();
    void detailRoutePathChanged();
    void lastObservedRoutePathChanged();
    void preservedNoteListSelectionIndexChanged();
    void routeSelectionSyncSuppressedChanged();
    void editorPopRepairRequestIdChanged();
    void detailPopRepairRequestIdChanged();

private:
    QString m_hierarchyRoutePath;
    QString m_noteListRoutePath;
    QString m_editorRoutePath;
    QString m_detailRoutePath;
    QString m_lastObservedRoutePath;
    int m_preservedNoteListSelectionIndex = -1;
    bool m_routeSelectionSyncSuppressed = false;
    int m_editorPopRepairRequestId = 0;
    int m_detailPopRepairRequestId = 0;
};
