#pragma once

#include <QObject>
#include <QVariantMap>

class MobileHierarchySelectionCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap activeHierarchyBindingSnapshot READ activeHierarchyBindingSnapshot WRITE setActiveHierarchyBindingSnapshot NOTIFY activeHierarchyBindingSnapshotChanged)

public:
    explicit MobileHierarchySelectionCoordinator(QObject* parent = nullptr);
    ~MobileHierarchySelectionCoordinator() override;

    QVariantMap activeHierarchyBindingSnapshot() const;
    void setActiveHierarchyBindingSnapshot(const QVariantMap& value);

    Q_INVOKABLE QVariantMap activeHierarchyBindingSnapshotFromSidebar(const QVariant& sidebarViewModel) const;
    Q_INVOKABLE int currentHierarchySelectionIndex(const QVariant& activeContentViewModel, int preservedSelectionIndex) const;

signals:
    void activeHierarchyBindingSnapshotChanged();

private:
    QVariantMap m_activeHierarchyBindingSnapshot;
};
