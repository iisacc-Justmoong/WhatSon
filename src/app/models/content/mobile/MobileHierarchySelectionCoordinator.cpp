#include "app/models/content/mobile/MobileHierarchySelectionCoordinator.hpp"

#include <QMetaMethod>
#include <QVariant>

MobileHierarchySelectionCoordinator::MobileHierarchySelectionCoordinator(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchySelectionCoordinator::~MobileHierarchySelectionCoordinator() = default;

QVariantMap MobileHierarchySelectionCoordinator::activeHierarchyBindingSnapshot() const
{
    return m_activeHierarchyBindingSnapshot;
}

void MobileHierarchySelectionCoordinator::setActiveHierarchyBindingSnapshot(const QVariantMap& value)
{
    if (m_activeHierarchyBindingSnapshot == value)
        return;
    m_activeHierarchyBindingSnapshot = value;
    emit activeHierarchyBindingSnapshotChanged();
}

QVariantMap MobileHierarchySelectionCoordinator::activeHierarchyBindingSnapshotFromSidebar(const QVariant& sidebarController) const
{
    QVariantMap snapshot;
    snapshot.insert(QStringLiteral("index"), 0);
    snapshot.insert(QStringLiteral("controller"), QVariant());

    QObject* sidebarObject = sidebarController.value<QObject*>();
    if (!sidebarObject)
        return snapshot;

    bool ok = false;
    const int resolvedIndex = sidebarObject->property("resolvedActiveHierarchyIndex").toInt(&ok);
    const int safeIndex = ok ? resolvedIndex : 0;

    QVariant resolvedHierarchyController = sidebarObject->property("resolvedHierarchyController");
    const QMetaObject* metaObject = sidebarObject->metaObject();
    const int methodIndex = metaObject ? metaObject->indexOfMethod("hierarchyControllerForIndex(QVariant)") : -1;
    if (methodIndex >= 0)
    {
        QVariant returnedValue;
        QMetaMethod method = metaObject->method(methodIndex);
        method.invoke(sidebarObject,
                      Q_RETURN_ARG(QVariant, returnedValue),
                      Q_ARG(QVariant, QVariant(safeIndex)));
        if (returnedValue.isValid())
            resolvedHierarchyController = returnedValue;
    }

    snapshot.insert(QStringLiteral("index"), safeIndex);
    snapshot.insert(QStringLiteral("controller"), resolvedHierarchyController);
    return snapshot;
}

int MobileHierarchySelectionCoordinator::currentHierarchySelectionIndex(const QVariant& activeContentController, const int preservedSelectionIndex) const
{
    QObject* controllerObject = activeContentController.value<QObject*>();
    if (!controllerObject)
        return preservedSelectionIndex;
    const QVariant selectedIndex = controllerObject->property("hierarchySelectedIndex");
    bool ok = false;
    const int resolvedIndex = selectedIndex.toInt(&ok);
    return ok ? resolvedIndex : preservedSelectionIndex;
}
