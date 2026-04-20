#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcePackageSupport_roundTripsAnnotationMetadataAndBitmap()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString assetFilePath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("cover.png"));
    QImage sourceImage(QSize(13, 7), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(12, 34, 56, 255));
    QVERIFY(sourceImage.save(assetFilePath));

    const WhatSon::Resources::ResourcePackageMetadata metadata =
        WhatSon::Resources::buildMetadataForAssetFile(
            assetFilePath,
            QStringLiteral("cover"),
            QStringLiteral("Demo.wsresources/cover.wsresource"));
    QCOMPARE(metadata.assetPath, QStringLiteral("cover.png"));
    QCOMPARE(metadata.annotationPath, QStringLiteral("annotation.png"));

    const QString metadataXml = WhatSon::Resources::createResourcePackageMetadataXml(metadata);
    QVERIFY(metadataXml.contains(QStringLiteral("<annotation path=\"annotation.png\"/>")));

    WhatSon::Resources::ResourcePackageMetadata parsedMetadata;
    QString parseError;
    QVERIFY2(
        WhatSon::Resources::parseResourcePackageMetadataXml(metadataXml, &parsedMetadata, &parseError),
        qPrintable(parseError));
    QCOMPARE(parsedMetadata.annotationPath, QStringLiteral("annotation.png"));

    const QByteArray annotationBytes =
        WhatSon::Resources::createEmptyAnnotationBitmapPngBytes(assetFilePath);
    QVERIFY(!annotationBytes.isEmpty());

    QImage annotationImage;
    QVERIFY(annotationImage.loadFromData(annotationBytes, "PNG"));
    QCOMPARE(annotationImage.size(), sourceImage.size());
    QCOMPARE(qAlpha(annotationImage.pixel(0, 0)), 0);
    QCOMPARE(
        qAlpha(annotationImage.pixel(annotationImage.width() - 1, annotationImage.height() - 1)),
        0);

    const QString packageDirectoryPath =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("cover.wsresource"));
    QVERIFY(QDir().mkpath(packageDirectoryPath));
    QVERIFY(QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(QStringLiteral("cover.png"))));

    QString writeError;
    QVERIFY2(
        WhatSon::Resources::writeResourcePackageAnnotationBitmap(
            packageDirectoryPath,
            assetFilePath,
            &writeError),
        qPrintable(writeError));

    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(metadataFile.write(metadataXml.toUtf8()) >= 0);
    metadataFile.close();

    WhatSon::Resources::ResourcePackageMetadata loadedMetadata;
    QString loadError;
    QVERIFY2(
        WhatSon::Resources::loadResourcePackageMetadata(
            packageDirectoryPath,
            &loadedMetadata,
            &loadError),
        qPrintable(loadError));
    QCOMPARE(loadedMetadata.assetPath, QStringLiteral("cover.png"));
    QCOMPARE(loadedMetadata.annotationPath, QStringLiteral("annotation.png"));
    QVERIFY(QFileInfo(WhatSon::Resources::annotationFilePathForPackage(packageDirectoryPath)).isFile());
}
