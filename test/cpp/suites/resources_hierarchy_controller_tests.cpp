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

void WhatSonCppRegressionTests::resourcesHierarchyController_mergesLegacyMusicResourcesIntoAudio()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString legacyPackageDirectoryPath =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("legacy-song.wsresource"));
    const QString audioPackageDirectoryPath =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("field-recording.wsresource"));
    QVERIFY(QDir().mkpath(legacyPackageDirectoryPath));
    QVERIFY(QDir().mkpath(audioPackageDirectoryPath));

    QFile legacyAsset(QDir(legacyPackageDirectoryPath).filePath(QStringLiteral("legacy-song.mp3")));
    QVERIFY(legacyAsset.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(legacyAsset.write(QByteArrayLiteral("legacy music bytes")) >= 0);
    legacyAsset.close();

    QFile audioAsset(QDir(audioPackageDirectoryPath).filePath(QStringLiteral("field-recording.wav")));
    QVERIFY(audioAsset.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(audioAsset.write(QByteArrayLiteral("audio bytes")) >= 0);
    audioAsset.close();

    QFile legacyMetadataFile(WhatSon::Resources::metadataFilePathForPackage(legacyPackageDirectoryPath));
    QVERIFY(legacyMetadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    const QString legacyMetadataXml = QStringLiteral(
        "<?xml version=\"1.0\"?>\n"
        "<wsresource version=\"1\" schema=\"whatson.resource.package\" "
        "id=\"legacy-song\" resourcePath=\"Demo.wsresources/legacy-song.wsresource\" "
        "bucket=\"Music\" type=\"music\" format=\".mp3\">\n"
        "    <asset path=\"legacy-song.mp3\"/>\n"
        "</wsresource>\n");
    QVERIFY(legacyMetadataFile.write(legacyMetadataXml.toUtf8()) >= 0);
    legacyMetadataFile.close();

    QFile audioMetadataFile(WhatSon::Resources::metadataFilePathForPackage(audioPackageDirectoryPath));
    QVERIFY(audioMetadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    const QString audioMetadataXml = QStringLiteral(
        "<?xml version=\"1.0\"?>\n"
        "<wsresource version=\"1\" schema=\"whatson.resource.package\" "
        "id=\"field-recording\" resourcePath=\"Demo.wsresources/field-recording.wsresource\" "
        "bucket=\"Audio\" type=\"audio\" format=\".wav\">\n"
        "    <asset path=\"field-recording.wav\"/>\n"
        "</wsresource>\n");
    QVERIFY(audioMetadataFile.write(audioMetadataXml.toUtf8()) >= 0);
    audioMetadataFile.close();

    ResourcesHierarchyController controller;
    controller.applyRuntimeSnapshot(
        {legacyPackageDirectoryPath, audioPackageDirectoryPath},
        temporaryDirectory.path(),
        true);

    bool foundAudioType = false;
    bool foundMusicType = false;
    bool foundMp3FormatUnderAudio = false;
    bool foundWavFormatUnderAudio = false;
    const QVariantList depthItems = controller.depthItems();
    for (const QVariant& depthItemValue : depthItems)
    {
        const QVariantMap depthItem = depthItemValue.toMap();
        const QString kind = depthItem.value(QStringLiteral("kind")).toString();
        const QString type = depthItem.value(QStringLiteral("type")).toString();
        const QString label = depthItem.value(QStringLiteral("label")).toString();
        if (kind == QStringLiteral("type") && type == QStringLiteral("audio"))
        {
            foundAudioType = true;
            QCOMPARE(label, QStringLiteral("Audio"));
            QCOMPARE(depthItem.value(QStringLiteral("count")).toInt(), 2);
        }
        if (kind == QStringLiteral("type") && type == QStringLiteral("music"))
        {
            foundMusicType = true;
        }
        if (kind == QStringLiteral("format") && type == QStringLiteral("audio") && label == QStringLiteral(".mp3"))
        {
            foundMp3FormatUnderAudio = true;
            QCOMPARE(depthItem.value(QStringLiteral("count")).toInt(), 1);
        }
        if (kind == QStringLiteral("format") && type == QStringLiteral("audio") && label == QStringLiteral(".wav"))
        {
            foundWavFormatUnderAudio = true;
            QCOMPARE(depthItem.value(QStringLiteral("count")).toInt(), 1);
        }
    }

    QVERIFY(foundAudioType);
    QVERIFY(!foundMusicType);
    QVERIFY(foundMp3FormatUnderAudio);
    QVERIFY(foundWavFormatUnderAudio);
}

void WhatSonCppRegressionTests::resourcesHierarchyController_commitsChevronExpansionThroughSharedBridge()
{
    ResourcesHierarchyController resourcesController;
    resourcesController.setResourcePaths({
        QStringLiteral("assets/Cover.png"),
        QStringLiteral("assets/Clip.mov")
    });

    int imageIndex = -1;
    QVariantMap imageNode;
    const QVariantList initialItems = resourcesController.depthItems();
    for (int i = 0; i < initialItems.size(); ++i)
    {
        const QVariantMap item = initialItems.at(i).toMap();
        if (item.value(QStringLiteral("kind")).toString() == QStringLiteral("type")
            && item.value(QStringLiteral("type")).toString() == QStringLiteral("image"))
        {
            imageIndex = i;
            imageNode = item;
            break;
        }
    }

    QVERIFY(imageIndex >= 0);
    QVERIFY(imageNode.value(QStringLiteral("showChevron")).toBool());
    QVERIFY(!imageNode.value(QStringLiteral("expanded")).toBool());

    WhatSon::Policy::ArchitecturePolicyLock::unlockForTests();
    HierarchyInteractionBridge bridge;
    SidebarHierarchyInteractionController interactionController;
    bridge.setHierarchyController(&resourcesController);
    interactionController.setHierarchyInteractionBridge(&bridge);
    interactionController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));

    const QString imageExpansionKey = interactionController.itemExpansionKey(imageNode, imageIndex);
    QCOMPARE(
        imageExpansionKey,
        QStringLiteral("hierarchy:%1:type:image")
            .arg(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources)));

    interactionController.captureExpansionState(resourcesController.depthItems());
    QVERIFY(interactionController.armExpansionKey(imageExpansionKey));
    const QVariantMap expandedResult =
        interactionController.requestChevronExpansion(imageIndex, imageExpansionKey, false, imageExpansionKey);

    QVERIFY(expandedResult.value(QStringLiteral("committed")).toBool());
    QVERIFY(resourcesController.depthItems().at(imageIndex).toMap().value(QStringLiteral("expanded")).toBool());
    QVERIFY(interactionController.expansionStateForKey(imageExpansionKey, false));

    const QVariantMap expandedNode = resourcesController.depthItems().at(imageIndex).toMap();
    QVERIFY(interactionController.armExpansionKey(imageExpansionKey));
    const QVariantMap collapsedResult =
        interactionController.requestChevronExpansion(imageIndex, imageExpansionKey, true, imageExpansionKey);

    QVERIFY(collapsedResult.value(QStringLiteral("committed")).toBool());
    QVERIFY(!resourcesController.depthItems().at(imageIndex).toMap().value(QStringLiteral("expanded")).toBool());
    QVERIFY(!interactionController.expansionStateForKey(imageExpansionKey, true));
    QCOMPARE(
        expandedNode.value(QStringLiteral("key")).toString(),
        imageNode.value(QStringLiteral("key")).toString());
}
