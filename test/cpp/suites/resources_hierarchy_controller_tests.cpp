#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcesHierarchyController_defaultsSelectionToImageAndFiltersList()
{
    ResourcesHierarchyController controller;
    controller.setResourcePaths({
        QStringLiteral("images/Cover.PNG"),
        QStringLiteral("documents/Report.pdf")
    });

    QCOMPARE(controller.selectedIndex(), 0);
    QCOMPARE(controller.itemLabel(0), QStringLiteral("Image"));
    QVERIFY(controller.noteListModel() != nullptr);
    QCOMPARE(controller.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = controller.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        controller.noteListModel()->data(imageIndex, ResourcesListModel::TypeRole).toString(),
        QStringLiteral("image"));

    controller.setSelectedIndex(-1);
    QCOMPARE(controller.selectedIndex(), 0);
    QCOMPARE(controller.noteListModel()->itemCount(), 1);
}

void WhatSonCppRegressionTests::resourcesHierarchyController_collapsesMultiDotImageFormatsIntoTerminalSuffix()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString packageDirectoryPath =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("simulator-screenshot.wsresource"));
    QVERIFY(QDir().mkpath(packageDirectoryPath));

    const QString assetFileName = QStringLiteral("Simulator Screenshot - iPhone 16 Pro Max - 2025-06-17 at 11.25.16.png");
    const QString assetFilePath = QDir(packageDirectoryPath).filePath(assetFileName);
    QImage sourceImage(QSize(11, 7), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(145, 37, 90, 255));
    QVERIFY(sourceImage.save(assetFilePath));

    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    const QString poisonedMetadataXml = QStringLiteral(
        "<?xml version=\"1.0\"?>\n"
        "<wsresource version=\"1\" schema=\"whatson.resource.package\" "
        "id=\"simulator-screenshot\" resourcePath=\"Demo.wsresources/simulator-screenshot.wsresource\" "
        "bucket=\"Image\" type=\"image\" format=\".25.16.png\">\n"
        "    <asset path=\"%1\"/>\n"
        "</wsresource>\n").arg(assetFileName);
    QVERIFY(metadataFile.write(poisonedMetadataXml.toUtf8()) >= 0);
    metadataFile.close();

    ResourcesHierarchyController controller;
    controller.applyRuntimeSnapshot({packageDirectoryPath}, temporaryDirectory.path(), true);

    bool foundPngFormat = false;
    bool foundPoisonedFormat = false;
    const QVariantList depthItems = controller.depthItems();
    for (const QVariant& depthItemValue : depthItems)
    {
        const QVariantMap depthItem = depthItemValue.toMap();
        if (depthItem.value(QStringLiteral("kind")).toString() != QStringLiteral("format"))
        {
            continue;
        }

        const QString label = depthItem.value(QStringLiteral("label")).toString();
        const int count = depthItem.value(QStringLiteral("count")).toInt();
        if (label == QStringLiteral(".png"))
        {
            foundPngFormat = true;
            QCOMPARE(count, 1);
        }
        if (label == QStringLiteral(".25.16.png"))
        {
            foundPoisonedFormat = true;
        }
    }

    QVERIFY(foundPngFormat);
    QVERIFY(!foundPoisonedFormat);
    QCOMPARE(controller.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = controller.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        controller.noteListModel()->data(imageIndex, ResourcesListModel::FormatRole).toString(),
        QStringLiteral(".png"));
}
