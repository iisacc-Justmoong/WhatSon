#include "test/cpp/whatson_cpp_regression_tests.hpp"

void WhatSonCppRegressionTests::resourceBitmapViewer_projectsRenderableImagePreviewState()
{
    ResourceBitmapViewer viewer;

    viewer.setResourceEntry(QVariantMap{
        {QStringLiteral("displayName"), QStringLiteral("Cover.PNG")},
        {QStringLiteral("renderMode"), QStringLiteral("image")},
        {QStringLiteral("format"), QStringLiteral(".PNG")},
        {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/WhatSon/Cover.PNG")}
    });

    QCOMPARE(viewer.normalizedFormat(), QStringLiteral(".png"));
    QVERIFY(viewer.bitmapPreviewCandidate());
    QVERIFY(viewer.bitmapFormatCompatible());
    QVERIFY(viewer.bitmapRenderable());
    QCOMPARE(
        viewer.openTarget(),
        QUrl::fromLocalFile(QStringLiteral("/tmp/WhatSon/Cover.PNG")).toString());
    QCOMPARE(viewer.viewerSource(), viewer.openTarget());
    QVERIFY(viewer.incompatibilityReason().isEmpty());

    viewer.setResourceEntry(QVariantMap{
        {QStringLiteral("displayName"), QStringLiteral("Source.psd")},
        {QStringLiteral("format"), QStringLiteral(".psd")},
        {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/WhatSon/Source.psd")}
    });

    QVERIFY(viewer.bitmapPreviewCandidate());
    QVERIFY(!viewer.bitmapFormatCompatible());
    QVERIFY(!viewer.bitmapRenderable());
    QVERIFY(!viewer.incompatibilityReason().trimmed().isEmpty());
}
