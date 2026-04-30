#include "app/runtime/bootstrap/WhatSonQmlContextBinder.hpp"

namespace WhatSon::Runtime::Bootstrap
{
namespace
{
    void appendContextObjectBinding(lvrs::QmlContextBindPlan& plan, const QString& contextName, QObject* object)
    {
        lvrs::QmlContextObjectBinding binding;
        binding.contextName = contextName;
        binding.object = object;
        plan.contextObjects.append(binding);
    }

} // namespace

    lvrs::QmlContextBindResult bindWorkspaceContextObjects(
        QQmlApplicationEngine& engine,
        const WorkspaceContextObjects& objects)
    {
        lvrs::QmlContextBindPlan plan;

        appendContextObjectBinding(plan, QStringLiteral("libraryHierarchyController"), objects.libraryHierarchyController);
        appendContextObjectBinding(
            plan,
            QStringLiteral("libraryNoteMutationController"),
            objects.libraryNoteMutationController);
        appendContextObjectBinding(plan, QStringLiteral("projectsHierarchyController"), objects.projectsHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("bookmarksHierarchyController"), objects.bookmarksHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("tagsHierarchyController"), objects.tagsHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("resourcesHierarchyController"), objects.resourcesHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("progressHierarchyController"), objects.progressHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("eventHierarchyController"), objects.eventHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("presetHierarchyController"), objects.presetHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("detailPanelController"), objects.detailPanelController);
        appendContextObjectBinding(plan, QStringLiteral("noteDetailPanelController"), objects.noteDetailPanelController);
        appendContextObjectBinding(
            plan,
            QStringLiteral("resourceDetailPanelController"),
            objects.resourceDetailPanelController);
        appendContextObjectBinding(plan, QStringLiteral("editorViewModeController"), objects.editorViewModeController);
        appendContextObjectBinding(plan, QStringLiteral("navigationModeController"), objects.navigationModeController);
        appendContextObjectBinding(plan, QStringLiteral("sidebarHierarchyController"), objects.sidebarHierarchyController);
        appendContextObjectBinding(plan, QStringLiteral("resourcesImportController"), objects.resourcesImportController);
        appendContextObjectBinding(plan, QStringLiteral("asyncScheduler"), objects.asyncScheduler);
        appendContextObjectBinding(plan, QStringLiteral("calendarBoardStore"), objects.calendarBoardStore);
        appendContextObjectBinding(plan, QStringLiteral("systemCalendarStore"), objects.systemCalendarStore);
        appendContextObjectBinding(plan, QStringLiteral("dayCalendarController"), objects.dayCalendarController);
        appendContextObjectBinding(plan, QStringLiteral("agendaController"), objects.agendaController);
        appendContextObjectBinding(plan, QStringLiteral("monthCalendarController"), objects.monthCalendarController);
        appendContextObjectBinding(plan, QStringLiteral("weekCalendarController"), objects.weekCalendarController);
        appendContextObjectBinding(plan, QStringLiteral("yearCalendarController"), objects.yearCalendarController);
        appendContextObjectBinding(plan, QStringLiteral("panelControllerRegistry"), objects.panelControllerRegistry);

        return lvrs::applyQmlContextBindPlan(engine, plan);
    }
} // namespace WhatSon::Runtime::Bootstrap
