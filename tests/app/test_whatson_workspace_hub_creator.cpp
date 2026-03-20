#include "WhatSonHubCreator.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include <utility>

namespace
{
    class HubCreatorProbe final : public WhatSonHubCreator
    {
    public:
        explicit HubCreatorProbe(QString workspaceRootPath)
            : WhatSonHubCreator(std::move(workspaceRootPath))
        {
        }

        using WhatSonHubCreator::ensureDirectory;
        using WhatSonHubCreator::joinPath;
        using WhatSonHubCreator::sanitizeHubName;
        using WhatSonHubCreator::writeTextFile;
    };
} // namespace

class WhatSonWorkspaceHubCreatorTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void sanitizeHubName_normalizesInput();
    void baseFileHelpers_createDirectoryAndFile();
    void joinPath_preservesContentUriSegments();
    void joinPath_resolvesAndroidExternalStorageTreeUri();
    void createHub_createsWorkspacePackageAndManifest();
    void createHubAtPath_createsExplicitPackagePathAndAppendsExtension();
    void createHub_failsWhenHubAlreadyExists();
    void createHub_failsWhenWorkspaceRootIsEmpty();
};

void WhatSonWorkspaceHubCreatorTest::sanitizeHubName_normalizesInput()
{
    const HubCreatorProbe creator(QStringLiteral("/tmp"));
    QCOMPARE(
        creator.sanitizeHubName(QStringLiteral("  Brand Hub 2026!  ")),
        QStringLiteral("brand-hub-2026"));
    QCOMPARE(
        creator.sanitizeHubName(QStringLiteral("%%%^&*   ")),
        QStringLiteral("untitled-hub"));
}

void WhatSonWorkspaceHubCreatorTest::baseFileHelpers_createDirectoryAndFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const HubCreatorProbe creator(tempDir.path());
    const QString nestedPath = creator.joinPath(tempDir.path(), QStringLiteral("alpha/beta"));

    QString errorMessage;
    QVERIFY2(creator.ensureDirectory(nestedPath, &errorMessage), qPrintable(errorMessage));

    const QString filePath = creator.joinPath(nestedPath, QStringLiteral("sample.txt"));
    QVERIFY2(
        creator.writeTextFile(filePath, QStringLiteral("hello-whatson"), &errorMessage),
        qPrintable(errorMessage));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(file.readAll()), QStringLiteral("hello-whatson"));
}

void WhatSonWorkspaceHubCreatorTest::joinPath_preservesContentUriSegments()
{
    const HubCreatorProbe creator(QStringLiteral("/tmp"));
    QCOMPARE(
        creator.joinPath(
            QStringLiteral("content://whatson.provider/tree/download"),
            QStringLiteral(".whatson/hub.json")),
        QStringLiteral("content://whatson.provider/tree/download/.whatson/hub.json"));
}

void WhatSonWorkspaceHubCreatorTest::joinPath_resolvesAndroidExternalStorageTreeUri()
{
    const HubCreatorProbe creator(QStringLiteral("/tmp"));
    QCOMPARE(
        creator.joinPath(
            QStringLiteral(
                "content://com.android.externalstorage.documents/tree/primary%3ADownload"),
            QStringLiteral(".whatson/hub.json")),
        QStringLiteral("/storage/emulated/0/Download/.whatson/hub.json"));
}

void WhatSonWorkspaceHubCreatorTest::createHub_createsWorkspacePackageAndManifest()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const WhatSonHubCreator creator(tempDir.path(), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY2(
        creator.createHub(QStringLiteral("Brand Hub 2026"), &packagePath, &errorMessage),
        qPrintable(errorMessage));

    QFileInfo packageInfo(packagePath);
    QVERIFY(packageInfo.exists());
    QVERIFY(packageInfo.isDir());
    QVERIFY(packageInfo.fileName().endsWith(QStringLiteral(".wshub")));

    const QString hubRootPath = QDir(tempDir.path()).filePath(QStringLiteral("hubs/brand-hub-2026.wshub"));
    const QString manifestPath = QDir(hubRootPath).filePath(QStringLiteral(".whatson/hub.json"));
    QFile manifestFile(manifestPath);
    QVERIFY(manifestFile.open(QIODevice::ReadOnly | QIODevice::Text));

    const QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestFile.readAll());
    QVERIFY(manifestDocument.isObject());
    const QJsonObject manifestObject = manifestDocument.object();
    QCOMPARE(manifestObject.value(QStringLiteral("format")).toString(), QStringLiteral("wshub"));
    QCOMPARE(manifestObject.value(QStringLiteral("hubDirectory")).toString(), QStringLiteral("brand-hub-2026.wshub"));
    QCOMPARE(manifestObject.value(QStringLiteral("contentsRoot")).toString(),
             QStringLiteral("brand-hub-2026.wscontents"));
    QCOMPARE(manifestObject.value(QStringLiteral("resourcesRoot")).toString(),
             QStringLiteral("brand-hub-2026.wsresources"));
    QCOMPARE(manifestObject.value(QStringLiteral("statFile")).toString(), QStringLiteral("brand-hub-2026Stat.wsstat"));

    QVERIFY(QFileInfo(QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026.wscontents"))).isDir());
    QVERIFY(
        QFileInfo(QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026.wscontents/Library.wslibrary"))).isDir());
    QVERIFY(QFileInfo(QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026.wscontents/Preset.wspreset"))).isDir());
    QVERIFY(QFileInfo(QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026.wsresources"))).isDir());
    QVERIFY(QFileInfo(QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026Stat.wsstat"))).isFile());
    QVERIFY(QFileInfo(
            QDir(hubRootPath).filePath(QStringLiteral("brand-hub-2026.wscontents/Library.wslibrary/index.wsnindex"))).
        isFile());
}

void WhatSonWorkspaceHubCreatorTest::createHubAtPath_createsExplicitPackagePathAndAppendsExtension()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const WhatSonHubCreator creator(tempDir.path(), QStringLiteral("hubs"));
    const QString requestedPackagePath = QDir(tempDir.path()).filePath(QStringLiteral("Custom Hub"));

    QString packagePath;
    QString errorMessage;
    QVERIFY2(
        creator.createHubAtPath(requestedPackagePath, &packagePath, &errorMessage),
        qPrintable(errorMessage));

    const QString expectedPackagePath =
        QDir(tempDir.path()).filePath(QStringLiteral("Custom Hub.wshub"));
    QCOMPARE(QDir::cleanPath(packagePath), QDir::cleanPath(expectedPackagePath));
    QVERIFY(QFileInfo(expectedPackagePath).isDir());
    QVERIFY(QFileInfo(QDir(expectedPackagePath).filePath(QStringLiteral(".whatson/hub.json"))).isFile());
    QVERIFY(QFileInfo(QDir(expectedPackagePath).filePath(QStringLiteral("custom-hub.wscontents"))).isDir());
    QVERIFY(QFileInfo(QDir(expectedPackagePath).filePath(QStringLiteral("custom-hub.wsresources"))).isDir());
    QVERIFY(QFileInfo(QDir(expectedPackagePath).filePath(QStringLiteral("custom-hubStat.wsstat"))).isFile());
}

void WhatSonWorkspaceHubCreatorTest::createHub_failsWhenHubAlreadyExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const WhatSonHubCreator creator(tempDir.path(), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY(creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));

    errorMessage.clear();
    QVERIFY(!creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Hub already exists")));
}

void WhatSonWorkspaceHubCreatorTest::createHub_failsWhenWorkspaceRootIsEmpty()
{
    const WhatSonHubCreator creator(QStringLiteral("   "), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY(!creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));
    QCOMPARE(errorMessage, QStringLiteral("Workspace root path must not be empty."));
}

QTEST_APPLESS_MAIN(WhatSonWorkspaceHubCreatorTest)

#include "test_whatson_workspace_hub_creator.moc"
