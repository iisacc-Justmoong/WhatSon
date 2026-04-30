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
