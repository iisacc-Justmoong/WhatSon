#include "MobileHierarchySelectionCoordinator.hpp"

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

QVariantMap MobileHierarchySelectionCoordinator::activeHierarchyBindingSnapshotFromSidebar(const QVariant& sidebarViewModel) const
{
    QVariantMap snapshot;
    snapshot.insert(QStringLiteral("index"), 0);
    snapshot.insert(QStringLiteral("viewModel"), QVariant());

    QObject* sidebarObject = sidebarViewModel.value<QObject*>();
    if (!sidebarObject)
        return snapshot;

    bool ok = false;
    const int resolvedIndex = sidebarObject->property("resolvedActiveHierarchyIndex").toInt(&ok);
    const int safeIndex = ok ? resolvedIndex : 0;

    QVariant resolvedHierarchyViewModel = sidebarObject->property("resolvedHierarchyViewModel");
    const QMetaObject* metaObject = sidebarObject->metaObject();
    const int methodIndex = metaObject ? metaObject->indexOfMethod("hierarchyViewModelForIndex(QVariant)") : -1;
    if (methodIndex >= 0)
    {
        QVariant returnedValue;
        QMetaMethod method = metaObject->method(methodIndex);
        method.invoke(sidebarObject,
                      Q_RETURN_ARG(QVariant, returnedValue),
                      Q_ARG(QVariant, QVariant(safeIndex)));
        if (returnedValue.isValid())
            resolvedHierarchyViewModel = returnedValue;
    }

    snapshot.insert(QStringLiteral("index"), safeIndex);
    snapshot.insert(QStringLiteral("viewModel"), resolvedHierarchyViewModel);
    return snapshot;
}

int MobileHierarchySelectionCoordinator::currentHierarchySelectionIndex(const QVariant& activeContentViewModel, const int preservedSelectionIndex) const
{
    QObject* viewModelObject = activeContentViewModel.value<QObject*>();
    if (!viewModelObject)
        return preservedSelectionIndex;
    const QVariant selectedIndex = viewModelObject->property("hierarchySelectedIndex");
    bool ok = false;
    const int resolvedIndex = selectedIndex.toInt(&ok);
    return ok ? resolvedIndex : preservedSelectionIndex;
}
