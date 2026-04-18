#pragma once

class QObject;
class QQmlContext;

namespace WhatSon::Runtime::Bootstrap
{
    struct WorkspaceContextObjects final
    {
        QObject* libraryHierarchyViewModel = nullptr;
        QObject* libraryNoteMutationViewModel = nullptr;
        QObject* projectsHierarchyViewModel = nullptr;
        QObject* bookmarksHierarchyViewModel = nullptr;
        QObject* tagsHierarchyViewModel = nullptr;
        QObject* resourcesHierarchyViewModel = nullptr;
        QObject* resourcesImportViewModel = nullptr;
        QObject* progressHierarchyViewModel = nullptr;
        QObject* eventHierarchyViewModel = nullptr;
        QObject* presetHierarchyViewModel = nullptr;
        QObject* detailPanelViewModel = nullptr;
        QObject* noteDetailPanelViewModel = nullptr;
        QObject* resourceDetailPanelViewModel = nullptr;
        QObject* editorViewModeViewModel = nullptr;
        QObject* navigationModeViewModel = nullptr;
        QObject* sidebarHierarchyViewModel = nullptr;
        QObject* asyncScheduler = nullptr;
        QObject* calendarBoardStore = nullptr;
        QObject* systemCalendarStore = nullptr;
        QObject* dayCalendarViewModel = nullptr;
        QObject* agendaViewModel = nullptr;
        QObject* monthCalendarViewModel = nullptr;
        QObject* weekCalendarViewModel = nullptr;
        QObject* yearCalendarViewModel = nullptr;
        QObject* panelViewModelRegistry = nullptr;
    };

    void bindWorkspaceContextObjects(QQmlContext* context, const WorkspaceContextObjects& objects);
} // namespace WhatSon::Runtime::Bootstrap
