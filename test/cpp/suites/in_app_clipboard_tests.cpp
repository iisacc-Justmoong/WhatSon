#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QMimeData>

void WhatSonCppRegressionTests::inAppClipboard_matchesMimeAndFileTypesToResourceTaxonomy()
{
    InAppClipboard clipboard;

    QVERIFY(clipboard.setResourceFileType(QStringLiteral("diagram.GLTF"), QStringLiteral("model/gltf+json")));
    QVERIFY(clipboard.hasResource());
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".gltf"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("model"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("3D Model"));

    QVERIFY(clipboard.setResourceFileType(QString(), QStringLiteral("audio/mpeg")));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".mp3"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("music"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("Music"));
    QVERIFY(clipboard.resourceFileName().endsWith(QStringLiteral(".mp3")));

    QMimeData mimeData;
    mimeData.setData(QStringLiteral("application/pdf"), QByteArrayLiteral("%PDF-1.7"));
    QVERIFY(clipboard.captureResourceFromMimeData(&mimeData));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".pdf"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("Document"));
    QCOMPARE(clipboard.resourceEntry().value(QStringLiteral("mimeType")).toString(), QStringLiteral("application/pdf"));

    clipboard.clear();
    QVERIFY(!clipboard.hasResource());
    QVERIFY(clipboard.resourceType().isEmpty());
}

void WhatSonCppRegressionTests::inAppClipboard_acceptsNonImagePayloadsFromAppAndMimeData()
{
    InAppClipboard clipboard;

    QVERIFY(clipboard.setResourceBytes(
        QByteArrayLiteral("%PDF-1.7\nclipboard document"),
        QStringLiteral("clipboard-document.pdf"),
        QStringLiteral("application/pdf")));
    QVERIFY(clipboard.hasResource());
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".pdf"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("Document"));
    QCOMPARE(clipboard.resourceImport().payloadBytes, QByteArrayLiteral("%PDF-1.7\nclipboard document"));
    QVERIFY(!clipboard.resourceEntry().value(QStringLiteral("hasImage")).toBool());
    QVERIFY(clipboard.resourceEntry().value(QStringLiteral("hasPayloadBytes")).toBool());

    QMimeData plainTextMimeData;
    plainTextMimeData.setText(QStringLiteral("Plain clipboard text"));
    QVERIFY(clipboard.captureResourceFromMimeData(&plainTextMimeData));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".txt"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));
    QCOMPARE(clipboard.resourceImport().payloadBytes, QByteArrayLiteral("Plain clipboard text"));

    QMimeData htmlMimeData;
    htmlMimeData.setHtml(QStringLiteral("<p>Clipboard <strong>HTML</strong></p>"));
    QVERIFY(clipboard.captureResourceFromMimeData(&htmlMimeData));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".html"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("link"));
    QCOMPARE(
        clipboard.resourceImport().payloadBytes,
        QByteArrayLiteral("<p>Clipboard <strong>HTML</strong></p>"));
}
