#include "runtime/bootstrap/WhatSonHubSyncWiring.hpp"
#include "runtime/bootstrap/WhatSonQmlContextBinder.hpp"
#include "sync/WhatSonHubSyncController.hpp"

#include <QCoreApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QObject>
#include <QtTest/QtTest>

class FakeLocalMutationSource final : public QObject
{
    Q_OBJECT

signals:
    void hubFilesystemMutated();
};

class RuntimeBootstrapWiringTest final : public QObject
{
    Q_OBJECT

private slots:
    void hubSyncWiring_mustConnectAllMutationSources();
    void qmlContextBinder_mustBindWorkspaceContextObjects();
};

void RuntimeBootstrapWiringTest::hubSyncWiring_mustConnectAllMutationSources()
{
    WhatSonHubSyncController controller;
    FakeLocalMutationSource librarySource;
    FakeLocalMutationSource bookmarksSource;
    FakeLocalMutationSource progressSource;

    const WhatSon::Runtime::Bootstrap::HubSyncWiringResult result =
        WhatSon::Runtime::Bootstrap::wireHubSyncController(
            &controller,
            QCoreApplication::instance(),
            {
                &librarySource,
                &bookmarksSource,
                &progressSource
            });

    QCOMPARE(result.localMutationConnections.size(), 3);
    QVERIFY(result.localMutationWiringComplete);
    QVERIFY(result.allConnectionsValid());
}

void RuntimeBootstrapWiringTest::qmlContextBinder_mustBindWorkspaceContextObjects()
{
    QQmlEngine engine;
    QQmlContext* rootContext = engine.rootContext();
    QVERIFY(rootContext != nullptr);

    QObject libraryHierarchyViewModel;
    QObject libraryNoteMutationViewModel;
    QObject projectsHierarchyViewModel;
    QObject bookmarksHierarchyViewModel;
    QObject tagsHierarchyViewModel;
    QObject resourcesHierarchyViewModel;
    QObject resourcesImportViewModel;
    QObject progressHierarchyViewModel;
    QObject eventHierarchyViewModel;
    QObject presetHierarchyViewModel;
    QObject detailPanelViewModel;
    QObject editorViewModeViewModel;
    QObject navigationModeViewModel;
    QObject sidebarHierarchyViewModel;
    QObject asyncScheduler;
    QObject calendarBoardStore;
    QObject systemCalendarStore;
    QObject dayCalendarViewModel;
    QObject todoListViewModel;
    QObject monthCalendarViewModel;
    QObject weekCalendarViewModel;
    QObject yearCalendarViewModel;
    QObject panelViewModelRegistry;

    WhatSon::Runtime::Bootstrap::WorkspaceContextObjects objects;
    objects.libraryHierarchyViewModel = &libraryHierarchyViewModel;
    objects.libraryNoteMutationViewModel = &libraryNoteMutationViewModel;
    objects.projectsHierarchyViewModel = &projectsHierarchyViewModel;
    objects.bookmarksHierarchyViewModel = &bookmarksHierarchyViewModel;
    objects.tagsHierarchyViewModel = &tagsHierarchyViewModel;
    objects.resourcesHierarchyViewModel = &resourcesHierarchyViewModel;
    objects.resourcesImportViewModel = &resourcesImportViewModel;
    objects.progressHierarchyViewModel = &progressHierarchyViewModel;
    objects.eventHierarchyViewModel = &eventHierarchyViewModel;
    objects.presetHierarchyViewModel = &presetHierarchyViewModel;
    objects.detailPanelViewModel = &detailPanelViewModel;
    objects.editorViewModeViewModel = &editorViewModeViewModel;
    objects.navigationModeViewModel = &navigationModeViewModel;
    objects.sidebarHierarchyViewModel = &sidebarHierarchyViewModel;
    objects.asyncScheduler = &asyncScheduler;
    objects.calendarBoardStore = &calendarBoardStore;
    objects.systemCalendarStore = &systemCalendarStore;
    objects.dayCalendarViewModel = &dayCalendarViewModel;
    objects.todoListViewModel = &todoListViewModel;
    objects.monthCalendarViewModel = &monthCalendarViewModel;
    objects.weekCalendarViewModel = &weekCalendarViewModel;
    objects.yearCalendarViewModel = &yearCalendarViewModel;
    objects.panelViewModelRegistry = &panelViewModelRegistry;

    WhatSon::Runtime::Bootstrap::bindWorkspaceContextObjects(rootContext, objects);

    auto assertContextObject = [rootContext](const char* name, QObject* expected)
    {
        QCOMPARE(rootContext->contextProperty(QString::fromLatin1(name)).value<QObject*>(), expected);
    };

    assertContextObject("libraryHierarchyViewModel", &libraryHierarchyViewModel);
    assertContextObject("libraryNoteMutationViewModel", &libraryNoteMutationViewModel);
    assertContextObject("projectsHierarchyViewModel", &projectsHierarchyViewModel);
    assertContextObject("bookmarksHierarchyViewModel", &bookmarksHierarchyViewModel);
    assertContextObject("tagsHierarchyViewModel", &tagsHierarchyViewModel);
    assertContextObject("resourcesHierarchyViewModel", &resourcesHierarchyViewModel);
    assertContextObject("resourcesImportViewModel", &resourcesImportViewModel);
    assertContextObject("progressHierarchyViewModel", &progressHierarchyViewModel);
    assertContextObject("eventHierarchyViewModel", &eventHierarchyViewModel);
    assertContextObject("presetHierarchyViewModel", &presetHierarchyViewModel);
    assertContextObject("detailPanelViewModel", &detailPanelViewModel);
    assertContextObject("editorViewModeViewModel", &editorViewModeViewModel);
    assertContextObject("navigationModeViewModel", &navigationModeViewModel);
    assertContextObject("sidebarHierarchyViewModel", &sidebarHierarchyViewModel);
    assertContextObject("asyncScheduler", &asyncScheduler);
    assertContextObject("calendarBoardStore", &calendarBoardStore);
    assertContextObject("systemCalendarStore", &systemCalendarStore);
    assertContextObject("dayCalendarViewModel", &dayCalendarViewModel);
    assertContextObject("todoListViewModel", &todoListViewModel);
    assertContextObject("monthCalendarViewModel", &monthCalendarViewModel);
    assertContextObject("weekCalendarViewModel", &weekCalendarViewModel);
    assertContextObject("yearCalendarViewModel", &yearCalendarViewModel);
    assertContextObject("panelViewModelRegistry", &panelViewModelRegistry);
}

QTEST_MAIN(RuntimeBootstrapWiringTest)

#include "test_runtime_bootstrap_wiring.moc"
