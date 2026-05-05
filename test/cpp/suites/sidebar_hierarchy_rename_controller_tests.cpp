#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("function decodedHierarchyPathSegments(rawPath)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function leafHierarchyItemLabel(rawLabel, rawPath)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (nextCharacter === \"\\\\\" || nextCharacter === \"/\")")));
    QVERIFY(sidebarSource.contains(QStringLiteral("renameController.leafHierarchyItemLabel(item.label, item.id)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("item && item.id !== undefined && item.id !== null ? item.id : \"\"")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("const segments = normalizedLabel.split(\"/\")")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_bindsInlineHelperDependenciesAtStartup()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("property var view: sidebarHierarchyView")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyController: sidebarHierarchyView.hierarchyController")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyInteractionBridge: sidebarHierarchyView.hierarchyInteractionBridge")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyRenameField: sidebarHierarchyView.hierarchyRenameFieldItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyDragDropBridge: sidebarHierarchyView.hierarchyDragDropBridge")));
    QVERIFY(sidebarSource.contains(QStringLiteral("hierarchyTree: sidebarHierarchyView.hierarchyTreeItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("bookmarkCanvas: sidebarHierarchyView.bookmarkPaletteCanvasItem")));
    QVERIFY(sidebarSource.contains(QStringLiteral("itemLocator: noteDropController")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("property var view: null")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("const normalizedModifiers = controller.")));
}

void WhatSonCppRegressionTests::sidebarHierarchyView_routesFooterActionsThroughListFooterSignal()
{
    const QString sidebarSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!sidebarSource.isEmpty());
    QVERIFY(sidebarSource.contains(QStringLiteral("function handleHierarchyFooterButtonClicked(index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("onButtonClicked: function (index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.create\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.delete\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.options\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestViewOptions();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestViewOptions();")));
}
