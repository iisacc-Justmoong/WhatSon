#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QBuffer>
#include <QMimeData>

void WhatSonCppRegressionTests::filetypeCapture_ownsClipboardFileTypeDetection()
{
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::normalizeMimeType(
            QStringLiteral(" Text/HTML; charset=utf-8 ")),
        QStringLiteral("text/html"));
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::formatFromMimeType(QStringLiteral("AUDIO/MPEG")),
        QStringLiteral(".mp3"));
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::normalizedFormatForFileType(
            QStringLiteral("Scene.GLTF"),
            QString()),
        QStringLiteral(".gltf"));
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::normalizedFormatForFileType(
            QString(),
            QStringLiteral("application/pdf")),
        QStringLiteral(".pdf"));
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::normalizedFileNameOrDefault(
            QStringLiteral("../Capture.PNG"),
            QStringLiteral(".png")),
        QStringLiteral("Capture.PNG"));
    QCOMPARE(
        WhatSon::Clipboard::FiletypeCapture::defaultResourceFileName(QStringLiteral(".pdf")),
        QStringLiteral("clipboard-resource.pdf"));
    QVERIFY(WhatSon::Clipboard::FiletypeCapture::mimeTypeLooksLikeImagePayload(
        QStringLiteral("application/x-qt-image")));
    QVERIFY(WhatSon::Clipboard::FiletypeCapture::mimeTypeLooksLikeImagePayload(
        QStringLiteral("com.apple.tiff")));
    QVERIFY(!WhatSon::Clipboard::FiletypeCapture::mimeTypeLooksLikeImagePayload(
        QStringLiteral("application/pdf")));

    const QString importSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/ClipboardResourceImport.cpp"));
    const QString filetypeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/FiletypeCapture.cpp"));
    QVERIFY(!importSource.contains(QStringLiteral("kFormatsByMimeType")));
    QVERIFY(!importSource.contains(QStringLiteral("QString formatFromMimeType(")));
    QVERIFY(filetypeSource.contains(QStringLiteral("kFormatsByMimeType")));
    QVERIFY(filetypeSource.contains(QStringLiteral("QString formatFromMimeType(")));
}

void WhatSonCppRegressionTests::inAppClipboard_extractsPlatformImageMimePayloads()
{
    QImage sourceImage(QSize(9, 5), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(25, 90, 170, 255));

    QByteArray encodedImage;
    QBuffer buffer(&encodedImage);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(sourceImage.save(&buffer, "PNG"));

    InAppClipboardManager clipboard;

    QMimeData genericQtImageMimeData;
    genericQtImageMimeData.setData(QStringLiteral("application/x-qt-image"), encodedImage);
    QVERIFY(clipboard.captureResourceFromMimeData(&genericQtImageMimeData));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    QCOMPARE(clipboard.resourceImport().image.size(), sourceImage.size());

    QMimeData macScreenshotMimeData;
    macScreenshotMimeData.setData(QStringLiteral("com.apple.tiff"), encodedImage);
    QVERIFY(clipboard.captureResourceFromMimeData(&macScreenshotMimeData));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    QCOMPARE(clipboard.resourceImport().image.size(), sourceImage.size());
}

void WhatSonCppRegressionTests::inAppClipboardStore_ownsResourceSnapshotState()
{
    const QString managerHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.h"));
    const QString managerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));
    const QString storeHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardStore.h"));
    const QString storeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardStore.cpp"));

    QVERIFY(managerHeader.contains(QStringLiteral("InAppClipboardStore m_store;")));
    QVERIFY(!managerHeader.contains(QStringLiteral("ClipboardResourceImport m_resourceImport;")));
    QVERIFY(storeHeader.contains(QStringLiteral("ClipboardResourceImport m_resourceImport;")));
    QVERIFY(managerSource.contains(QStringLiteral("return m_store.setResourceImport(std::move(resourceImport));")));
    QVERIFY(managerSource.contains(QStringLiteral("connect(&m_store, &InAppClipboardStore::resourceChanged")));
    QVERIFY(storeSource.contains(QStringLiteral("InAppClipboardStore::setResourceImport")));
    QVERIFY(storeSource.contains(QStringLiteral("emit resourceChanged();")));

    InAppClipboardStore store;
    QSignalSpy resourceChangedSpy(&store, &InAppClipboardStore::resourceChanged);
    QVERIFY(store.setResourceImport(WhatSon::Clipboard::resourceImportForBytes(
        QByteArrayLiteral("clipboard document"),
        QStringLiteral("clipboard-document.txt"),
        QStringLiteral("text/plain"))));
    QCOMPARE(resourceChangedSpy.count(), 1);
    QVERIFY(store.hasResource());
    QCOMPARE(store.resourceFileName(), QStringLiteral("clipboard-document.txt"));
    QCOMPARE(store.resourceType(), QStringLiteral("document"));
    QCOMPARE(store.resourceImport().payloadBytes, QByteArrayLiteral("clipboard document"));

    const WhatSon::Clipboard::ClipboardResourceImport takenResource = store.takeResourceImport();
    QCOMPARE(takenResource.payloadBytes, QByteArrayLiteral("clipboard document"));
    QVERIFY(!store.hasResource());
    QCOMPARE(resourceChangedSpy.count(), 2);
}

void WhatSonCppRegressionTests::inAppClipboard_matchesMimeAndFileTypesToResourceTaxonomy()
{
    InAppClipboardManager clipboard;

    QVERIFY(clipboard.setResourceFileType(QStringLiteral("diagram.GLTF"), QStringLiteral("model/gltf+json")));
    QVERIFY(clipboard.hasResource());
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".gltf"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("model"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("3D Model"));

    QVERIFY(clipboard.setResourceFileType(QString(), QStringLiteral("audio/mpeg")));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".mp3"));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("audio"));
    QCOMPARE(clipboard.resourceBucket(), QStringLiteral("Audio"));
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
    InAppClipboardManager clipboard;

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
