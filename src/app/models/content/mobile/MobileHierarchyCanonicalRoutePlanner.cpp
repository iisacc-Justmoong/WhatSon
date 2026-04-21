#include "app/models/content/mobile/MobileHierarchyCanonicalRoutePlanner.hpp"

#include <QtGlobal>

namespace
{
    QString normalizedPath(const QString& value)
    {
        return value.trimmed();
    }
}

MobileHierarchyCanonicalRoutePlanner::MobileHierarchyCanonicalRoutePlanner(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyCanonicalRoutePlanner::~MobileHierarchyCanonicalRoutePlanner() = default;

QString MobileHierarchyCanonicalRoutePlanner::hierarchyRoutePath() const { return m_hierarchyRoutePath; }
void MobileHierarchyCanonicalRoutePlanner::setHierarchyRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_hierarchyRoutePath == normalized)
        return;
    m_hierarchyRoutePath = normalized;
    emit hierarchyRoutePathChanged();
}

QString MobileHierarchyCanonicalRoutePlanner::noteListRoutePath() const { return m_noteListRoutePath; }
void MobileHierarchyCanonicalRoutePlanner::setNoteListRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_noteListRoutePath == normalized)
        return;
    m_noteListRoutePath = normalized;
    emit noteListRoutePathChanged();
}

QString MobileHierarchyCanonicalRoutePlanner::editorRoutePath() const { return m_editorRoutePath; }
void MobileHierarchyCanonicalRoutePlanner::setEditorRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_editorRoutePath == normalized)
        return;
    m_editorRoutePath = normalized;
    emit editorRoutePathChanged();
}

QString MobileHierarchyCanonicalRoutePlanner::detailRoutePath() const { return m_detailRoutePath; }
void MobileHierarchyCanonicalRoutePlanner::setDetailRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_detailRoutePath == normalized)
        return;
    m_detailRoutePath = normalized;
    emit detailRoutePathChanged();
}

int MobileHierarchyCanonicalRoutePlanner::routeStackDepth(const QVariant& depthValue) const
{
    bool ok = false;
    const int depth = depthValue.toInt(&ok);
    return ok ? qMax(0, depth) : 0;
}

QVariantMap MobileHierarchyCanonicalRoutePlanner::canonicalRoutePlan(const QString& targetPath, const bool hasRouter, const bool hasNoteListModel, const bool transitionActive) const
{
    QVariantMap plan;
    plan.insert(QStringLiteral("allowed"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("cancelTransition"), transitionActive);
    plan.insert(QStringLiteral("resetToRoot"), hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("requestViewHook"), hasRouter && hasNoteListModel);

    QVariantList pushPaths;
    if (hasRouter && hasNoteListModel)
    {
        pushPaths.push_back(m_noteListRoutePath);
        if (normalizedPath(targetPath) == m_editorRoutePath || normalizedPath(targetPath) == m_detailRoutePath)
            pushPaths.push_back(m_editorRoutePath);
        if (normalizedPath(targetPath) == m_detailRoutePath)
            pushPaths.push_back(m_detailRoutePath);
    }
    plan.insert(QStringLiteral("pushPaths"), pushPaths);
    return plan;
}

QVariantMap MobileHierarchyCanonicalRoutePlanner::hierarchyRootPlan(const bool hasRouter, const bool transitionActive, const QString& displayedPath, const bool canGoBack) const
{
    QVariantMap plan;
    const bool alreadyRoot = normalizedPath(displayedPath) == m_hierarchyRoutePath && !canGoBack;
    plan.insert(QStringLiteral("allowed"), hasRouter);
    plan.insert(QStringLiteral("cancelTransition"), transitionActive);
    plan.insert(QStringLiteral("alreadyRoot"), alreadyRoot);
    plan.insert(QStringLiteral("setRoot"), hasRouter && !alreadyRoot);
    plan.insert(QStringLiteral("requestViewHook"), hasRouter && !alreadyRoot);
    return plan;
}
