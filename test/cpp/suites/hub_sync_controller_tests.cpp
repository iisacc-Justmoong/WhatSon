#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp"

namespace
{
    bool writeTextFixture(const QString& filePath, const QString& text)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        return file.write(text.toUtf8()) >= 0;
    }
} // namespace

void WhatSonCppRegressionTests::hubSyncController_splitsFilesystemResponsibilitiesIntoDedicatedObjects()
{
    const QString controllerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/WhatSonHubSyncController.hpp"));
    const QString controllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/WhatSonHubSyncController.cpp"));
    const QString observationHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp"));
    const QString watcherHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/WhatSonHubSyncWatcher.hpp"));
    const QString schedulerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/sync/WhatSonHubSyncScheduler.hpp"));

    QVERIFY(!controllerHeader.isEmpty());
    QVERIFY(observationHeader.contains(QStringLiteral("class WhatSonHubSyncObservationBuilder final")));
    QVERIFY(watcherHeader.contains(QStringLiteral("class WhatSonHubSyncWatcher final : public QObject")));
    QVERIFY(schedulerHeader.contains(QStringLiteral("class WhatSonHubSyncScheduler final : public QObject")));

    QVERIFY(controllerHeader.contains(
        QStringLiteral("#include \"app/models/file/sync/WhatSonHubSyncObservationBuilder.hpp\"")));
    QVERIFY(controllerHeader.contains(
        QStringLiteral("#include \"app/models/file/sync/WhatSonHubSyncScheduler.hpp\"")));
    QVERIFY(controllerHeader.contains(
        QStringLiteral("#include \"app/models/file/sync/WhatSonHubSyncWatcher.hpp\"")));
    QVERIFY(controllerHeader.contains(QStringLiteral("WhatSonHubSyncObservationBuilder m_observationBuilder;")));
    QVERIFY(controllerHeader.contains(QStringLiteral("WhatSonHubSyncScheduler m_scheduler;")));
    QVERIFY(controllerHeader.contains(QStringLiteral("WhatSonHubSyncWatcher m_watcher;")));

    QVERIFY(!controllerHeader.contains(QStringLiteral("#include <QFileSystemWatcher>")));
    QVERIFY(!controllerHeader.contains(QStringLiteral("#include <QTimer>")));
    QVERIFY(!controllerSource.contains(QStringLiteral("#include <QCryptographicHash>")));
    QVERIFY(!controllerSource.contains(QStringLiteral("#include <QDirIterator>")));
    QVERIFY(!controllerSource.contains(QStringLiteral("#include <QSet>")));
}

void WhatSonCppRegressionTests::hubSyncObservationBuilder_ignoresPrivateWhatSonBookkeeping()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Observed.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString privatePath = QDir(hubPath).filePath(QStringLiteral(".whatson"));
    QVERIFY(QDir().mkpath(contentsPath));
    QVERIFY(QDir().mkpath(privatePath));

    const QString noteIndexPath = QDir(contentsPath).filePath(QStringLiteral("index.wsnindex"));
    const QString heartbeatPath = QDir(privatePath).filePath(QStringLiteral("write-lease.json"));
    QVERIFY(writeTextFixture(noteIndexPath, QStringLiteral("alpha")));
    QVERIFY(writeTextFixture(heartbeatPath, QStringLiteral("heartbeat:1")));

    const WhatSonHubSyncObservationBuilder builder;
    const WhatSonHubSyncObservation baseline = builder.inspectHub(hubPath);
    QVERIFY(!baseline.signature.isEmpty());
    QVERIFY(baseline.directoryWatchPaths.contains(QDir::cleanPath(hubPath)));
    QVERIFY(baseline.directoryWatchPaths.contains(QDir::cleanPath(contentsPath)));
    QVERIFY(!baseline.directoryWatchPaths.contains(QDir::cleanPath(privatePath)));

    QVERIFY(writeTextFixture(heartbeatPath, QStringLiteral("heartbeat:2 with ignored private churn")));
    const WhatSonHubSyncObservation privateChange = builder.inspectHub(hubPath);
    QCOMPARE(privateChange.signature, baseline.signature);

    QVERIFY(writeTextFixture(noteIndexPath, QStringLiteral("alpha beta visible change")));
    const WhatSonHubSyncObservation visibleChange = builder.inspectHub(hubPath);
    QVERIFY(visibleChange.signature != baseline.signature);
}

void WhatSonCppRegressionTests::hubSyncWiring_includesNoteEditorSessionVersionDiffMutations()
{
    const QString mainSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));
    QVERIFY(!mainSource.isEmpty());

    const int wiringIndex = mainSource.indexOf(QStringLiteral("wireHubSyncController"));
    QVERIFY(wiringIndex >= 0);
    QVERIFY(!mainSource.mid(wiringIndex).contains(QStringLiteral("&noteEditorSession")));
    QVERIFY(mainSource.mid(wiringIndex).contains(QStringLiteral("&progressHierarchyController")));
}
