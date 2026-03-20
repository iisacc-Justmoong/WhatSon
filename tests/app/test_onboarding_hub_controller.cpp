#include "OnboardingHubController.hpp"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class OnboardingHubControllerTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void createHubAtUrl_createsAndLoadsHub();
    void createHubInDirectoryUrl_appendsDefaultHubFileName();
    void createHubInDirectoryUrl_avoidsCollidingHubFileNames();
    void createHubInDirectoryUrl_preservesContentUriTargetPath();
    void createHubInDirectoryUrl_resolvesAndroidExternalStorageTreeUri();
    void prepareHubSelectionFromUrl_collectsMultiplePackageCandidates();
    void loadHubSelectionCandidate_loadsChosenPackage();
    void loadHubFromUrl_resolvesSinglePackageInSelectedFolder();
    void loadHubFromUrl_resolvesPackageFromNestedDirectory();
    void loadHubFromUrl_resolvesPackageFromNestedFile();
    void loadHubFromUrl_rejectsAmbiguousPackageSelection();
    void syncCurrentHubSelection_updatesHubNameAndFolder();
};

void OnboardingHubControllerTest::createHubAtUrl_createsAndLoadsHub()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    OnboardingHubController controller;
    const QString createdHubPath = QDir(tempDir.path()).filePath(QStringLiteral("alpha.wshub"));

    int createInvocationCount = 0;
    int loadInvocationCount = 0;
    QString capturedCreatePath;
    QString capturedLoadPath;
    controller.setCreateHubCallback(
        [&createInvocationCount, &capturedCreatePath, createdHubPath](
        const QString& requestedPath,
        QString* outPackagePath,
        QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            ++createInvocationCount;
            capturedCreatePath = QDir::cleanPath(requestedPath);
            if (outPackagePath != nullptr)
            {
                *outPackagePath = createdHubPath;
            }
            return true;
        });
    controller.setLoadHubCallback(
        [&loadInvocationCount, &capturedLoadPath, createdHubPath](const QString& hubPath, QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            ++loadInvocationCount;
            capturedLoadPath = QDir::cleanPath(hubPath);
            return true;
        });

    QSignalSpy hubCreatedSpy(&controller, &OnboardingHubController::hubCreated);
    QSignalSpy hubLoadedSpy(&controller, &OnboardingHubController::hubLoaded);
    QSignalSpy currentHubNameChangedSpy(&controller, &OnboardingHubController::currentHubNameChanged);
    QSignalSpy currentHubPathNameChangedSpy(&controller, &OnboardingHubController::currentHubPathNameChanged);
    QSignalSpy operationFailedSpy(&controller, &OnboardingHubController::operationFailed);

    QVERIFY(controller.createHubAtUrl(QUrl::fromLocalFile(createdHubPath)));
    QCOMPARE(createInvocationCount, 1);
    QCOMPARE(loadInvocationCount, 1);
    QCOMPARE(capturedCreatePath, createdHubPath);
    QCOMPARE(capturedLoadPath, createdHubPath);
    QCOMPARE(hubCreatedSpy.count(), 1);
    QCOMPARE(hubLoadedSpy.count(), 1);
    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QCOMPARE(currentHubPathNameChangedSpy.count(), 1);
    QCOMPARE(operationFailedSpy.count(), 0);
    QCOMPARE(controller.currentHubName(), QStringLiteral("alpha"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("alpha.wshub"));
    QCOMPARE(controller.lastError(), QString());
    QCOMPARE(controller.currentFolderUrl(), QUrl::fromLocalFile(tempDir.path()));
}

void OnboardingHubControllerTest::createHubInDirectoryUrl_appendsDefaultHubFileName()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    OnboardingHubController controller;
    QString capturedCreatePath;
    controller.setCreateHubCallback(
        [&capturedCreatePath](const QString& requestedPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            capturedCreatePath = QDir::cleanPath(requestedPath);
            if (outPackagePath != nullptr)
            {
                *outPackagePath = capturedCreatePath;
            }
            return true;
        });
    controller.setLoadHubCallback([](const QString&, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        return true;
    });

    QVERIFY(controller.createHubInDirectoryUrl(QUrl::fromLocalFile(tempDir.path()), QString()));
    QCOMPARE(capturedCreatePath, QDir(tempDir.path()).filePath(QStringLiteral("Untitled.wshub")));
}

void OnboardingHubControllerTest::createHubInDirectoryUrl_avoidsCollidingHubFileNames()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString existingHubPath = QDir(tempDir.path()).filePath(QStringLiteral("Untitled.wshub"));
    QVERIFY(QDir().mkpath(existingHubPath));

    OnboardingHubController controller;
    QString capturedCreatePath;
    controller.setCreateHubCallback(
        [&capturedCreatePath](const QString& requestedPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            capturedCreatePath = QDir::cleanPath(requestedPath);
            if (outPackagePath != nullptr)
            {
                *outPackagePath = capturedCreatePath;
            }
            return true;
        });
    controller.setLoadHubCallback([](const QString&, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        return true;
    });

    QVERIFY(controller.createHubInDirectoryUrl(QUrl::fromLocalFile(tempDir.path()), QStringLiteral("Untitled.wshub")));
    QCOMPARE(capturedCreatePath, QDir(tempDir.path()).filePath(QStringLiteral("Untitled-2.wshub")));
}

void OnboardingHubControllerTest::createHubInDirectoryUrl_preservesContentUriTargetPath()
{
    OnboardingHubController controller;
    QString capturedCreatePath;
    controller.setCreateHubCallback(
        [&capturedCreatePath](const QString& requestedPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            capturedCreatePath = requestedPath;
            if (outPackagePath != nullptr)
            {
                *outPackagePath = requestedPath;
            }
            return true;
        });
    controller.setLoadHubCallback([](const QString&, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        return true;
    });

    const QUrl directoryUrl(QStringLiteral("content://whatson.provider/tree/download"));
    QVERIFY(controller.createHubInDirectoryUrl(directoryUrl, QStringLiteral("Untitled.wshub")));
    QCOMPARE(capturedCreatePath, QStringLiteral("content://whatson.provider/tree/download/Untitled.wshub"));
    QCOMPARE(controller.currentFolderUrl(), directoryUrl);
}

void OnboardingHubControllerTest::createHubInDirectoryUrl_resolvesAndroidExternalStorageTreeUri()
{
    OnboardingHubController controller;
    QString capturedCreatePath;
    controller.setCreateHubCallback(
        [&capturedCreatePath](const QString& requestedPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            Q_UNUSED(errorMessage);
            capturedCreatePath = requestedPath;
            if (outPackagePath != nullptr)
            {
                *outPackagePath = requestedPath;
            }
            return true;
        });
    controller.setLoadHubCallback([](const QString&, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        return true;
    });

    const QUrl directoryUrl(
        QStringLiteral("content://com.android.externalstorage.documents/tree/primary%3ADownload"));
    QVERIFY(controller.createHubInDirectoryUrl(directoryUrl, QStringLiteral("Untitled.wshub")));
    QCOMPARE(capturedCreatePath, QStringLiteral("/storage/emulated/0/Download/Untitled.wshub"));
    QCOMPARE(controller.currentFolderUrl(), QUrl::fromLocalFile(QStringLiteral("/storage/emulated/0/Download")));
}

void OnboardingHubControllerTest::prepareHubSelectionFromUrl_collectsMultiplePackageCandidates()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString alphaHubPath = QDir(tempDir.path()).filePath(QStringLiteral("alpha.wshub"));
    const QString betaHubPath = QDir(tempDir.path()).filePath(QStringLiteral("beta.wshub"));
    QVERIFY(QDir().mkpath(alphaHubPath));
    QVERIFY(QDir().mkpath(betaHubPath));

    OnboardingHubController controller;
    bool loadCallbackInvoked = false;
    controller.setLoadHubCallback([&loadCallbackInvoked](const QString&, QString*) -> bool
    {
        loadCallbackInvoked = true;
        return true;
    });

    QSignalSpy candidateSpy(&controller, &OnboardingHubController::hubSelectionCandidatesChanged);
    QVERIFY(controller.prepareHubSelectionFromUrl(QUrl::fromLocalFile(tempDir.path())));
    QVERIFY(!loadCallbackInvoked);
    QCOMPARE(candidateSpy.count(), 1);
    QCOMPARE(controller.hubSelectionCandidateNames(), QStringList({QStringLiteral("alpha.wshub"), QStringLiteral("beta.wshub")}));
    QCOMPARE(controller.lastError(), QString());
}

void OnboardingHubControllerTest::loadHubSelectionCandidate_loadsChosenPackage()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString alphaHubPath = QDir(tempDir.path()).filePath(QStringLiteral("alpha.wshub"));
    const QString betaHubPath = QDir(tempDir.path()).filePath(QStringLiteral("beta.wshub"));
    QVERIFY(QDir().mkpath(alphaHubPath));
    QVERIFY(QDir().mkpath(betaHubPath));

    OnboardingHubController controller;
    QString loadedHubPath;
    controller.setLoadHubCallback([&loadedHubPath](const QString& hubPathValue, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        loadedHubPath = QDir::cleanPath(hubPathValue);
        return true;
    });

    QVERIFY(controller.prepareHubSelectionFromUrl(QUrl::fromLocalFile(tempDir.path())));
    QVERIFY(controller.loadHubSelectionCandidate(1));
    QCOMPARE(loadedHubPath, betaHubPath);
    QCOMPARE(controller.currentHubName(), QStringLiteral("beta"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("beta.wshub"));
    QVERIFY(controller.hubSelectionCandidateNames().isEmpty());
    QCOMPARE(controller.lastError(), QString());
}

void OnboardingHubControllerTest::loadHubFromUrl_resolvesSinglePackageInSelectedFolder()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("sample.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    OnboardingHubController controller;
    QString loadedHubPath;
    QSignalSpy currentHubNameChangedSpy(&controller, &OnboardingHubController::currentHubNameChanged);
    QSignalSpy currentHubPathNameChangedSpy(&controller, &OnboardingHubController::currentHubPathNameChanged);
    controller.setLoadHubCallback([&loadedHubPath](const QString& hubPathValue, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        loadedHubPath = QDir::cleanPath(hubPathValue);
        return true;
    });

    QVERIFY(controller.loadHubFromUrl(QUrl::fromLocalFile(tempDir.path())));
    QCOMPARE(loadedHubPath, hubPath);
    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QCOMPARE(currentHubPathNameChangedSpy.count(), 1);
    QCOMPARE(controller.currentHubName(), QStringLiteral("sample"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("sample.wshub"));
    QCOMPARE(controller.lastError(), QString());
}

void OnboardingHubControllerTest::loadHubFromUrl_resolvesPackageFromNestedDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("sample.wshub"));
    const QString nestedDirectoryPath =
        QDir(hubPath).filePath(QStringLiteral("sample.wscontents/Library.wslibrary"));
    QVERIFY(QDir().mkpath(nestedDirectoryPath));

    OnboardingHubController controller;
    QString loadedHubPath;
    controller.setLoadHubCallback([&loadedHubPath](const QString& hubPathValue, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        loadedHubPath = QDir::cleanPath(hubPathValue);
        return true;
    });

    QVERIFY(controller.loadHubFromUrl(QUrl::fromLocalFile(nestedDirectoryPath)));
    QCOMPARE(loadedHubPath, hubPath);
    QCOMPARE(controller.currentHubName(), QStringLiteral("sample"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("sample.wshub"));
    QCOMPARE(controller.lastError(), QString());
}

void OnboardingHubControllerTest::loadHubFromUrl_resolvesPackageFromNestedFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("sample.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral("sample.wscontents"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));

    const QString nestedFilePath = QDir(contentsDirectoryPath).filePath(QStringLiteral("hub.wscontents"));
    QFile nestedFile(nestedFilePath);
    QVERIFY(nestedFile.open(QIODevice::WriteOnly | QIODevice::Text));
    nestedFile.write("{}");
    nestedFile.close();

    OnboardingHubController controller;
    QString loadedHubPath;
    controller.setLoadHubCallback([&loadedHubPath](const QString& hubPathValue, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        loadedHubPath = QDir::cleanPath(hubPathValue);
        return true;
    });

    QVERIFY(controller.loadHubFromUrl(QUrl::fromLocalFile(nestedFilePath)));
    QCOMPARE(loadedHubPath, hubPath);
    QCOMPARE(controller.currentHubName(), QStringLiteral("sample"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("sample.wshub"));
    QCOMPARE(controller.lastError(), QString());
}

void OnboardingHubControllerTest::loadHubFromUrl_rejectsAmbiguousPackageSelection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QVERIFY(QDir().mkpath(QDir(tempDir.path()).filePath(QStringLiteral("alpha.wshub"))));
    QVERIFY(QDir().mkpath(QDir(tempDir.path()).filePath(QStringLiteral("beta.wshub"))));

    OnboardingHubController controller;
    bool loadCallbackInvoked = false;
    controller.setLoadHubCallback([&loadCallbackInvoked](const QString&, QString*) -> bool
    {
        loadCallbackInvoked = true;
        return true;
    });

    QSignalSpy operationFailedSpy(&controller, &OnboardingHubController::operationFailed);

    QVERIFY(!controller.loadHubFromUrl(QUrl::fromLocalFile(tempDir.path())));
    QVERIFY(!loadCallbackInvoked);
    QCOMPARE(operationFailedSpy.count(), 1);
    QVERIFY(controller.lastError().contains(QStringLiteral("multiple WhatSon Hub packages")));
}

void OnboardingHubControllerTest::syncCurrentHubSelection_updatesHubNameAndFolder()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("TestHub.wshub"));
    QVERIFY(QDir().mkpath(hubPath));

    OnboardingHubController controller;
    QSignalSpy currentHubNameChangedSpy(&controller, &OnboardingHubController::currentHubNameChanged);
    QSignalSpy currentHubPathNameChangedSpy(&controller, &OnboardingHubController::currentHubPathNameChanged);
    QSignalSpy currentFolderUrlChangedSpy(&controller, &OnboardingHubController::currentFolderUrlChanged);

    controller.syncCurrentHubSelection(hubPath);

    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QCOMPARE(currentHubPathNameChangedSpy.count(), 1);
    QVERIFY(currentFolderUrlChangedSpy.count() >= 1);
    QCOMPARE(controller.currentHubName(), QStringLiteral("TestHub"));
    QCOMPARE(controller.currentHubPathName(), QStringLiteral("TestHub.wshub"));
    QCOMPARE(controller.currentFolderUrl(), QUrl::fromLocalFile(tempDir.path()));
}

QTEST_APPLESS_MAIN(OnboardingHubControllerTest)

#include "test_onboarding_hub_controller.moc"
