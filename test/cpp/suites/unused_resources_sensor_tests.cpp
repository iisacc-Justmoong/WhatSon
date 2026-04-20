#include "../whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::unusedResourcesSensor_reportsHubPackagesMissingFromAllNoteEmbeddings()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString resourcesDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesDirectoryPath));

    auto createResourcePackage = [&](const QString& resourceId, const QRgb fillColor) -> QString
    {
        const QString assetFilePath =
            QDir(workspaceDir.path()).filePath(QStringLiteral("%1.png").arg(resourceId));
        QImage image(QSize(11, 7), QImage::Format_ARGB32_Premultiplied);
        image.fill(fillColor);
        if (!image.save(assetFilePath))
        {
            return {};
        }

        const QString packageDirectoryPath =
            QDir(resourcesDirectoryPath).filePath(QStringLiteral("%1.wsresource").arg(resourceId));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return {};
        }

        const QString resourcePath =
            WhatSon::Resources::resourcePathForPackageDirectory(packageDirectoryPath);
        const WhatSon::Resources::ResourcePackageMetadata metadata =
            WhatSon::Resources::buildMetadataForAssetFile(assetFilePath, resourceId, resourcePath);

        if (!QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(metadata.assetPath)))
        {
            return {};
        }

        QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
        if (!metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return {};
        }
        if (metadataFile.write(
                WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8())
            < 0)
        {
            return {};
        }
        metadataFile.close();

        QString annotationError;
        if (!WhatSon::Resources::writeResourcePackageAnnotationBitmap(
                packageDirectoryPath,
                assetFilePath,
                &annotationError))
        {
            return {};
        }
        return resourcePath;
    };

    const QString usedResourcePath = createResourcePackage(QStringLiteral("used-cover"), qRgba(20, 60, 90, 255));
    const QString unusedResourcePath = createResourcePackage(QStringLiteral("unused-cover"), qRgba(90, 60, 20, 255));
    const QString hiddenOnlyResourcePath = createResourcePackage(QStringLiteral("hidden-only"), qRgba(40, 90, 30, 255));
    QVERIFY(!usedResourcePath.isEmpty());
    QVERIFY(!unusedResourcePath.isEmpty());
    QVERIFY(!hiddenOnlyResourcePath.isEmpty());

    QString createError;
    const QString usedNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("used-note"),
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />\nvisible body")
            .arg(usedResourcePath),
        &createError);
    QVERIFY2(
        !usedNoteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create used note fixture: %1").arg(createError)));

    const QString hiddenNotesDirectoryPath = QDir(contentsDirectoryPath).filePath(QStringLiteral(".archive"));
    QVERIFY(QDir().mkpath(hiddenNotesDirectoryPath));

    createError.clear();
    const QString hiddenNoteDirectoryPath = createLocalNoteForRegression(
        hiddenNotesDirectoryPath,
        QStringLiteral("hidden-note"),
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />").arg(hiddenOnlyResourcePath),
        &createError);
    QVERIFY2(
        !hiddenNoteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create hidden note fixture: %1").arg(createError)));

    UnusedResourcesSensor sensor;
    QSignalSpy unusedResourcesChangedSpy(&sensor, &UnusedResourcesSensor::unusedResourcesChanged);
    QSignalSpy scanCompletedSpy(&sensor, &UnusedResourcesSensor::scanCompleted);

    sensor.setHubPath(hubPath);

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 2);
    const QStringList expectedUnusedResourcePaths{
        hiddenOnlyResourcePath,
        unusedResourcePath
    };
    QCOMPARE(
        sensor.unusedResourcePaths(),
        expectedUnusedResourcePaths);
    QCOMPARE(unusedResourcesChangedSpy.count(), 1);
    QCOMPARE(scanCompletedSpy.count(), 1);

    const QVariantList unusedEntries = sensor.unusedResources();
    QCOMPARE(unusedEntries.size(), 2);

    const QVariantMap firstEntry = unusedEntries.at(0).toMap();
    QCOMPARE(firstEntry.value(QStringLiteral("resourcePath")).toString(), hiddenOnlyResourcePath);
    QCOMPARE(firstEntry.value(QStringLiteral("annotationPath")).toString(), QStringLiteral("annotation.png"));
    QVERIFY(firstEntry.value(QStringLiteral("metadataValid")).toBool());
    QVERIFY(QFileInfo(firstEntry.value(QStringLiteral("assetAbsolutePath")).toString()).isFile());

    const QVariantList completedEntries = scanCompletedSpy.constFirst().constFirst().toList();
    QCOMPARE(completedEntries.size(), 2);
}

void WhatSonCppRegressionTests::unusedResourcesSensor_refreshesAfterRawBodyEmbedsAResource()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString resourcesDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesDirectoryPath));

    const QString assetFilePath = QDir(workspaceDir.path()).filePath(QStringLiteral("loose-image.png"));
    QImage image(QSize(9, 9), QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(80, 20, 140, 255));
    QVERIFY(image.save(assetFilePath));

    const QString packageDirectoryPath =
        QDir(resourcesDirectoryPath).filePath(QStringLiteral("loose-image.wsresource"));
    QVERIFY(QDir().mkpath(packageDirectoryPath));

    const QString resourcePath =
        WhatSon::Resources::resourcePathForPackageDirectory(packageDirectoryPath);
    const WhatSon::Resources::ResourcePackageMetadata metadata =
        WhatSon::Resources::buildMetadataForAssetFile(
            assetFilePath,
            QStringLiteral("loose-image"),
            resourcePath);

    QVERIFY(QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(metadata.assetPath)));
    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(metadataFile.write(WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8()) >= 0);
    metadataFile.close();

    QString annotationError;
    QVERIFY2(
        WhatSon::Resources::writeResourcePackageAnnotationBitmap(
            packageDirectoryPath,
            assetFilePath,
            &annotationError),
        qPrintable(annotationError));

    QString createError;
    const QString noteId = QStringLiteral("dynamic-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        noteId,
        QStringLiteral("body-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    UnusedResourcesSensor sensor;
    QSignalSpy unusedResourcesChangedSpy(&sensor, &UnusedResourcesSensor::unusedResourcesChanged);
    QSignalSpy scanCompletedSpy(&sensor, &UnusedResourcesSensor::scanCompleted);

    sensor.setHubPath(hubPath);

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 1);
    QCOMPARE(sensor.unusedResourcePaths(), QStringList{resourcePath});

    const QString bodyFilePath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    QFile bodyFile(bodyFilePath);
    QVERIFY(bodyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(
        bodyFile.write(
            WhatSon::NoteBodyPersistence::serializeBodyDocument(
                noteId,
                QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />").arg(resourcePath))
                .toUtf8())
        >= 0);
    bodyFile.close();

    sensor.refresh();

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 0);
    QVERIFY(sensor.unusedResources().isEmpty());
    QCOMPARE(sensor.collectUnusedResourcePaths(), QStringList{});
    QCOMPARE(unusedResourcesChangedSpy.count(), 2);
    QCOMPARE(scanCompletedSpy.count(), 3);
}
