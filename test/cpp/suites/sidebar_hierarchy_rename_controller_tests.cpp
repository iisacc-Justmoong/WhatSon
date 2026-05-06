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
    const qsizetype footerButtonsIndex = sidebarSource.indexOf(QStringLiteral("readonly property var hierarchyFooterToolbarButtons"));
    QVERIFY(footerButtonsIndex >= 0);
    const qsizetype footerCreateEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.create\""), footerButtonsIndex);
    const qsizetype footerDeleteEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.delete\""), footerButtonsIndex);
    const qsizetype footerOptionsEventIndex = sidebarSource.indexOf(QStringLiteral("eventName: \"hierarchy.footer.options\""), footerButtonsIndex);
    QVERIFY(footerCreateEventIndex >= 0);
    QVERIFY(footerDeleteEventIndex > footerCreateEventIndex);
    QVERIFY(footerOptionsEventIndex > footerDeleteEventIndex);
    QVERIFY(sidebarSource.contains(QStringLiteral("function handleHierarchyFooterButtonClicked(index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function hierarchyFooterActionName(index, eventName)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("function requestHierarchyFooterAction(action)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("property string hierarchyFooterTriggerQueuedAction: \"\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("onButtonClicked: function (index, config)")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.create\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.delete\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(\"hierarchy.footer.options\");")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestHierarchyFooterAction(action);")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.create\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.delete\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("eventName: \"hierarchy.footer.options\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generaladd\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generaldelete\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("iconName: \"generalmoreHorizontal\"")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("iconName: \"generalsettings\"")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button1: sidebarHierarchyView.hierarchyFooterToolbarButtons[0]")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button2: sidebarHierarchyView.hierarchyFooterToolbarButtons[1]")));
    QVERIFY(sidebarSource.contains(QStringLiteral("button3: sidebarHierarchyView.hierarchyFooterToolbarButtons[2]")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("sidebarHierarchyView.requestViewOptions();")));
    QVERIFY(sidebarSource.contains(QStringLiteral("if (sidebarHierarchyView.hierarchyFooterTriggerQueuedAction === normalizedAction)\n            return true;")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestCreateFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestDeleteFolder();")));
    QVERIFY(!sidebarSource.contains(QStringLiteral("onClicked: function () {\n                    sidebarHierarchyView.requestViewOptions();")));
}
