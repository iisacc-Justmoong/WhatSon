#include "OnboardingHubController.hpp"

#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class OnboardingHubControllerTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void createHubAtUrl_createsAndLoadsHub();
    void loadHubFromUrl_resolvesSinglePackageInSelectedFolder();
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
    QSignalSpy operationFailedSpy(&controller, &OnboardingHubController::operationFailed);

    QVERIFY(controller.createHubAtUrl(QUrl::fromLocalFile(createdHubPath)));
    QCOMPARE(createInvocationCount, 1);
    QCOMPARE(loadInvocationCount, 1);
    QCOMPARE(capturedCreatePath, createdHubPath);
    QCOMPARE(capturedLoadPath, createdHubPath);
    QCOMPARE(hubCreatedSpy.count(), 1);
    QCOMPARE(hubLoadedSpy.count(), 1);
    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QCOMPARE(operationFailedSpy.count(), 0);
    QCOMPARE(controller.currentHubName(), QStringLiteral("alpha"));
    QCOMPARE(controller.lastError(), QString());
    QCOMPARE(controller.currentFolderUrl(), QUrl::fromLocalFile(tempDir.path()));
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
    controller.setLoadHubCallback([&loadedHubPath](const QString& hubPathValue, QString* errorMessage) -> bool
    {
        Q_UNUSED(errorMessage);
        loadedHubPath = QDir::cleanPath(hubPathValue);
        return true;
    });

    QVERIFY(controller.loadHubFromUrl(QUrl::fromLocalFile(tempDir.path())));
    QCOMPARE(loadedHubPath, hubPath);
    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QCOMPARE(controller.currentHubName(), QStringLiteral("sample"));
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
    QSignalSpy currentFolderUrlChangedSpy(&controller, &OnboardingHubController::currentFolderUrlChanged);

    controller.syncCurrentHubSelection(hubPath);

    QCOMPARE(currentHubNameChangedSpy.count(), 1);
    QVERIFY(currentFolderUrlChangedSpy.count() >= 1);
    QCOMPARE(controller.currentHubName(), QStringLiteral("TestHub"));
    QCOMPARE(controller.currentFolderUrl(), QUrl::fromLocalFile(tempDir.path()));
}

QTEST_APPLESS_MAIN(OnboardingHubControllerTest)

#include "test_onboarding_hub_controller.moc"
