#include "WhatSonHubCreator.hpp"
#include "WhatSonWorkspaceHubCreator.hpp"

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

        QString creatorName() const override
        {
            return QStringLiteral("HubCreatorProbe");
        }

        QStringList requiredRelativePaths() const override
        {
            return {};
        }

        bool createHub(const QString&, QString*, QString*) const override
        {
            return false;
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

private slots:
    void sanitizeHubName_normalizesInput();
    void baseFileHelpers_createDirectoryAndFile();
    void createHub_createsWorkspacePackageAndManifest();
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

void WhatSonWorkspaceHubCreatorTest::createHub_createsWorkspacePackageAndManifest()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const WhatSonWorkspaceHubCreator creator(tempDir.path(), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY2(
        creator.createHub(QStringLiteral("Brand Hub 2026"), &packagePath, &errorMessage),
        qPrintable(errorMessage));

    QFileInfo packageInfo(packagePath);
    QVERIFY(packageInfo.exists());
    QVERIFY(packageInfo.isFile());
    QCOMPARE(packageInfo.suffix(), QStringLiteral("wshub"));

    const QString hubRootPath = QDir(tempDir.path()).filePath(QStringLiteral("hubs/brand-hub-2026"));
    const QString manifestPath = QDir(hubRootPath).filePath(QStringLiteral(".whatson/hub.json"));
    QFile manifestFile(manifestPath);
    QVERIFY(manifestFile.open(QIODevice::ReadOnly | QIODevice::Text));

    const QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestFile.readAll());
    QVERIFY(manifestDocument.isObject());
    const QJsonObject manifestObject = manifestDocument.object();
    QCOMPARE(manifestObject.value(QStringLiteral("format")).toString(), QStringLiteral("wshub"));
    QCOMPARE(manifestObject.value(QStringLiteral("hubDirectory")).toString(), QStringLiteral("brand-hub-2026"));
}

void WhatSonWorkspaceHubCreatorTest::createHub_failsWhenHubAlreadyExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const WhatSonWorkspaceHubCreator creator(tempDir.path(), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY(creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));

    errorMessage.clear();
    QVERIFY(!creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("Hub already exists")));
}

void WhatSonWorkspaceHubCreatorTest::createHub_failsWhenWorkspaceRootIsEmpty()
{
    const WhatSonWorkspaceHubCreator creator(QStringLiteral("   "), QStringLiteral("hubs"));
    QString packagePath;
    QString errorMessage;
    QVERIFY(!creator.createHub(QStringLiteral("Alpha"), &packagePath, &errorMessage));
    QCOMPARE(errorMessage, QStringLiteral("Workspace root path must not be empty."));
}

QTEST_APPLESS_MAIN(WhatSonWorkspaceHubCreatorTest)

#include "test_whatson_workspace_hub_creator.moc"
