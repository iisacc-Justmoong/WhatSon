#include "MobileHierarchyRouteStateStore.hpp"

MobileHierarchyRouteStateStore::MobileHierarchyRouteStateStore(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyRouteStateStore::~MobileHierarchyRouteStateStore() = default;

QString MobileHierarchyRouteStateStore::lastObservedRoutePath() const { return m_lastObservedRoutePath; }
void MobileHierarchyRouteStateStore::setLastObservedRoutePath(const QString& value)
{
    const QString normalized = value.trimmed();
    if (m_lastObservedRoutePath == normalized)
        return;
    m_lastObservedRoutePath = normalized;
    emit lastObservedRoutePathChanged();
}

int MobileHierarchyRouteStateStore::preservedNoteListSelectionIndex() const noexcept { return m_preservedNoteListSelectionIndex; }
void MobileHierarchyRouteStateStore::setPreservedNoteListSelectionIndex(const int value)
{
    if (m_preservedNoteListSelectionIndex == value)
        return;
    m_preservedNoteListSelectionIndex = value;
    emit preservedNoteListSelectionIndexChanged();
}

bool MobileHierarchyRouteStateStore::routeSelectionSyncSuppressed() const noexcept { return m_routeSelectionSyncSuppressed; }
void MobileHierarchyRouteStateStore::setRouteSelectionSyncSuppressed(const bool value)
{
    if (m_routeSelectionSyncSuppressed == value)
        return;
    m_routeSelectionSyncSuppressed = value;
    emit routeSelectionSyncSuppressedChanged();
}

int MobileHierarchyRouteStateStore::editorPopRepairRequestId() const noexcept { return m_editorPopRepairRequestId; }
void MobileHierarchyRouteStateStore::setEditorPopRepairRequestId(const int value)
{
    if (m_editorPopRepairRequestId == value)
        return;
    m_editorPopRepairRequestId = value;
    emit editorPopRepairRequestIdChanged();
}

int MobileHierarchyRouteStateStore::detailPopRepairRequestId() const noexcept { return m_detailPopRepairRequestId; }
void MobileHierarchyRouteStateStore::setDetailPopRepairRequestId(const int value)
{
    if (m_detailPopRepairRequestId == value)
        return;
    m_detailPopRepairRequestId = value;
    emit detailPopRepairRequestIdChanged();
}

int MobileHierarchyRouteStateStore::normalizedInteger(const QVariant& value, const int fallbackValue) const
{
    bool ok = false;
    const int converted = value.toInt(&ok);
    return ok ? converted : fallbackValue;
}

int MobileHierarchyRouteStateStore::rememberSelectionIndex(const QVariant& explicitSelectionIndex, const int currentHierarchySelectionIndex)
{
    int nextSelectionIndex = normalizedInteger(explicitSelectionIndex, -1);
    if (nextSelectionIndex < 0)
        nextSelectionIndex = currentHierarchySelectionIndex;
    setPreservedNoteListSelectionIndex(nextSelectionIndex);
    return nextSelectionIndex;
}

int MobileHierarchyRouteStateStore::resolvedSelectionRestoreTarget(const QVariant& explicitSelectionIndex) const
{
    const int explicitIndex = normalizedInteger(explicitSelectionIndex, -1);
    return explicitIndex >= 0 ? explicitIndex : m_preservedNoteListSelectionIndex;
}
