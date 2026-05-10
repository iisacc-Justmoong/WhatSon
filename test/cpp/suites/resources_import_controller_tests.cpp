#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QBuffer>
#include <QMimeData>

void WhatSonCppRegressionTests::resourcesImportController_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/file/import/ResourcesImportController.cpp"));

    QVERIFY(!importControllerSource.isEmpty());
    QVERIFY(importControllerSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importControllerSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
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

    QMimeData imageObjectMimeData;
    imageObjectMimeData.setImageData(sourceImage);

    QImage extractedObjectImage;
    QVERIFY(WhatSon::Resources::ClipboardImportSupport::extractClipboardImage(
        &imageObjectMimeData,
        &extractedObjectImage));
    QCOMPARE(extractedObjectImage.size(), sourceImage.size());
}
