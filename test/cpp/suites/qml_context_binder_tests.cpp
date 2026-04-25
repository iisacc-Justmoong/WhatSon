#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::qmlContextBinder_usesLvrsBindPlanForContextAndViewModels()
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
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::QmlViewModelBinding binding;")));
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::QmlContextObjectBinding binding;")));
    QVERIFY(binderSource.contains(QStringLiteral("lvrs::applyQmlContextBindPlan(engine, plan);")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendViewModelBinding(plan, QStringLiteral(\"libraryHierarchyViewModel\")")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendViewModelBinding(plan, QStringLiteral(\"noteDetailPanelViewModel\")")));
    QVERIFY(binderSource.contains(QStringLiteral("QStringLiteral(\"resourceDetailPanelViewModel\")")));
    QVERIFY(binderSource.contains(QStringLiteral("objects.resourceDetailPanelViewModel")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"resourcesImportViewModel\")")));
    QVERIFY(binderSource.contains(
        QStringLiteral("appendContextObjectBinding(plan, QStringLiteral(\"panelViewModelRegistry\")")));
    QVERIFY(!binderSource.contains(QStringLiteral("setContextProperty(")));

    QVERIFY(mainCppSource.contains(QStringLiteral("const lvrs::QmlContextBindResult workspaceContextBindResult")));
    QVERIFY(mainCppSource.contains(QStringLiteral("bindWorkspaceContextObjects(engine, workspaceContextObjects)")));
    QVERIFY(mainCppSource.contains(QStringLiteral("workspaceContextBindResult.errorMessage()")));
    QVERIFY(!mainCppSource.contains(QStringLiteral("bindWorkspaceContextObjects(engine.rootContext()")));

    QVERIFY(!mainQmlSource.contains(QStringLiteral("function registerRootViewModels()")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("LV.ViewModels.set(")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("registerRootViewModels();")));
}
