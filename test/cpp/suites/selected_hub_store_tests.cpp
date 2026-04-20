#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::selectedHubStore_persistsNormalizedSelectionsWithinSandboxedSettings()
{
    QTemporaryDir settingsDir;
    QTemporaryDir hubRootDir;
    QVERIFY(settingsDir.isValid());
    QVERIFY(hubRootDir.isValid());

    const ScopedQSettingsSandbox settingsSandbox(settingsDir.path());
    SelectedHubStore store;

    const QString selectedHubPath = QDir(hubRootDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    QVERIFY(QDir().mkpath(selectedHubPath));

    const QByteArray bookmark("bookmark-token");
    store.setSelectedHubSelection(selectedHubPath, bookmark);

    QCOMPARE(store.selectedHubPath(), WhatSon::HubPath::normalizeAbsolutePath(selectedHubPath));
    QCOMPARE(store.selectedHubAccessBookmark(), bookmark);

    store.clearSelectedHubPath();
    QCOMPARE(store.selectedHubPath(), QString());
    QCOMPARE(store.selectedHubAccessBookmark(), QByteArray());

    store.setSelectedHubPath(QDir(hubRootDir.path()).filePath(QStringLiteral("NotAHub.txt")));
    QCOMPARE(store.selectedHubPath(), QString());
}
