#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/clipboard/InAppClipboard.h"

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
} // namespace

void WhatSonCppRegressionTests::inAppClipboard_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp"));

    QVERIFY(!importControllerSource.isEmpty());
    QVERIFY(importControllerSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importControllerSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.hpp")).exists());
}

void WhatSonCppRegressionTests::inAppClipboard_importsUrlsForEditorAsResourcePackages()
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
    const QString capturedImagePath = QDir(workspaceDirectory.path()).filePath(QStringLiteral("capture.png"));
    QVERIFY(sourceImage.save(capturedImagePath, "PNG"));

    InAppClipboard clipboard;
    clipboard.setCurrentHubPath(hubPath);

    const QVariantList importedEntries =
        clipboard.importUrlsForEditor(QVariantList{QUrl::fromLocalFile(capturedImagePath)});
    QVERIFY2(
        importedEntries.size() == 1,
        qPrintable(clipboard.lastError()));

    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    const QString resourceId = importedResource.value(QStringLiteral("resourceId")).toString();
    const QString resourcePath = importedResource.value(QStringLiteral("resourcePath")).toString();
    const QString assetPath = importedResource.value(QStringLiteral("assetPath")).toString();

    QVERIFY(!resourceId.isEmpty());
    QVERIFY(resourcePath.startsWith(QStringLiteral(".wsresources/")));
    QVERIFY(resourcePath.endsWith(QStringLiteral(".wsresource")));
    QVERIFY(assetPath.endsWith(QStringLiteral(".png")));
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));

    const QString packageDirectoryPath = QDir(hubPath).filePath(resourcePath);
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(assetPath)).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName())).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::annotationFileName())).isFile());

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(resourcePath));

    const QString clipboardHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboard.h"));
    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp"));
    QVERIFY(clipboardHeader.contains(QStringLiteral("importClipboardResourceForEditor")));
    QVERIFY(clipboardHeader.contains(QStringLiteral("importUrlsForEditor")));
    QVERIFY(!clipboardHeader.contains(QStringLiteral("importClipboardImage")));
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

    InAppClipboard clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    clipboard.setCurrentHubPath(hubPath);

    const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
    QVERIFY2(importedEntries.size() == 1, qPrintable(clipboard.lastError()));

    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));
    QCOMPARE(importedResource.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));
    QVERIFY(importedResource.value(QStringLiteral("resourcePath")).toString().endsWith(QStringLiteral(".wsresource")));
    QVERIFY(importedResource.value(QStringLiteral("assetPath")).toString().endsWith(QStringLiteral(".png")));
    QVERIFY(!clipboard.hasResource());

    const QString packageDirectoryPath =
        QDir(hubPath).filePath(importedResource.value(QStringLiteral("resourcePath")).toString());
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(
        importedResource.value(QStringLiteral("assetPath")).toString())).isFile());

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
}
