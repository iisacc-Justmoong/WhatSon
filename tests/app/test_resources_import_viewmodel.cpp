#include "viewmodel/hierarchy/resources/ResourcesImportViewModel.hpp"

#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>
#include <QtTest/QtTest>

namespace
{
    bool writeUtf8File(const QString& path, const QString& text)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        return file.write(text.toUtf8()) >= 0;
    }

    bool createHubShell(const QString& hubPath)
    {
        return QDir().mkpath(QDir(hubPath).filePath(QStringLiteral("ImportHub.wscontents")))
            && QDir().mkpath(QDir(hubPath).filePath(QStringLiteral("ImportHub.wsresources")));
    }

    bool readResourcesStore(
        const QString& filePath,
        WhatSonResourcesHierarchyStore* outStore,
        QString* errorMessage = nullptr)
    {
        if (outStore == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outStore must not be null.");
            }
            return false;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open Resources.wsresources: %1").arg(filePath);
            }
            return false;
        }

        WhatSonResourcesHierarchyParser parser;
        return parser.parse(QString::fromUtf8(file.readAll()), outStore, errorMessage);
    }

    bool createExistingResourcePackage(
        const QString& resourcesDirectoryPath,
        const QString& resourceId,
        const QString& assetFileName,
        const QString& assetPayload = QStringLiteral("payload"))
    {
        const QString packageDirectoryPath = QDir(resourcesDirectoryPath).filePath(
            resourceId + QStringLiteral(".wsresource"));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return false;
        }

        const QString resourcePath = QStringLiteral("%1/%2")
                                         .arg(QFileInfo(resourcesDirectoryPath).fileName(), QFileInfo(packageDirectoryPath).fileName());
        const WhatSon::Resources::ResourcePackageMetadata metadata =
            WhatSon::Resources::buildMetadataForAssetFile(assetFileName, resourceId, resourcePath);

        return writeUtf8File(QDir(packageDirectoryPath).filePath(assetFileName), assetPayload)
            && writeUtf8File(
                QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
                WhatSon::Resources::createResourcePackageMetadataXml(metadata));
    }
}

class ResourcesImportViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void importUrls_importsFilesIntoCurrentHubAndRewritesResourcesList();
    void importUrlsForEditor_returnsImportedResourceMetadataEntries();
    void importUrls_preservesExistingResourcesAndGeneratesUniqueIds();
    void importUrls_withoutLocalFiles_failsWithSelectionMessage();
    void importUrls_withoutCurrentHubPath_fails();
};

void ResourcesImportViewModelTest::importUrls_importsFilesIntoCurrentHubAndRewritesResourcesList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ImportHub.wshub"));
    QVERIFY(createHubShell(hubPath));

    const QString firstSourceFilePath = QDir(tempDir.path()).filePath(QStringLiteral("Poster.PNG"));
    const QString secondSourceFilePath = QDir(tempDir.path()).filePath(QStringLiteral("voice.mp3"));
    QVERIFY(writeUtf8File(firstSourceFilePath, QStringLiteral("png-bytes")));
    QVERIFY(writeUtf8File(secondSourceFilePath, QStringLiteral("mp3-bytes")));

    ResourcesImportViewModel viewModel;
    viewModel.setCurrentHubPath(hubPath);

    int reloadCallCount = 0;
    QString reloadedHubPath;
    viewModel.setReloadResourcesCallback(
        [&reloadCallCount, &reloadedHubPath](const QString& hubPathArg, QString* errorMessage) -> bool
        {
            ++reloadCallCount;
            reloadedHubPath = hubPathArg;
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        });

    QSignalSpy importCompletedSpy(&viewModel, &ResourcesImportViewModel::importCompleted);
    QSignalSpy operationFailedSpy(&viewModel, &ResourcesImportViewModel::operationFailed);

    const QVariantList droppedUrls{
        QUrl::fromLocalFile(firstSourceFilePath),
        QUrl::fromLocalFile(secondSourceFilePath)
    };
    QVERIFY(viewModel.canImportUrls(droppedUrls));
    QVERIFY(viewModel.importUrls(droppedUrls));

    QCOMPARE(reloadCallCount, 1);
    QCOMPARE(reloadedHubPath, hubPath);
    QCOMPARE(operationFailedSpy.count(), 0);
    QCOMPARE(importCompletedSpy.count(), 1);
    QCOMPARE(importCompletedSpy.takeFirst().at(0).toInt(), 2);
    QVERIFY(!viewModel.busy());
    QCOMPARE(viewModel.lastError(), QString());

    const QString resourcesRootPath = QDir(hubPath).filePath(QStringLiteral("ImportHub.wsresources"));
    const QString posterPackagePath = QDir(resourcesRootPath).filePath(QStringLiteral("poster.wsresource"));
    const QString voicePackagePath = QDir(resourcesRootPath).filePath(QStringLiteral("voice.wsresource"));
    QVERIFY(QFileInfo(posterPackagePath).isDir());
    QVERIFY(QFileInfo(voicePackagePath).isDir());
    QVERIFY(QFileInfo(QDir(posterPackagePath).filePath(QStringLiteral("Poster.PNG"))).isFile());
    QVERIFY(QFileInfo(QDir(voicePackagePath).filePath(QStringLiteral("voice.mp3"))).isFile());

    WhatSon::Resources::ResourcePackageMetadata posterMetadata;
    QString metadataError;
    QVERIFY2(
        WhatSon::Resources::loadResourcePackageMetadata(posterPackagePath, &posterMetadata, &metadataError),
        qPrintable(metadataError));
    QCOMPARE(posterMetadata.resourceId, QStringLiteral("poster"));
    QCOMPARE(posterMetadata.resourcePath, QStringLiteral("ImportHub.wsresources/poster.wsresource"));
    QCOMPARE(posterMetadata.assetPath, QStringLiteral("Poster.PNG"));
    QCOMPARE(posterMetadata.format, QStringLiteral(".PNG"));
    QCOMPARE(posterMetadata.type, QStringLiteral("image"));
    QCOMPARE(posterMetadata.bucket, QStringLiteral("Image"));

    WhatSon::Resources::ResourcePackageMetadata voiceMetadata;
    QVERIFY2(
        WhatSon::Resources::loadResourcePackageMetadata(voicePackagePath, &voiceMetadata, &metadataError),
        qPrintable(metadataError));
    QCOMPARE(voiceMetadata.resourceId, QStringLiteral("voice"));
    QCOMPARE(voiceMetadata.format, QStringLiteral(".mp3"));
    QCOMPARE(voiceMetadata.type, QStringLiteral("music"));
    QCOMPARE(voiceMetadata.bucket, QStringLiteral("Music"));

    WhatSonResourcesHierarchyStore resourcesStore;
    const QString resourcesFilePath = QDir(hubPath).filePath(QStringLiteral("ImportHub.wscontents/Resources.wsresources"));
    QString parseError;
    QVERIFY2(readResourcesStore(resourcesFilePath, &resourcesStore, &parseError), qPrintable(parseError));
    QCOMPARE(
        resourcesStore.resourcePaths(),
        QStringList({
            QStringLiteral("ImportHub.wsresources/poster.wsresource"),
            QStringLiteral("ImportHub.wsresources/voice.wsresource")
        }));
}

void ResourcesImportViewModelTest::importUrls_preservesExistingResourcesAndGeneratesUniqueIds()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ImportHub.wshub"));
    QVERIFY(createHubShell(hubPath));

    const QString resourcesRootPath = QDir(hubPath).filePath(QStringLiteral("ImportHub.wsresources"));
    QVERIFY(createExistingResourcePackage(
        resourcesRootPath,
        QStringLiteral("poster"),
        QStringLiteral("Poster.PNG"),
        QStringLiteral("old-png")));

    WhatSonResourcesHierarchyStore existingStore;
    existingStore.setHubPath(hubPath);
    existingStore.setResourcePaths({QStringLiteral("ImportHub.wsresources/poster.wsresource")});
    QString writeError;
    QVERIFY2(
        existingStore.writeToFile(
            QDir(hubPath).filePath(QStringLiteral("ImportHub.wscontents/Resources.wsresources")),
            &writeError),
        qPrintable(writeError));

    const QString duplicateSourceFilePath = QDir(tempDir.path()).filePath(QStringLiteral("Poster.PNG"));
    QVERIFY(writeUtf8File(duplicateSourceFilePath, QStringLiteral("new-png")));

    ResourcesImportViewModel viewModel;
    viewModel.setCurrentHubPath(hubPath);
    viewModel.setReloadResourcesCallback(
        [](const QString&, QString* errorMessage) -> bool
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        });

    QVERIFY(viewModel.importUrls({QUrl::fromLocalFile(duplicateSourceFilePath)}));

    const QString duplicatePackagePath = QDir(resourcesRootPath).filePath(QStringLiteral("poster-2.wsresource"));
    QVERIFY(QFileInfo(duplicatePackagePath).isDir());

    WhatSonResourcesHierarchyStore resourcesStore;
    QString parseError;
    QVERIFY2(
        readResourcesStore(
            QDir(hubPath).filePath(QStringLiteral("ImportHub.wscontents/Resources.wsresources")),
            &resourcesStore,
            &parseError),
        qPrintable(parseError));
    QCOMPARE(
        resourcesStore.resourcePaths(),
        QStringList({
            QStringLiteral("ImportHub.wsresources/poster.wsresource"),
            QStringLiteral("ImportHub.wsresources/poster-2.wsresource")
        }));
}

void ResourcesImportViewModelTest::importUrlsForEditor_returnsImportedResourceMetadataEntries()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ImportHub.wshub"));
    QVERIFY(createHubShell(hubPath));

    const QString sourceFilePath = QDir(tempDir.path()).filePath(QStringLiteral("Gallery-Preview.JPG"));
    QVERIFY(writeUtf8File(sourceFilePath, QStringLiteral("jpg-bytes")));

    ResourcesImportViewModel viewModel;
    viewModel.setCurrentHubPath(hubPath);
    viewModel.setReloadResourcesCallback(
        [](const QString&, QString* errorMessage) -> bool
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        });

    const QVariantList importedEntries = viewModel.importUrlsForEditor({QUrl::fromLocalFile(sourceFilePath)});
    QCOMPARE(importedEntries.size(), 1);
    const QVariantMap entry = importedEntries.constFirst().toMap();
    QCOMPARE(entry.value(QStringLiteral("resourceId")).toString(), QStringLiteral("gallery-preview"));
    QCOMPARE(entry.value(QStringLiteral("resourcePath")).toString(), QStringLiteral("ImportHub.wsresources/gallery-preview.wsresource"));
    QCOMPARE(entry.value(QStringLiteral("assetPath")).toString(), QStringLiteral("Gallery-Preview.JPG"));
    QCOMPARE(entry.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(entry.value(QStringLiteral("format")).toString(), QStringLiteral(".jpg"));
    QCOMPARE(entry.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));
}

void ResourcesImportViewModelTest::importUrls_withoutLocalFiles_failsWithSelectionMessage()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ImportHub.wshub"));
    QVERIFY(createHubShell(hubPath));

    ResourcesImportViewModel viewModel;
    viewModel.setCurrentHubPath(hubPath);

    QSignalSpy operationFailedSpy(&viewModel, &ResourcesImportViewModel::operationFailed);

    const QVariantList nonLocalUrls{QUrl(QStringLiteral("https://example.com/poster.png"))};
    QVERIFY(!viewModel.canImportUrls(nonLocalUrls));
    QVERIFY(!viewModel.importUrls(nonLocalUrls));

    QCOMPARE(operationFailedSpy.count(), 1);
    QCOMPARE(viewModel.lastError(), QStringLiteral("Select at least one local file to import as a resource."));
}

void ResourcesImportViewModelTest::importUrls_withoutCurrentHubPath_fails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourceFilePath = QDir(tempDir.path()).filePath(QStringLiteral("Poster.PNG"));
    QVERIFY(writeUtf8File(sourceFilePath, QStringLiteral("png-bytes")));

    ResourcesImportViewModel viewModel;
    QSignalSpy operationFailedSpy(&viewModel, &ResourcesImportViewModel::operationFailed);

    QVERIFY(!viewModel.canImportUrls({QUrl::fromLocalFile(sourceFilePath)}));
    QVERIFY(!viewModel.importUrls({QUrl::fromLocalFile(sourceFilePath)}));

    QCOMPARE(operationFailedSpy.count(), 1);
    QCOMPARE(viewModel.lastError(), QStringLiteral("Current hub path is empty."));
}

QTEST_MAIN(ResourcesImportViewModelTest)

#include "test_resources_import_viewmodel.moc"
