#include "MobileHierarchyNavigationCoordinator.hpp"

namespace
{
    QString normalizedPath(const QString& value)
    {
        return value.trimmed();
    }
}

MobileHierarchyNavigationCoordinator::MobileHierarchyNavigationCoordinator(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyNavigationCoordinator::~MobileHierarchyNavigationCoordinator() = default;

QString MobileHierarchyNavigationCoordinator::hierarchyRoutePath() const { return m_hierarchyRoutePath; }
void MobileHierarchyNavigationCoordinator::setHierarchyRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_hierarchyRoutePath == normalized)
        return;
    m_hierarchyRoutePath = normalized;
    emit hierarchyRoutePathChanged();
}

QString MobileHierarchyNavigationCoordinator::noteListRoutePath() const { return m_noteListRoutePath; }
void MobileHierarchyNavigationCoordinator::setNoteListRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_noteListRoutePath == normalized)
        return;
    m_noteListRoutePath = normalized;
    emit noteListRoutePathChanged();
}

QString MobileHierarchyNavigationCoordinator::editorRoutePath() const { return m_editorRoutePath; }
void MobileHierarchyNavigationCoordinator::setEditorRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_editorRoutePath == normalized)
        return;
    m_editorRoutePath = normalized;
    emit editorRoutePathChanged();
}

QString MobileHierarchyNavigationCoordinator::detailRoutePath() const { return m_detailRoutePath; }
void MobileHierarchyNavigationCoordinator::setDetailRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_detailRoutePath == normalized)
        return;
    m_detailRoutePath = normalized;
    emit detailRoutePathChanged();
}

QString MobileHierarchyNavigationCoordinator::displayedBodyRoutePath(const QVariant& bodyItem, const QVariant& activePageRouter) const
{
    QObject* bodyObject = bodyItem.value<QObject*>();
    if (bodyObject)
    {
        if (bodyObject->property("detailPanelPage").isValid())
            return m_detailRoutePath;
        if (bodyObject->property("contentViewModel").isValid())
            return m_editorRoutePath;
        if (bodyObject->property("noteListModel").isValid())
            return m_noteListRoutePath;
        if (bodyObject->property("sidebarHierarchyViewModel").isValid())
            return m_hierarchyRoutePath;
    }

    QObject* routerObject = activePageRouter.value<QObject*>();
    if (!routerObject)
        return QString();
    return routerObject->property("currentPath").toString();
}

QVariantMap MobileHierarchyNavigationCoordinator::openDetailPanelPlan(const bool hasRouter, const bool hasNoteListModel, const QString& currentPath, const QString& displayedPath, const int depth) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("allowed"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("alreadyOpen"), normalizedPath(currentPath) == m_detailRoutePath && normalizedPath(displayedPath) == m_detailRoutePath && depth >= 4);
    plan.insert(QStringLiteral("directPush"), normalizedPath(currentPath) == m_editorRoutePath && normalizedPath(displayedPath) == m_editorRoutePath && depth >= 3);
    return plan;
}

QVariantMap MobileHierarchyNavigationCoordinator::openNoteListPlan(const bool hasRouter, const bool hasNoteListModel, const QString& currentPath, const QString& displayedPath, const int depth) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("allowed"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("alreadyOpen"), normalizedPath(currentPath) == m_noteListRoutePath && normalizedPath(displayedPath) == m_noteListRoutePath && depth >= 2);
    plan.insert(QStringLiteral("directPush"), normalizedPath(currentPath) == m_hierarchyRoutePath && normalizedPath(displayedPath) == m_hierarchyRoutePath && depth <= 1);
    return plan;
}

QVariantMap MobileHierarchyNavigationCoordinator::openEditorPlan(const QVariant& noteId, const bool hasActiveContentViewModel, const bool hasNoteListModel, const bool hasRouter, const QString& currentPath, const QString& displayedPath, const int depth) const
{
    QVariantMap plan;
    const QString normalizedNoteId = noteId.toString().trimmed();
    const bool allowed = !normalizedNoteId.isEmpty() && hasActiveContentViewModel && hasNoteListModel && hasRouter;
    plan.insert(QStringLiteral("allowed"), allowed);
    plan.insert(QStringLiteral("alreadyOpen"), normalizedPath(currentPath) == m_editorRoutePath && normalizedPath(displayedPath) == m_editorRoutePath && depth >= 3);
    plan.insert(QStringLiteral("directPush"), normalizedPath(currentPath) == m_noteListRoutePath && normalizedPath(displayedPath) == m_noteListRoutePath && depth >= 2);
    return plan;
}

QVariantMap MobileHierarchyNavigationCoordinator::calendarSurfacePlan(const bool hasRouter, const bool hasNoteListModel, const QString& displayedPath) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("alreadyVisible"), normalizedPath(displayedPath) == m_editorRoutePath);
    plan.insert(QStringLiteral("routeToEditor"), hasRouter && hasNoteListModel && normalizedPath(displayedPath) != m_editorRoutePath);
    plan.insert(QStringLiteral("allowed"), hasRouter);
    return plan;
}

QVariantMap MobileHierarchyNavigationCoordinator::overlayDismissPlan(const bool agendaVisible, const bool dayVisible, const bool weekVisible, const bool monthVisible, const bool yearVisible) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("dismissAgenda"), agendaVisible);
    plan.insert(QStringLiteral("dismissDay"), dayVisible);
    plan.insert(QStringLiteral("dismissWeek"), weekVisible);
    plan.insert(QStringLiteral("dismissMonth"), monthVisible);
    plan.insert(QStringLiteral("dismissYear"), yearVisible);
    return plan;
}
