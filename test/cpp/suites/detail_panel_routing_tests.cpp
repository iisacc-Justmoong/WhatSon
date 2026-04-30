#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::detailPanelRouting_separatesNoteAndResourceViewsAndControllers()
{
    const QString detailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/DetailPanel.qml"));
    const QString noteDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/NoteDetailPanel.qml"));
    const QString resourceDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/ResourceDetailPanel.qml"));
    const QString binderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/detailPanel/DetailPanelCurrentHierarchyBinder.cpp"));
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString mainCppSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(!detailPanelSource.isEmpty());
    QVERIFY(!noteDetailPanelSource.isEmpty());
    QVERIFY(!resourceDetailPanelSource.isEmpty());
    QVERIFY(!binderSource.isEmpty());
    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(!mainCppSource.isEmpty());

    QVERIFY(detailPanelSource.contains(
        QStringLiteral("typeof noteDetailPanelController !== \"undefined\" ? noteDetailPanelController : null")));
    QVERIFY(detailPanelSource.contains(
        QStringLiteral("typeof resourceDetailPanelController !== \"undefined\" ? resourceDetailPanelController : null")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("NoteDetailPanel {")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("ResourceDetailPanel {")));
    QVERIFY(detailPanelSource.contains(
        QStringLiteral("sidebarHierarchyControllerObject.activeHierarchyController === resourcesHierarchyControllerObject")));

    QVERIFY(noteDetailPanelSource.contains(QStringLiteral("property var noteDetailPanelController: null")));
    QVERIFY(noteDetailPanelSource.contains(
        QStringLiteral("readonly property var detailPanelRuntime: noteDetailPanel.noteDetailPanelController")));
    QVERIFY(resourceDetailPanelSource.contains(QStringLiteral("property var resourceDetailPanelController: null")));

    QVERIFY(!mainQmlSource.contains(QStringLiteral("LV.Controllers")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("LV.ViewModels")));
    QVERIFY(mainCppSource.contains(QStringLiteral("workspaceContextObjects.noteDetailPanelController = &noteDetailPanelController;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("workspaceContextObjects.resourceDetailPanelController = &resourceDetailPanelController;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("NoteDetailPanelController noteDetailPanelController;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("ResourceDetailPanelController resourceDetailPanelController;")));

    QVERIFY(binderSource.contains(QStringLiteral("setNoteDetailPanelController")));
    QVERIFY(binderSource.contains(QStringLiteral("setResourceDetailPanelController")));
    QVERIFY(binderSource.contains(QStringLiteral("HierarchyDomain::Resources")));
    QVERIFY(binderSource.contains(QStringLiteral("setCurrentResourceListModel")));
}
