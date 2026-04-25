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

    void appendViewModelBinding(lvrs::QmlContextBindPlan& plan, const QString& key, QObject* object)
    {
        lvrs::QmlViewModelBinding binding;
        binding.key = key;
        binding.object = object;
        binding.contextName = key;
        plan.viewModels.append(binding);
    }
} // namespace

    lvrs::QmlContextBindResult bindWorkspaceContextObjects(
        QQmlApplicationEngine& engine,
        const WorkspaceContextObjects& objects)
    {
        lvrs::QmlContextBindPlan plan;

        appendViewModelBinding(plan, QStringLiteral("libraryHierarchyViewModel"), objects.libraryHierarchyViewModel);
        appendViewModelBinding(
            plan,
            QStringLiteral("libraryNoteMutationViewModel"),
            objects.libraryNoteMutationViewModel);
        appendViewModelBinding(plan, QStringLiteral("projectsHierarchyViewModel"), objects.projectsHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("bookmarksHierarchyViewModel"), objects.bookmarksHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("tagsHierarchyViewModel"), objects.tagsHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("resourcesHierarchyViewModel"), objects.resourcesHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("progressHierarchyViewModel"), objects.progressHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("eventHierarchyViewModel"), objects.eventHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("presetHierarchyViewModel"), objects.presetHierarchyViewModel);
        appendViewModelBinding(plan, QStringLiteral("detailPanelViewModel"), objects.detailPanelViewModel);
        appendViewModelBinding(plan, QStringLiteral("noteDetailPanelViewModel"), objects.noteDetailPanelViewModel);
        appendViewModelBinding(
            plan,
            QStringLiteral("resourceDetailPanelViewModel"),
            objects.resourceDetailPanelViewModel);
        appendViewModelBinding(plan, QStringLiteral("editorViewModeViewModel"), objects.editorViewModeViewModel);
        appendViewModelBinding(plan, QStringLiteral("navigationModeViewModel"), objects.navigationModeViewModel);
        appendViewModelBinding(plan, QStringLiteral("sidebarHierarchyViewModel"), objects.sidebarHierarchyViewModel);

        appendContextObjectBinding(plan, QStringLiteral("resourcesImportViewModel"), objects.resourcesImportViewModel);
        appendContextObjectBinding(plan, QStringLiteral("asyncScheduler"), objects.asyncScheduler);
        appendContextObjectBinding(plan, QStringLiteral("calendarBoardStore"), objects.calendarBoardStore);
        appendContextObjectBinding(plan, QStringLiteral("systemCalendarStore"), objects.systemCalendarStore);
        appendContextObjectBinding(plan, QStringLiteral("dayCalendarViewModel"), objects.dayCalendarViewModel);
        appendContextObjectBinding(plan, QStringLiteral("agendaViewModel"), objects.agendaViewModel);
        appendContextObjectBinding(plan, QStringLiteral("monthCalendarViewModel"), objects.monthCalendarViewModel);
        appendContextObjectBinding(plan, QStringLiteral("weekCalendarViewModel"), objects.weekCalendarViewModel);
        appendContextObjectBinding(plan, QStringLiteral("yearCalendarViewModel"), objects.yearCalendarViewModel);
        appendContextObjectBinding(plan, QStringLiteral("panelViewModelRegistry"), objects.panelViewModelRegistry);

        return lvrs::applyQmlContextBindPlan(engine, plan);
    }
} // namespace WhatSon::Runtime::Bootstrap
