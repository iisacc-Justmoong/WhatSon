#include "app/models/content/mobile/MobileHierarchyPopRepairPolicy.hpp"

namespace
{
    QString normalizedPath(const QString& value)
    {
        return value.trimmed();
    }
}

MobileHierarchyPopRepairPolicy::MobileHierarchyPopRepairPolicy(QObject* parent)
    : QObject(parent)
{
}

MobileHierarchyPopRepairPolicy::~MobileHierarchyPopRepairPolicy() = default;

QString MobileHierarchyPopRepairPolicy::noteListRoutePath() const { return m_noteListRoutePath; }
void MobileHierarchyPopRepairPolicy::setNoteListRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_noteListRoutePath == normalized)
        return;
    m_noteListRoutePath = normalized;
    emit noteListRoutePathChanged();
}

QString MobileHierarchyPopRepairPolicy::editorRoutePath() const { return m_editorRoutePath; }
void MobileHierarchyPopRepairPolicy::setEditorRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_editorRoutePath == normalized)
        return;
    m_editorRoutePath = normalized;
    emit editorRoutePathChanged();
}

QString MobileHierarchyPopRepairPolicy::detailRoutePath() const { return m_detailRoutePath; }
void MobileHierarchyPopRepairPolicy::setDetailRoutePath(const QString& value)
{
    const QString normalized = normalizedPath(value);
    if (m_detailRoutePath == normalized)
        return;
    m_detailRoutePath = normalized;
    emit detailRoutePathChanged();
}

int MobileHierarchyPopRepairPolicy::editorPopRepairRequestId() const noexcept { return m_editorPopRepairRequestId; }
void MobileHierarchyPopRepairPolicy::setEditorPopRepairRequestId(const int value)
{
    if (m_editorPopRepairRequestId == value)
        return;
    m_editorPopRepairRequestId = value;
    emit editorPopRepairRequestIdChanged();
}

int MobileHierarchyPopRepairPolicy::detailPopRepairRequestId() const noexcept { return m_detailPopRepairRequestId; }
void MobileHierarchyPopRepairPolicy::setDetailPopRepairRequestId(const int value)
{
    if (m_detailPopRepairRequestId == value)
        return;
    m_detailPopRepairRequestId = value;
    emit detailPopRepairRequestIdChanged();
}

QVariantMap MobileHierarchyPopRepairPolicy::repairVerificationPlan(const int requestId, const bool editorRepair, const bool hasRouter, const bool hasNoteListModel, const QString& displayedPath, const QString& currentPath, const int depth, const int attemptsRemaining) const
{
    QVariantMap plan;
    const int expectedRequestId = editorRepair ? m_editorPopRepairRequestId : m_detailPopRepairRequestId;
    const QString targetPath = editorRepair ? m_noteListRoutePath : m_editorRoutePath;
    const int minimumDepth = editorRepair ? 2 : 3;

    plan.insert(QStringLiteral("valid"), requestId == expectedRequestId && hasRouter && hasNoteListModel);
    plan.insert(QStringLiteral("done"), false);
    plan.insert(QStringLiteral("retry"), false);
    if (!(requestId == expectedRequestId && hasRouter && hasNoteListModel))
        return plan;

    if (normalizedPath(displayedPath) == targetPath
        || (normalizedPath(currentPath) == targetPath && depth >= minimumDepth))
    {
        plan.insert(QStringLiteral("done"), true);
        return plan;
    }

    if (attemptsRemaining > 0)
    {
        plan.insert(QStringLiteral("retry"), true);
        plan.insert(QStringLiteral("nextAttemptsRemaining"), attemptsRemaining - 1);
        return plan;
    }

    plan.insert(QStringLiteral("fallbackToCanonicalRoute"), true);
    return plan;
}

QVariantMap MobileHierarchyPopRepairPolicy::committedTransitionPlan(const QVariantMap& state, const bool hasNoteListModel, const QString& displayedPath) const
{
    QVariantMap plan;
    const QString operation = state.value(QStringLiteral("operation")).toString();
    const QString fromPath = normalizedPath(state.value(QStringLiteral("fromPath")).toString());

    plan.insert(QStringLiteral("requestViewHook"), true);
    plan.insert(QStringLiteral("rememberSelection"), operation == QStringLiteral("pop") && hasNoteListModel);
    plan.insert(QStringLiteral("scheduleRepair"), false);

    if (operation != QStringLiteral("pop") || !hasNoteListModel)
        return plan;

    if (fromPath == m_detailRoutePath)
    {
        plan.insert(QStringLiteral("cancelDetailRepair"), true);
        if (normalizedPath(displayedPath) != m_editorRoutePath)
        {
            plan.insert(QStringLiteral("scheduleRepair"), true);
            plan.insert(QStringLiteral("repairEditor"), false);
            plan.insert(QStringLiteral("repairRequestId"), m_detailPopRepairRequestId);
            plan.insert(QStringLiteral("attemptsRemaining"), 2);
        }
        return plan;
    }

    if (fromPath == m_editorRoutePath)
    {
        plan.insert(QStringLiteral("cancelEditorRepair"), true);
        if (normalizedPath(displayedPath) != m_noteListRoutePath)
        {
            plan.insert(QStringLiteral("scheduleRepair"), true);
            plan.insert(QStringLiteral("repairEditor"), true);
            plan.insert(QStringLiteral("repairRequestId"), m_editorPopRepairRequestId);
            plan.insert(QStringLiteral("attemptsRemaining"), 2);
        }
    }

    return plan;
}
