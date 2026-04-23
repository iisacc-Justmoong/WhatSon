#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourcesHierarchyViewModel_defaultsSelectionToImageAndFiltersList()
{
    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({
        QStringLiteral("images/Cover.PNG"),
        QStringLiteral("documents/Report.pdf")
    });

    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Image"));
    QVERIFY(viewModel.noteListModel() != nullptr);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = viewModel.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        viewModel.noteListModel()->data(imageIndex, ResourcesListModel::TypeRole).toString(),
        QStringLiteral("image"));

    viewModel.setSelectedIndex(-1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);
}

void WhatSonCppRegressionTests::resourcesHierarchyViewModel_collapsesMultiDotImageFormatsIntoTerminalSuffix()
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

    ResourcesHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot({packageDirectoryPath}, temporaryDirectory.path(), true);

    bool foundPngFormat = false;
    bool foundPoisonedFormat = false;
    const QVariantList depthItems = viewModel.depthItems();
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
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = viewModel.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        viewModel.noteListModel()->data(imageIndex, ResourcesListModel::FormatRole).toString(),
        QStringLiteral(".png"));
}
