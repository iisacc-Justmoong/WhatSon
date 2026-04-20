#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::startupHubResolver_returnsEmptyWithoutPersistedSelection()
{
    FakeSelectedHubStore store;
    const WhatSonHubMountValidator hubMountValidator;

    const WhatSon::Runtime::Startup::StartupHubSelection selection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(store, hubMountValidator);

    QVERIFY(!selection.mounted);
    QCOMPARE(selection.source, WhatSon::Runtime::Startup::StartupHubSource::None);
    QCOMPARE(selection.hubPath, QString());
    QCOMPARE(selection.accessBookmark, QByteArray());
    QCOMPARE(selection.failureMessage, QString());
}

void WhatSonCppRegressionTests::startupHubResolver_mountsPersistedCompleteHubPackage()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString fixtureError;
    const QString persistedHubPath = createMinimalHubFixture(
        workspaceDir.path(),
        QStringLiteral("PersistedWorkspace.wshub"),
        &fixtureError);
    QVERIFY2(!persistedHubPath.isEmpty(), qPrintable(fixtureError));

    FakeSelectedHubStore store;
    const QByteArray bookmark("persisted-bookmark");
    store.setSelectedHubSelection(persistedHubPath, bookmark);

    const WhatSonHubMountValidator hubMountValidator;
    const WhatSon::Runtime::Startup::StartupHubSelection selection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(store, hubMountValidator);

    QVERIFY(selection.mounted);
    QCOMPARE(selection.source, WhatSon::Runtime::Startup::StartupHubSource::PersistedSelection);
    QCOMPARE(selection.hubPath, WhatSon::HubPath::normalizeAbsolutePath(persistedHubPath));
    QCOMPARE(selection.accessBookmark, bookmark);
    QCOMPARE(selection.failureMessage, QString());
}

void WhatSonCppRegressionTests::startupHubResolver_keepsPersistedFailureVisibleWithoutSwitchingToBlueprint()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    FakeSelectedHubStore store;
    const QString missingPersistedHubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("MissingPersisted.wshub"));
    const QByteArray bookmark("persisted-bookmark");
    store.setSelectedHubSelection(missingPersistedHubPath, bookmark);

    const WhatSonHubMountValidator hubMountValidator;
    const WhatSon::Runtime::Startup::StartupHubSelection selection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(store, hubMountValidator);

    QVERIFY(!selection.mounted);
    QCOMPARE(selection.source, WhatSon::Runtime::Startup::StartupHubSource::None);
    QCOMPARE(selection.hubPath, QString());
    QCOMPARE(selection.accessBookmark, QByteArray());
    QVERIFY(selection.failureMessage.contains(QStringLiteral("does not exist")));
}
