#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::detailPanelRouting_separatesNoteAndResourceViewsAndViewModels()
{
    const QString detailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/DetailPanel.qml"));
    const QString noteDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/NoteDetailPanel.qml"));
    const QString resourceDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/ResourceDetailPanel.qml"));
    const QString binderSource = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/detailPanel/DetailPanelCurrentHierarchyBinder.cpp"));
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString mainCppSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(!detailPanelSource.isEmpty());
    QVERIFY(!noteDetailPanelSource.isEmpty());
    QVERIFY(!resourceDetailPanelSource.isEmpty());
    QVERIFY(!binderSource.isEmpty());
    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(!mainCppSource.isEmpty());

    QVERIFY(detailPanelSource.contains(QStringLiteral("LV.ViewModels.get(\"noteDetailPanelViewModel\")")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("LV.ViewModels.get(\"resourceDetailPanelViewModel\")")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("NoteDetailPanel {")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("ResourceDetailPanel {")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("sidebarHierarchyVm.activeHierarchyViewModel === resourcesHierarchyVm")));

    QVERIFY(noteDetailPanelSource.contains(QStringLiteral("property var noteDetailPanelViewModel: null")));
    QVERIFY(noteDetailPanelSource.contains(QStringLiteral("readonly property var detailPanelVm: noteDetailPanel.noteDetailPanelViewModel")));
    QVERIFY(resourceDetailPanelSource.contains(QStringLiteral("property var resourceDetailPanelViewModel: null")));

    QVERIFY(mainQmlSource.contains(QStringLiteral("LV.ViewModels.set(\"noteDetailPanelViewModel\", noteDetailPanelViewModel);")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("LV.ViewModels.set(\"resourceDetailPanelViewModel\", resourceDetailPanelViewModel);")));
    QVERIFY(mainCppSource.contains(QStringLiteral("NoteDetailPanelViewModel noteDetailPanelViewModel;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("ResourceDetailPanelViewModel resourceDetailPanelViewModel;")));

    QVERIFY(binderSource.contains(QStringLiteral("setNoteDetailPanelViewModel")));
    QVERIFY(binderSource.contains(QStringLiteral("setResourceDetailPanelViewModel")));
    QVERIFY(binderSource.contains(QStringLiteral("HierarchyDomain::Resources")));
    QVERIFY(binderSource.contains(QStringLiteral("setCurrentResourceListModel")));
}
