#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContextBinder_usesLvrsBindPlanForWorkspaceContextObjects()
{
    const QString binderHeader = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.hpp"));
    const QString binderSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlContextBinder.cpp"));
    const QString mainCppSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(binderHeader.contains(QStringLiteral("#include \"backend/runtime/qmlcontextbinder.h\"")));
    QVERIFY(binderHeader.contains(QStringLiteral("lvrs::QmlContextBindResult bindWorkspaceContextObjects(")));
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::QmlContextBindPlan plan;")));
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::QmlContextObjectBinding binding;")));
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::applyQmlContextBindPlan(engine, plan);")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"libraryHierarchyController\")")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"noteDetailPanelController\")")));
    QVERIFY(binderSource.contains(QStringLiteral("QStringLiteral(\"resourceDetailPanelController\")")));
    QVERIFY(binderSource.contains(QStringLiteral("objects.resourceDetailPanelController")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"resourcesImportController\")")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"panelControllerRegistry\")")));
    QVERIFY(binderHeader.contains(QStringLiteral("QObject* noteActiveState = nullptr;")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"noteActiveState\")")));
    QVERIFY(!binderSource.contains(QStringLiteral("QmlControllerBinding")));
    QVERIFY(!binderSource.contains(QStringLiteral("QmlViewModelBinding")));
    QVERIFY(!binderSource.contains(QStringLiteral("appendControllerBinding")));
    QVERIFY(!binderSource.contains(QStringLiteral("setContextProperty(")));

    QVERIFY(mainCppSource.contains(QStringLiteral("const lvrs::QmlContextBindResult workspaceContextBindResult")));
    QVERIFY(mainCppSource.contains(QStringLiteral("NoteActiveStateTracker noteActiveState;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("noteActiveState.setHierarchyContextSource(&sidebarHierarchyController);")));
    QVERIFY(mainCppSource.contains(QStringLiteral("workspaceContextObjects.noteActiveState = &noteActiveState;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("bindWorkspaceContextObjects(engine, workspaceContextObjects)")));
    QVERIFY(mainCppSource.contains(QStringLiteral("workspaceContextBindResult.errorMessage()")));
    QVERIFY(!mainCppSource.contains(QStringLiteral("bindWorkspaceContextObjects(engine.rootContext()")));

    QVERIFY(!mainQmlSource.contains(QStringLiteral("function registerRootControllers()")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("LV.Controllers")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("LV.ViewModels")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("registerRootControllers();")));
}
