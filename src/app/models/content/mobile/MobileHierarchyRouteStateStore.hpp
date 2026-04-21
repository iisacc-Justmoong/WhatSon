#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class MobileHierarchyRouteStateStore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastObservedRoutePath READ lastObservedRoutePath WRITE setLastObservedRoutePath NOTIFY lastObservedRoutePathChanged)
    Q_PROPERTY(int preservedNoteListSelectionIndex READ preservedNoteListSelectionIndex WRITE setPreservedNoteListSelectionIndex NOTIFY preservedNoteListSelectionIndexChanged)
    Q_PROPERTY(bool routeSelectionSyncSuppressed READ routeSelectionSyncSuppressed WRITE setRouteSelectionSyncSuppressed NOTIFY routeSelectionSyncSuppressedChanged)
    Q_PROPERTY(int editorPopRepairRequestId READ editorPopRepairRequestId WRITE setEditorPopRepairRequestId NOTIFY editorPopRepairRequestIdChanged)
    Q_PROPERTY(int detailPopRepairRequestId READ detailPopRepairRequestId WRITE setDetailPopRepairRequestId NOTIFY detailPopRepairRequestIdChanged)

public:
    explicit MobileHierarchyRouteStateStore(QObject* parent = nullptr);
    ~MobileHierarchyRouteStateStore() override;

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
    Q_INVOKABLE int rememberSelectionIndex(const QVariant& explicitSelectionIndex, int currentHierarchySelectionIndex);
    Q_INVOKABLE int resolvedSelectionRestoreTarget(const QVariant& explicitSelectionIndex) const;

signals:
    void lastObservedRoutePathChanged();
    void preservedNoteListSelectionIndexChanged();
    void routeSelectionSyncSuppressedChanged();
    void editorPopRepairRequestIdChanged();
    void detailPopRepairRequestIdChanged();

private:
    QString m_lastObservedRoutePath;
    int m_preservedNoteListSelectionIndex = -1;
    bool m_routeSelectionSyncSuppressed = false;
    int m_editorPopRepairRequestId = 0;
    int m_detailPopRepairRequestId = 0;
};
