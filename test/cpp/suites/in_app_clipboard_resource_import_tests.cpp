#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/clipboard/InAppClipboardManager.h"
#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QBuffer>
#include <QClipboard>
#include <QMimeData>

#include <algorithm>

namespace
{
    QString readUtf8FileForInAppClipboardImportTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    bool isThirtyTwoCharacterAlnumResourceId(const QString& value)
    {
        static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9]{32}$"));
        return pattern.match(value).hasMatch();
    }

    QString resourcesFilePathForHub(const QString& hubPath)
    {
        return QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    }

    QString resourcesFileTextForHub(const QString& hubPath)
    {
        return readUtf8FileForInAppClipboardImportTest(resourcesFilePathForHub(hubPath));
    }

    QString resourcesDirectoryPathForHub(const QString& hubPath)
    {
        return QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    }

    QVector<WhatSon::Resources::ResourcePackageMetadata> importedResourceMetadataForHub(const QString& hubPath)
    {
        QVector<WhatSon::Resources::ResourcePackageMetadata> result;
        const QDir resourcesDirectory(resourcesDirectoryPathForHub(hubPath));
        const QFileInfoList packageDirectories = resourcesDirectory.entryInfoList(
            QStringList{QStringLiteral("*.wsresource")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        result.reserve(packageDirectories.size());
        for (const QFileInfo& packageDirectory : packageDirectories)
        {
            WhatSon::Resources::ResourcePackageMetadata metadata;
            QString metadataError;
            if (WhatSon::Resources::loadResourcePackageMetadata(
                packageDirectory.absoluteFilePath(),
                &metadata,
                &metadataError))
            {
                result.push_back(metadata);
            }
        }
        return result;
    }

    WhatSon::Resources::ResourcePackageMetadata singleImportedResourceMetadataForHub(const QString& hubPath)
    {
        const QVector<WhatSon::Resources::ResourcePackageMetadata> metadata = importedResourceMetadataForHub(hubPath);
        return metadata.size() == 1 ? metadata.constFirst() : WhatSon::Resources::ResourcePackageMetadata{};
    }

} // namespace

void WhatSonCppRegressionTests::inAppClipboard_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));

    QVERIFY(!clipboardSource.isEmpty());
    QVERIFY(clipboardSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(clipboardSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
    QVERIFY(!QFileInfo::exists(QStringLiteral("src/app/models/clipboard/ClipboardEditorPaste.h")));
    QVERIFY(!QFileInfo::exists(QStringLiteral("src/app/models/clipboard/ClipboardEditorPaste.cpp")));
    QVERIFY(!QFileInfo::exists(QStringLiteral("src/app/models/editor/EditorInputCommandFilter.hpp")));
    QVERIFY(!QFileInfo::exists(QStringLiteral("src/app/models/editor/EditorInputCommandFilter.cpp")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.hpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp")).exists());
}

void WhatSonCppRegressionTests::inAppClipboard_importsUrlsAsResourcePackagesWithoutEditorWrappers()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("CaptureHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    QImage sourceImage(QSize(7, 5), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(40, 120, 210, 255));
    const QString capturedImagePath = QDir(workspaceDirectory.path()).filePath(QStringLiteral("clipboard-resource.png"));
    QVERIFY(sourceImage.save(capturedImagePath, "PNG"));

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);
    QSignalSpy importCompletedSpy(&clipboard, &InAppClipboardManager::importCompleted);

    QVERIFY2(
        clipboard.importUrls(QVariantList{QUrl::fromLocalFile(capturedImagePath)}),
        qPrintable(clipboard.lastError()));

    const QString resourceId = QStringLiteral("clipboard-resource");
    const QString resourcePath = QStringLiteral(".wsresources/clipboard-resource.wsresource");
    const QString assetPath = QStringLiteral("clipboard-resource.png");

    QCOMPARE(resourceId, QStringLiteral("clipboard-resource"));
    QCOMPARE(resourcePath, QStringLiteral(".wsresources/clipboard-resource.wsresource"));
    QCOMPARE(assetPath, QStringLiteral("clipboard-resource.png"));
    QVERIFY(resourcePath.startsWith(QStringLiteral(".wsresources/")));
    QVERIFY(resourcePath.endsWith(QStringLiteral(".wsresource")));
    QVERIFY(assetPath.endsWith(QStringLiteral(".png")));
    QCOMPARE(importCompletedSpy.count(), 1);
    QCOMPARE(importCompletedSpy.constFirst().constFirst().toInt(), 1);

    const QString packageDirectoryPath = QDir(hubPath).filePath(resourcePath);
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(assetPath)).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName())).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::annotationFileName())).isFile());

    const QString resourcesListText = resourcesFileTextForHub(hubPath);
    QVERIFY(resourcesListText.contains(resourcePath));

    const QString clipboardHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.h"));
    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));
    QVERIFY(clipboardHeader.contains(QStringLiteral("importClipboardResource")));
    QVERIFY(!clipboardHeader.contains(QStringLiteral("importClipboardResourceForEditor")));
    QVERIFY(!clipboardHeader.contains(QStringLiteral("importUrlsForEditor")));
    QVERIFY(!clipboardHeader.contains(QStringLiteral("importClipboardImage")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp")).exists());
    QVERIFY(!clipboardSource.contains(QStringLiteral("ClipboardImportSupport")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("importClipboardImageInternal")));
}

void WhatSonCppRegressionTests::inAppClipboard_importsClipboardImageThroughManager()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    QImage clipboardImage(QSize(9, 6), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(210, 45, 120, 255));

    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    clipboard.setCurrentHubPath(hubPath);

    QVERIFY2(
        clipboard.importClipboardResource(InAppClipboardManager::ConflictPolicyAbort),
        qPrintable(clipboard.lastError()));

    const WhatSon::Resources::ResourcePackageMetadata importedResource = singleImportedResourceMetadataForHub(hubPath);
    QCOMPARE(importedResource.type, QStringLiteral("image"));
    QCOMPARE(importedResource.format, QStringLiteral(".png"));
    QCOMPARE(importedResource.bucket, QStringLiteral("Image"));
    QVERIFY(importedResource.resourcePath.endsWith(QStringLiteral(".wsresource")));
    QVERIFY(importedResource.assetPath.endsWith(QStringLiteral(".png")));
    QVERIFY(!clipboard.hasResource());

    const QString packageDirectoryPath = QDir(hubPath).filePath(importedResource.resourcePath);
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(importedResource.assetPath)).isFile());

    const QString resourcesListText = resourcesFileTextForHub(hubPath);
    QVERIFY(resourcesListText.contains(importedResource.resourcePath));
}

void WhatSonCppRegressionTests::inAppClipboard_importsClipboardImagesWithRandomAlnumResourceIds()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardRandomNameHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);

    QImage firstClipboardImage(QSize(10, 7), QImage::Format_ARGB32_Premultiplied);
    firstClipboardImage.fill(qRgba(32, 90, 180, 255));
    QVERIFY(clipboard.setImageResource(firstClipboardImage, QStringLiteral("image/png")));
    QVERIFY2(
        clipboard.importClipboardResource(InAppClipboardManager::ConflictPolicyAbort),
        qPrintable(clipboard.lastError()));
    const QVector<WhatSon::Resources::ResourcePackageMetadata> firstMetadata =
        importedResourceMetadataForHub(hubPath);
    QCOMPARE(firstMetadata.size(), 1);

    QImage secondClipboardImage(QSize(11, 8), QImage::Format_ARGB32_Premultiplied);
    secondClipboardImage.fill(qRgba(180, 80, 32, 255));
    QVERIFY(clipboard.setImageResource(secondClipboardImage, QStringLiteral("image/png")));
    QVERIFY2(
        clipboard.importClipboardResource(InAppClipboardManager::ConflictPolicyAbort),
        qPrintable(clipboard.lastError()));
    const QVector<WhatSon::Resources::ResourcePackageMetadata> allMetadata =
        importedResourceMetadataForHub(hubPath);
    QCOMPARE(allMetadata.size(), 2);

    const WhatSon::Resources::ResourcePackageMetadata firstResource = firstMetadata.constFirst();
    const WhatSon::Resources::ResourcePackageMetadata secondResource =
        allMetadata.constFirst().resourceId == firstResource.resourceId ? allMetadata.constLast() : allMetadata.constFirst();
    const QString firstResourceId = firstResource.resourceId;
    const QString secondResourceId = secondResource.resourceId;
    const QString firstResourcePath = firstResource.resourcePath;
    const QString secondResourcePath = secondResource.resourcePath;

    QVERIFY(isThirtyTwoCharacterAlnumResourceId(firstResourceId));
    QVERIFY(isThirtyTwoCharacterAlnumResourceId(secondResourceId));
    QVERIFY(firstResourceId != secondResourceId);
    QCOMPARE(firstResourcePath, QStringLiteral(".wsresources/%1.wsresource").arg(firstResourceId));
    QCOMPARE(secondResourcePath, QStringLiteral(".wsresources/%1.wsresource").arg(secondResourceId));
    QCOMPARE(
        firstResource.assetPath,
        QStringLiteral("%1.png").arg(firstResourceId));
    QCOMPARE(
        secondResource.assetPath,
        QStringLiteral("%1.png").arg(secondResourceId));

    const QString firstPackageDirectoryPath = QDir(hubPath).filePath(firstResourcePath);
    const QString secondPackageDirectoryPath = QDir(hubPath).filePath(secondResourcePath);
    QVERIFY(QFileInfo(firstPackageDirectoryPath).isDir());
    QVERIFY(QFileInfo(secondPackageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(firstPackageDirectoryPath).filePath(firstResource.assetPath)).isFile());
    QVERIFY(QFileInfo(QDir(secondPackageDirectoryPath).filePath(secondResource.assetPath)).isFile());

    const QString resourcesListText = resourcesFileTextForHub(hubPath);
    QVERIFY(resourcesListText.contains(firstResourcePath));
    QVERIFY(resourcesListText.contains(secondResourcePath));
}

void WhatSonCppRegressionTests::inAppClipboard_randomizesClipboardResourceNameBeforeConflictPreflight()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardPreflightRandomNameHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    QImage existingClipboardNamedImage(QSize(8, 6), QImage::Format_ARGB32_Premultiplied);
    existingClipboardNamedImage.fill(qRgba(30, 120, 210, 255));
    const QString existingClipboardNamedPath =
        QDir(workspaceDirectory.path()).filePath(QStringLiteral("clipboard-resource.png"));
    QVERIFY(existingClipboardNamedImage.save(existingClipboardNamedPath, "PNG"));

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);

    QVERIFY2(
        clipboard.importUrls(QVariantList{QUrl::fromLocalFile(existingClipboardNamedPath)}),
        qPrintable(clipboard.lastError()));
    const QString existingResourcePath = QStringLiteral(".wsresources/clipboard-resource.wsresource");
    QVERIFY(resourcesFileTextForHub(hubPath).contains(existingResourcePath));

    QImage pastedClipboardImage(QSize(9, 7), QImage::Format_ARGB32_Premultiplied);
    pastedClipboardImage.fill(qRgba(190, 75, 40, 255));
    QVERIFY(clipboard.setImageResource(pastedClipboardImage, QStringLiteral("image/png")));

    QVERIFY2(
        clipboard.importClipboardResource(InAppClipboardManager::ConflictPolicyAbort),
        qPrintable(clipboard.lastError()));

    const QVector<WhatSon::Resources::ResourcePackageMetadata> metadata = importedResourceMetadataForHub(hubPath);
    QCOMPARE(metadata.size(), 2);
    const auto pastedIterator = std::find_if(
        metadata.cbegin(),
        metadata.cend(),
        [](const WhatSon::Resources::ResourcePackageMetadata& item)
        {
            return item.resourceId != QStringLiteral("clipboard-resource");
        });
    QVERIFY(pastedIterator != metadata.cend());
    const QString pastedResourceId = pastedIterator->resourceId;
    const QString pastedResourcePath = pastedIterator->resourcePath;
    const QString pastedAssetPath = pastedIterator->assetPath;

    QVERIFY(isThirtyTwoCharacterAlnumResourceId(pastedResourceId));
    QCOMPARE(pastedResourcePath, QStringLiteral(".wsresources/%1.wsresource").arg(pastedResourceId));
    QCOMPARE(pastedAssetPath, QStringLiteral("%1.png").arg(pastedResourceId));
    QVERIFY(pastedAssetPath != QStringLiteral("clipboard-resource.png"));

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(existingResourcePath));
    QVERIFY(resourcesListText.contains(pastedResourcePath));
}

void WhatSonCppRegressionTests::inAppClipboard_importsNonImageClipboardPayloadThroughManager()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardDocumentHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QByteArray pdfBytes = QByteArrayLiteral("%PDF-1.7\nclipboard document");
    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setResourceBytes(
        pdfBytes,
        QStringLiteral("clipboard-document.pdf"),
        QStringLiteral("application/pdf")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".pdf"));
    clipboard.setCurrentHubPath(hubPath);

    QVERIFY2(
        clipboard.importClipboardResource(InAppClipboardManager::ConflictPolicyAbort),
        qPrintable(clipboard.lastError()));

    const WhatSon::Resources::ResourcePackageMetadata importedResource = singleImportedResourceMetadataForHub(hubPath);
    QCOMPARE(importedResource.type, QStringLiteral("document"));
    QCOMPARE(importedResource.format, QStringLiteral(".pdf"));
    QCOMPARE(importedResource.bucket, QStringLiteral("Document"));
    QVERIFY(importedResource.resourcePath.endsWith(QStringLiteral(".wsresource")));
    QVERIFY(importedResource.assetPath.endsWith(QStringLiteral(".pdf")));
    QVERIFY(!clipboard.hasResource());

    const QString packageDirectoryPath = QDir(hubPath).filePath(importedResource.resourcePath);
    const QString assetPath = QDir(packageDirectoryPath).filePath(importedResource.assetPath);
    QFile assetFile(assetPath);
    QVERIFY(assetFile.open(QIODevice::ReadOnly));
    QCOMPARE(assetFile.readAll(), pdfBytes);

    const QString resourcesListText = resourcesFileTextForHub(hubPath);
    QVERIFY(resourcesListText.contains(importedResource.resourcePath));
}

void WhatSonCppRegressionTests::inAppClipboard_refreshReplacesStaleSnapshotWithSystemClipboardImage()
{
    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setResourceBytes(
        QByteArrayLiteral("%PDF-1.7\nold clipboard document"),
        QStringLiteral("old-clipboard-document.pdf"),
        QStringLiteral("application/pdf")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));

    QClipboard* systemClipboard = QGuiApplication::clipboard();
    QVERIFY(systemClipboard != nullptr);
    systemClipboard->clear();
    QImage clipboardImage(QSize(5, 4), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(70, 88, 210, 255));
    systemClipboard->setImage(clipboardImage);

    QVERIFY(clipboard.refreshClipboardResourceAvailabilitySnapshot());
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    QVERIFY(clipboard.resourceEntry().value(QStringLiteral("hasImage")).toBool());
}
