#pragma once

#include "backend/runtime/qmlcontextbinder.h"

class QObject;
class QQmlApplicationEngine;

namespace WhatSon::Runtime::Bootstrap
{
    struct WorkspaceContextObjects final
    {
        QObject* libraryHierarchyController = nullptr;
        QObject* libraryNoteMutationController = nullptr;
        QObject* projectsHierarchyController = nullptr;
        QObject* bookmarksHierarchyController = nullptr;
        QObject* tagsHierarchyController = nullptr;
        QObject* resourcesHierarchyController = nullptr;
        QObject* resourcesImportController = nullptr;
        QObject* progressHierarchyController = nullptr;
        QObject* eventHierarchyController = nullptr;
        QObject* presetHierarchyController = nullptr;
        QObject* detailPanelController = nullptr;
        QObject* noteDetailPanelController = nullptr;
        QObject* resourceDetailPanelController = nullptr;
        QObject* editorViewModeController = nullptr;
        QObject* navigationModeController = nullptr;
        QObject* sidebarHierarchyController = nullptr;
        QObject* asyncScheduler = nullptr;
        QObject* calendarBoardStore = nullptr;
        QObject* systemCalendarStore = nullptr;
        QObject* dayCalendarController = nullptr;
        QObject* agendaController = nullptr;
        QObject* monthCalendarController = nullptr;
        QObject* weekCalendarController = nullptr;
        QObject* yearCalendarController = nullptr;
        QObject* panelControllerRegistry = nullptr;
    };

    lvrs::QmlContextBindResult bindWorkspaceContextObjects(
        QQmlApplicationEngine& engine,
        const WorkspaceContextObjects& objects);
} // namespace WhatSon::Runtime::Bootstrap
