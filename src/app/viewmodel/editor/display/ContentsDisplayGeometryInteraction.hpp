#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QVariantList>

class ContentsDisplayGeometryInteraction : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentsView READ contentsView WRITE setContentsView NOTIFY contentsViewChanged)
    Q_PROPERTY(QObject* eventPump READ eventPump WRITE setEventPump NOTIFY eventPumpChanged)
    Q_PROPERTY(QObject* minimapLayer READ minimapLayer WRITE setMinimapLayer NOTIFY minimapLayerChanged)
    Q_PROPERTY(QObject* refreshCoordinator READ refreshCoordinator WRITE setRefreshCoordinator NOTIFY refreshCoordinatorChanged)
    Q_PROPERTY(QObject* viewportCoordinator READ viewportCoordinator WRITE setViewportCoordinator NOTIFY viewportCoordinatorChanged)

public:
    explicit ContentsDisplayGeometryInteraction(QObject* parent = nullptr);
    ~ContentsDisplayGeometryInteraction() override;

    QObject* contentsView() const noexcept;
    void setContentsView(QObject* value);
    QObject* eventPump() const noexcept;
    void setEventPump(QObject* value);
    QObject* minimapLayer() const noexcept;
    void setMinimapLayer(QObject* value);
    QObject* refreshCoordinator() const noexcept;
    void setRefreshCoordinator(QObject* value);
    QObject* viewportCoordinator() const noexcept;
    void setViewportCoordinator(QObject* value);

    Q_INVOKABLE void refreshMinimapSnapshot() const;
    Q_INVOKABLE void refreshMinimapCursorTracking(const QVariant& rowsOverride = {}) const;
    Q_INVOKABLE void refreshMinimapViewportTracking(const QVariant& trackHeightOverride = {}) const;
    Q_INVOKABLE void resetNoteEntryLineGeometryState() const;
    Q_INVOKABLE void resetGutterRefreshState() const;
    Q_INVOKABLE bool refreshLiveLogicalLineMetrics() const;
    Q_INVOKABLE int activeLogicalLineCountSnapshot() const;
    Q_INVOKABLE void scheduleGutterRefresh(int passCount, const QString& reason) const;
    Q_INVOKABLE void scheduleNoteEntryGutterRefresh(const QString& noteId) const;
    Q_INVOKABLE void scheduleCursorDrivenUiRefresh() const;
    Q_INVOKABLE void scheduleViewportGutterRefresh() const;
    Q_INVOKABLE void scheduleMinimapSnapshotRefresh(bool forceFull) const;
    Q_INVOKABLE void scrollEditorViewportToMinimapPosition(double localY) const;
    Q_INVOKABLE void correctTypingViewport(bool forceAnchor) const;
    Q_INVOKABLE void scheduleTypingViewportCorrection(bool forceAnchor) const;

signals:
    void contentsViewChanged();
    void eventPumpChanged();
    void minimapLayerChanged();
    void refreshCoordinatorChanged();
    void viewportCoordinatorChanged();

private:
    QVariant invoke(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    bool invokeBool(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    void invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    QVariant property(QObject* target, const char* propertyName) const;
    void setProperty(QObject* target, const char* propertyName, const QVariant& value) const;
    QVariantMap safeMap(const QVariant& value) const;

    QPointer<QObject> m_contentsView;
    QPointer<QObject> m_eventPump;
    QPointer<QObject> m_minimapLayer;
    QPointer<QObject> m_refreshCoordinator;
    QPointer<QObject> m_viewportCoordinator;
};
