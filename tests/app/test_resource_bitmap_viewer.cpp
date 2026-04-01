#include "file/viewer/ImageFormatCompatibilityLayer.hpp"
#include "file/viewer/ResourceBitmapViewer.hpp"

#include <QSignalSpy>
#include <QUrl>
#include <QVariantMap>
#include <QtTest/QtTest>

class ResourceBitmapViewerTest final : public QObject
{
    Q_OBJECT

private slots:
    void imageFormatCompatibilityLayer_mustNormalizeAndClassifyFormats();
    void resourceBitmapViewer_mustExposeRenderableBitmapState();
    void resourceBitmapViewer_mustRejectUnsupportedBitmapFormats();
    void resourceBitmapViewer_mustResolveFallbackSourceFromResolvedPath();
};

void ResourceBitmapViewerTest::imageFormatCompatibilityLayer_mustNormalizeAndClassifyFormats()
{
    QCOMPARE(
        WhatSon::Viewer::ImageFormatCompatibilityLayer::normalizedBitmapFormat(QStringLiteral("image/jpeg")),
        QStringLiteral(".jpg"));
    QCOMPARE(
        WhatSon::Viewer::ImageFormatCompatibilityLayer::normalizedBitmapFormat(QStringLiteral("/tmp/Poster.WEBP")),
        QStringLiteral(".webp"));

    QVERIFY(WhatSon::Viewer::ImageFormatCompatibilityLayer::isBitmapFormatCompatible(QStringLiteral(".png")));
    QVERIFY(!WhatSon::Viewer::ImageFormatCompatibilityLayer::isBitmapFormatCompatible(QStringLiteral(".pdf")));
}

void ResourceBitmapViewerTest::resourceBitmapViewer_mustExposeRenderableBitmapState()
{
    ResourceBitmapViewer viewer;
    QSignalSpy stateSpy(&viewer, &ResourceBitmapViewer::viewerStateChanged);

    QVariantMap entry;
    entry.insert(QStringLiteral("renderMode"), QStringLiteral("image"));
    entry.insert(QStringLiteral("format"), QStringLiteral(".PNG"));
    entry.insert(QStringLiteral("source"), QStringLiteral("file:///tmp/poster.png"));

    viewer.setResourceEntry(entry);

    QVERIFY(stateSpy.count() > 0);
    QCOMPARE(viewer.normalizedFormat(), QStringLiteral(".png"));
    QVERIFY(viewer.bitmapFormatCompatible());
    QVERIFY(viewer.bitmapRenderable());
    QCOMPARE(viewer.openTarget(), QStringLiteral("file:///tmp/poster.png"));
    QCOMPARE(viewer.viewerSource(), QStringLiteral("file:///tmp/poster.png"));
    QCOMPARE(viewer.incompatibilityReason(), QString());
}

void ResourceBitmapViewerTest::resourceBitmapViewer_mustRejectUnsupportedBitmapFormats()
{
    ResourceBitmapViewer viewer;

    QVariantMap entry;
    entry.insert(QStringLiteral("renderMode"), QStringLiteral("image"));
    entry.insert(QStringLiteral("format"), QStringLiteral(".whatsonbitmap"));
    entry.insert(QStringLiteral("source"), QStringLiteral("file:///tmp/poster.whatsonbitmap"));

    viewer.setResourceEntry(entry);

    QCOMPARE(viewer.normalizedFormat(), QStringLiteral(".whatsonbitmap"));
    QVERIFY(!viewer.bitmapFormatCompatible());
    QVERIFY(!viewer.bitmapRenderable());
    QVERIFY(viewer.incompatibilityReason().contains(QStringLiteral("Unsupported bitmap format")));
}

void ResourceBitmapViewerTest::resourceBitmapViewer_mustResolveFallbackSourceFromResolvedPath()
{
    ResourceBitmapViewer viewer;

    QVariantMap entry;
    entry.insert(QStringLiteral("renderMode"), QStringLiteral("image"));
    entry.insert(QStringLiteral("resolvedPath"), QStringLiteral("/tmp/cover.jpg"));

    viewer.setResourceEntry(entry);

    const QString expectedOpenTarget = QUrl::fromLocalFile(QStringLiteral("/tmp/cover.jpg")).toString();
    QCOMPARE(viewer.openTarget(), expectedOpenTarget);
    QVERIFY(viewer.bitmapFormatCompatible());
    QVERIFY(viewer.bitmapRenderable());
    QCOMPARE(viewer.viewerSource(), expectedOpenTarget);
}

QTEST_MAIN(ResourceBitmapViewerTest)

#include "test_resource_bitmap_viewer.moc"

