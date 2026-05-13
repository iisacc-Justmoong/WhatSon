#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/file/resource/ResourcesImportController.hpp"

#include <QBuffer>
#include <QMimeData>

namespace
{
    QString readUtf8FileForResourceImportControllerTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }
} // namespace

void WhatSonCppRegressionTests::resourcesImportController_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp"));

    QVERIFY(!importControllerSource.isEmpty());
    QVERIFY(importControllerSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importControllerSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
}

void WhatSonCppRegressionTests::resourcesImportController_editorImageImportRegistersPackageBeforeNoteSourceInsertion()
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

    ResourcesImportController controller;
    controller.setCurrentHubPath(hubPath);

    const QVariantList importedEntries =
        controller.importUrlsForEditor(QVariantList{QUrl::fromLocalFile(capturedImagePath)});
    QVERIFY2(
        importedEntries.size() == 1,
        qPrintable(controller.lastError()));

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
    const QString resourcesListText = readUtf8FileForResourceImportControllerTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(resourcePath));

    NoteEditorDocumentSession session;
    const QString editorHtml = WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
        QStringLiteral("clipboard-note"),
        QStringLiteral("Capture\nReady"));
    const QVariantMap insertion = session.insertImportedResourcesIntoSource(
        editorHtml,
        QStringLiteral("Capture").size(),
        0,
        importedEntries);

    QVERIFY(insertion.value(QStringLiteral("valid")).toBool());
    const QString bodySourceText = insertion.value(QStringLiteral("bodySourceText")).toString();
    QVERIFY(bodySourceText.contains(
        QStringLiteral("path=\"%1\"").arg(resourcePath)));
    QVERIFY(bodySourceText.contains(
        QStringLiteral("id=\"%1\"").arg(resourceId)));
    QVERIFY(!bodySourceText.contains(QStringLiteral("<img")));
    QVERIFY(!bodySourceText.contains(assetPath));
    QCOMPARE(session.parsedLineCount(), 3);
}

void WhatSonCppRegressionTests::resourceClipboardImportSupport_extractsMimeImagePayloads()
{
    QImage sourceImage(QSize(9, 5), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(25, 90, 170, 255));

    QByteArray encodedPng;
    QBuffer buffer(&encodedPng);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(sourceImage.save(&buffer, "PNG"));

    QMimeData platformMimeData;
    platformMimeData.setData(QStringLiteral("public.png"), encodedPng);

    QImage extractedPlatformImage;
    QVERIFY(WhatSon::Resources::ClipboardImportSupport::extractClipboardImage(
        &platformMimeData,
        &extractedPlatformImage));
    QCOMPARE(extractedPlatformImage.size(), sourceImage.size());

    QMimeData genericQtImageMimeData;
    genericQtImageMimeData.setData(QStringLiteral("application/x-qt-image"), encodedPng);

    QImage extractedGenericImage;
    QVERIFY(WhatSon::Resources::ClipboardImportSupport::extractClipboardImage(
        &genericQtImageMimeData,
        &extractedGenericImage));
    QCOMPARE(extractedGenericImage.size(), sourceImage.size());

    QMimeData macScreenshotMimeData;
    macScreenshotMimeData.setData(QStringLiteral("com.apple.tiff"), encodedPng);

    QImage extractedMacScreenshotImage;
    QVERIFY(WhatSon::Resources::ClipboardImportSupport::extractClipboardImage(
        &macScreenshotMimeData,
        &extractedMacScreenshotImage));
    QCOMPARE(extractedMacScreenshotImage.size(), sourceImage.size());

    QMimeData imageObjectMimeData;
    imageObjectMimeData.setImageData(sourceImage);

    QImage extractedObjectImage;
    QVERIFY(WhatSon::Resources::ClipboardImportSupport::extractClipboardImage(
        &imageObjectMimeData,
        &extractedObjectImage));
    QCOMPARE(extractedObjectImage.size(), sourceImage.size());
}
