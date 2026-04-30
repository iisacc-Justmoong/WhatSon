#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::runtimeParallelLoader_usesLvrsBootstrapParallelForDomainLoads()
{
    const QString loaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp"));

    QVERIFY(loaderSource.contains(QStringLiteral("#include \"backend/runtime/bootstrapparallel.h\"")));
    QVERIFY(loaderSource.contains(QStringLiteral("QList<lvrs::BootstrapParallelTask> bootstrapTasks;")));
    QVERIFY(loaderSource.contains(QStringLiteral("lvrs::BootstrapParallelTask task;")));
    QVERIFY(loaderSource.contains(QStringLiteral("task.load = [")));
    QVERIFY(loaderSource.contains(QStringLiteral("lvrs::BootstrapParallelRunOptions bootstrapOptions;")));
    QVERIFY(loaderSource.contains(QStringLiteral("lvrs::runBootstrapParallelTasks(bootstrapTasks, bootstrapOptions)")));
    QVERIFY(loaderSource.contains(QStringLiteral("WhatSonRuntimeDomainSnapshots::buildSharedContext(normalizedPath)")));
    QVERIFY(loaderSource.contains(QStringLiteral("WhatSonRuntimeDomainSnapshots::buildBookmarks(librarySnapshot.allNotes)")));
    QVERIFY(loaderSource.contains(QStringLiteral("if (!allSucceeded)")));
    QVERIFY(loaderSource.contains(QStringLiteral("applySkipped=1")));

    QVERIFY(!loaderSource.contains(QStringLiteral("spawnFunctionLoadThread")));
    QVERIFY(!loaderSource.contains(QStringLiteral("QEventLoop waitLoop")));
    QVERIFY(!loaderSource.contains(QStringLiteral("QThread*")));
    QVERIFY(!loaderSource.contains(QStringLiteral("thread->start()")));

    const qsizetype applyGateIndex = loaderSource.indexOf(QStringLiteral("if (!allSucceeded)"));
    const qsizetype libraryApplyIndex = loaderSource.indexOf(QStringLiteral("targets.libraryController->applyRuntimeSnapshot"));
    QVERIFY(applyGateIndex >= 0);
    QVERIFY(libraryApplyIndex > applyGateIndex);
}
