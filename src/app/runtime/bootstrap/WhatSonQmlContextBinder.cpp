#include "WhatSonQmlContextBinder.hpp"

#include <QQmlContext>

namespace WhatSon::Runtime::Bootstrap
{
    void bindWorkspaceContextObjects(QQmlContext* context, const WorkspaceContextObjects& objects)
    {
        if (context == nullptr)
        {
            return;
        }

        context->setContextProperty(QStringLiteral("libraryHierarchyViewModel"), objects.libraryHierarchyViewModel);
        context->setContextProperty(QStringLiteral("libraryNoteMutationViewModel"), objects.libraryNoteMutationViewModel);
        context->setContextProperty(QStringLiteral("projectsHierarchyViewModel"), objects.projectsHierarchyViewModel);
        context->setContextProperty(QStringLiteral("bookmarksHierarchyViewModel"), objects.bookmarksHierarchyViewModel);
        context->setContextProperty(QStringLiteral("tagsHierarchyViewModel"), objects.tagsHierarchyViewModel);
        context->setContextProperty(QStringLiteral("resourcesHierarchyViewModel"), objects.resourcesHierarchyViewModel);
        context->setContextProperty(QStringLiteral("resourcesImportViewModel"), objects.resourcesImportViewModel);
        context->setContextProperty(QStringLiteral("progressHierarchyViewModel"), objects.progressHierarchyViewModel);
        context->setContextProperty(QStringLiteral("eventHierarchyViewModel"), objects.eventHierarchyViewModel);
        context->setContextProperty(QStringLiteral("presetHierarchyViewModel"), objects.presetHierarchyViewModel);
        context->setContextProperty(QStringLiteral("detailPanelViewModel"), objects.detailPanelViewModel);
        context->setContextProperty(QStringLiteral("noteDetailPanelViewModel"), objects.noteDetailPanelViewModel);
        context->setContextProperty(
            QStringLiteral("resourceDetailPanelViewModel"),
            objects.resourceDetailPanelViewModel);
        context->setContextProperty(QStringLiteral("editorViewModeViewModel"), objects.editorViewModeViewModel);
        context->setContextProperty(QStringLiteral("navigationModeViewModel"), objects.navigationModeViewModel);
        context->setContextProperty(QStringLiteral("sidebarHierarchyViewModel"), objects.sidebarHierarchyViewModel);
        context->setContextProperty(QStringLiteral("asyncScheduler"), objects.asyncScheduler);
        context->setContextProperty(QStringLiteral("calendarBoardStore"), objects.calendarBoardStore);
        context->setContextProperty(QStringLiteral("systemCalendarStore"), objects.systemCalendarStore);
        context->setContextProperty(QStringLiteral("dayCalendarViewModel"), objects.dayCalendarViewModel);
        context->setContextProperty(QStringLiteral("agendaViewModel"), objects.agendaViewModel);
        context->setContextProperty(QStringLiteral("monthCalendarViewModel"), objects.monthCalendarViewModel);
        context->setContextProperty(QStringLiteral("weekCalendarViewModel"), objects.weekCalendarViewModel);
        context->setContextProperty(QStringLiteral("yearCalendarViewModel"), objects.yearCalendarViewModel);
        context->setContextProperty(QStringLiteral("panelViewModelRegistry"), objects.panelViewModelRegistry);
    }
} // namespace WhatSon::Runtime::Bootstrap
