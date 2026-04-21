#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::hubMountValidator_acceptsCompleteHubPackage()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString fixtureError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDir.path(),
        QStringLiteral("Workspace.wshub"),
        &fixtureError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(fixtureError));

    const WhatSonHubMountValidator hubMountValidator;
    const WhatSonHubMountValidation validation = hubMountValidator.resolveMountedHub(hubPath);

    QVERIFY2(validation.mounted, qPrintable(validation.failureMessage));
    QCOMPARE(validation.hubPath, WhatSon::HubPath::normalizeAbsolutePath(hubPath));
    QCOMPARE(validation.failureMessage, QString());
}

void WhatSonCppRegressionTests::hubMountValidator_rejectsIncompleteHubPackage()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString fixtureError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDir.path(),
        QStringLiteral("BrokenWorkspace.wshub"),
        &fixtureError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(fixtureError));
    QVERIFY(QFile::remove(QDir(hubPath).filePath(QStringLiteral(".wscontents/Tags.wstags"))));

    const WhatSonHubMountValidator hubMountValidator;
    const WhatSonHubMountValidation validation = hubMountValidator.resolveMountedHub(hubPath);

    QVERIFY(!validation.mounted);
    QCOMPARE(validation.hubPath, QString());
    QVERIFY(validation.failureMessage.contains(QStringLiteral("Required hub entry is missing")));
    QVERIFY(validation.failureMessage.contains(QStringLiteral("Tags.wstags")));
}
