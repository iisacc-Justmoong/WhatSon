#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include "app/models/clipboard/InAppClipboardManager.h"

#include <QClipboard>
#include <QMimeData>

namespace
{
    QString readUtf8FileForInAppClipboardImportTest(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }
} // namespace

void WhatSonCppRegressionTests::inAppClipboard_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));

    QVERIFY(!clipboardSource.isEmpty());
    QVERIFY(clipboardSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(clipboardSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/file/resource/ResourcesImportController.hpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/InAppClipboardResourceImport.cpp")).exists());
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp")).exists());
}

void WhatSonCppRegressionTests::inAppClipboard_importsUrlsForEditorAsResourcePackages()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("CaptureHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    QImage sourceImage(QSize(7, 5), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(40, 120, 210, 255));
    const QString capturedImagePath = QDir(workspaceDirectory.path()).filePath(QStringLiteral("capture.png"));
    QVERIFY(sourceImage.save(capturedImagePath, "PNG"));

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);

    const QVariantList importedEntries =
        clipboard.importUrlsForEditor(QVariantList{QUrl::fromLocalFile(capturedImagePath)});
    QVERIFY2(
        importedEntries.size() == 1,
        qPrintable(clipboard.lastError()));

    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    const QString resourceId = importedResource.value(QStringLiteral("resourceId")).toString();
    const QString resourcePath = importedResource.value(QStringLiteral("resourcePath")).toString();
    const QString assetPath = importedResource.value(QStringLiteral("assetPath")).toString();

    QVERIFY(!resourceId.isEmpty());
    QVERIFY(resourcePath.startsWith(QStringLiteral(".wsresources/")));
    QVERIFY(resourcePath.endsWith(QStringLiteral(".wsresource")));
    QVERIFY(assetPath.endsWith(QStringLiteral(".png")));
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));

    const QString packageDirectoryPath = QDir(hubPath).filePath(resourcePath);
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(assetPath)).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName())).isFile());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(WhatSon::Resources::annotationFileName())).isFile());

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(resourcePath));

    const QString clipboardHeader = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.h"));
    const QString clipboardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/clipboard/InAppClipboardManager.cpp"));
    QVERIFY(clipboardHeader.contains(QStringLiteral("importClipboardResourceForEditor")));
    QVERIFY(clipboardHeader.contains(QStringLiteral("importUrlsForEditor")));
    QVERIFY(!clipboardHeader.contains(QStringLiteral("importClipboardImage")));
    QVERIFY(!QFileInfo(QStringLiteral("src/app/models/clipboard/ClipboardResourcePackageImport.cpp")).exists());
    QVERIFY(!clipboardSource.contains(QStringLiteral("ClipboardImportSupport")));
    QVERIFY(!clipboardSource.contains(QStringLiteral("importClipboardImageInternal")));
}

void WhatSonCppRegressionTests::inAppClipboard_importsClipboardImageThroughManager()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    QImage clipboardImage(QSize(9, 6), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(210, 45, 120, 255));

    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    clipboard.setCurrentHubPath(hubPath);

    const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
    QVERIFY2(importedEntries.size() == 1, qPrintable(clipboard.lastError()));

    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));
    QCOMPARE(importedResource.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));
    QVERIFY(importedResource.value(QStringLiteral("resourcePath")).toString().endsWith(QStringLiteral(".wsresource")));
    QVERIFY(importedResource.value(QStringLiteral("assetPath")).toString().endsWith(QStringLiteral(".png")));
    QVERIFY(!clipboard.hasResource());

    const QString packageDirectoryPath =
        QDir(hubPath).filePath(importedResource.value(QStringLiteral("resourcePath")).toString());
    QVERIFY(QFileInfo(packageDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(packageDirectoryPath).filePath(
        importedResource.value(QStringLiteral("assetPath")).toString())).isFile());

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
}

void WhatSonCppRegressionTests::inAppClipboard_importsNonImageClipboardPayloadThroughManager()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardDocumentHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QByteArray pdfBytes = QByteArrayLiteral("%PDF-1.7\nclipboard document");
    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setResourceBytes(
        pdfBytes,
        QStringLiteral("clipboard-document.pdf"),
        QStringLiteral("application/pdf")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".pdf"));
    clipboard.setCurrentHubPath(hubPath);

    const QVariantList importedEntries = clipboard.importClipboardResourceForEditor();
    QVERIFY2(importedEntries.size() == 1, qPrintable(clipboard.lastError()));

    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("document"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".pdf"));
    QCOMPARE(importedResource.value(QStringLiteral("bucket")).toString(), QStringLiteral("Document"));
    QVERIFY(importedResource.value(QStringLiteral("resourcePath")).toString().endsWith(QStringLiteral(".wsresource")));
    QVERIFY(importedResource.value(QStringLiteral("assetPath")).toString().endsWith(QStringLiteral(".pdf")));
    QVERIFY(!clipboard.hasResource());

    const QString packageDirectoryPath =
        QDir(hubPath).filePath(importedResource.value(QStringLiteral("resourcePath")).toString());
    const QString assetPath = QDir(packageDirectoryPath).filePath(
        importedResource.value(QStringLiteral("assetPath")).toString());
    QFile assetFile(assetPath);
    QVERIFY(assetFile.open(QIODevice::ReadOnly));
    QCOMPARE(assetFile.readAll(), pdfBytes);

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
}

void WhatSonCppRegressionTests::clipboardEditorPaste_insertsImageResourceThroughPasteObject()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardEditorPasteHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("clipboard-editor-paste-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QImage clipboardImage(QSize(12, 8), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(80, 20, 190, 255));

    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setImageResource(clipboardImage, QStringLiteral("image/png")));
    clipboard.setCurrentHubPath(hubPath);

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("clipboard-editor-paste-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    ClipboardEditorPaste editorPaste;
    const QString editorHtml = readUtf8FileForInAppClipboardImportTest(session.editorFilePath());
    const QVariantMap result = editorPaste.pasteImageResourceIntoEditor(
        &clipboard,
        &session,
        editorHtml,
        QStringLiteral("Alpha").size(),
        0);

    QVERIFY2(result.value(QStringLiteral("valid")).toBool(), qPrintable(result.value(QStringLiteral("errorMessage")).toString()));
    QCOMPARE(result.value(QStringLiteral("nativePaste")).toBool(), false);
    QCOMPARE(result.value(QStringLiteral("changed")).toBool(), true);
    QCOMPARE(result.value(QStringLiteral("reloadSucceeded")).toBool(), true);
    QVERIFY(!clipboard.hasResource());

    const QVariantList importedEntries = result.value(QStringLiteral("importedEntries")).toList();
    QCOMPARE(importedEntries.size(), 1);
    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));
    QVERIFY(importedResource.value(QStringLiteral("resourcePath")).toString().endsWith(QStringLiteral(".wsresource")));

    const QString bodySourceText = result.value(QStringLiteral("bodySourceText")).toString();
    const QString editorDocumentText = result.value(QStringLiteral("editorDocumentText")).toString();
    QVERIFY(bodySourceText.contains(QStringLiteral("<resource type=\"image\" format=\".png\"")));
    QVERIFY(bodySourceText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(editorDocumentText.contains(QStringLiteral("whatson-resource-frame")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("<img src=\"file://")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("width=\"12\"")));
    QVERIFY(editorDocumentText.contains(QStringLiteral("height=\"8\"")));

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
}

void WhatSonCppRegressionTests::clipboardEditorPaste_capturesSystemClipboardImageForEditorPaste()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());
    QTemporaryDir sessionRootDir;
    QVERIFY(sessionRootDir.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("SystemClipboardEditorPasteHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QString libraryDirectoryPath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents")))
            .filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = createLocalNoteForRegression(
        libraryDirectoryPath,
        QStringLiteral("system-clipboard-editor-paste-note"),
        QStringLiteral("Alpha\nBeta"),
        &createError);
    QVERIFY2(!noteDirectoryPath.isEmpty(), qPrintable(createError));

    QClipboard* systemClipboard = QGuiApplication::clipboard();
    QVERIFY(systemClipboard != nullptr);
    systemClipboard->clear();
    QImage clipboardImage(QSize(13, 9), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(12, 140, 70, 255));
    systemClipboard->setImage(clipboardImage);

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);
    QVERIFY(!clipboard.hasResource());

    NoteEditorDocumentSession session;
    session.setSessionRootPathForTests(sessionRootDir.path());

    QSignalSpy loadedSpy(&session, &NoteEditorDocumentSession::editorSourceLoaded);
    QVERIFY(session.openNoteForEditing(QStringLiteral("system-clipboard-editor-paste-note"), noteDirectoryPath));
    QTRY_COMPARE_WITH_TIMEOUT(loadedSpy.count(), 1, 3000);

    ClipboardEditorPaste editorPaste;
    const QString editorHtml = readUtf8FileForInAppClipboardImportTest(session.editorFilePath());
    const QVariantMap result = editorPaste.pasteImageResourceIntoEditor(
        &clipboard,
        &session,
        editorHtml,
        QStringLiteral("Alpha").size(),
        0);

    QVERIFY2(result.value(QStringLiteral("valid")).toBool(), qPrintable(result.value(QStringLiteral("errorMessage")).toString()));
    QCOMPARE(result.value(QStringLiteral("nativePaste")).toBool(), false);
    QVERIFY(!clipboard.hasResource());

    const QVariantList importedEntries = result.value(QStringLiteral("importedEntries")).toList();
    QCOMPARE(importedEntries.size(), 1);
    const QVariantMap importedResource = importedEntries.constFirst().toMap();
    QCOMPARE(importedResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(importedResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));
    QVERIFY(result.value(QStringLiteral("bodySourceText")).toString().contains(
        importedResource.value(QStringLiteral("resourcePath")).toString()));
    QVERIFY(result.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("width=\"13\"")));
    QVERIFY(result.value(QStringLiteral("editorDocumentText")).toString().contains(QStringLiteral("height=\"9\"")));

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(resourcesListText.contains(importedResource.value(QStringLiteral("resourcePath")).toString()));
}

void WhatSonCppRegressionTests::inAppClipboard_refreshReplacesStaleSnapshotWithSystemClipboardImage()
{
    InAppClipboardManager clipboard;
    QVERIFY(clipboard.setResourceBytes(
        QByteArrayLiteral("%PDF-1.7\nold clipboard document"),
        QStringLiteral("old-clipboard-document.pdf"),
        QStringLiteral("application/pdf")));
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));

    QClipboard* systemClipboard = QGuiApplication::clipboard();
    QVERIFY(systemClipboard != nullptr);
    systemClipboard->clear();
    QImage clipboardImage(QSize(5, 4), QImage::Format_ARGB32_Premultiplied);
    clipboardImage.fill(qRgba(70, 88, 210, 255));
    systemClipboard->setImage(clipboardImage);

    QVERIFY(clipboard.refreshClipboardResourceAvailabilitySnapshot());
    QCOMPARE(clipboard.resourceType(), QStringLiteral("image"));
    QCOMPARE(clipboard.resourceFormat(), QStringLiteral(".png"));
    QVERIFY(clipboard.resourceEntry().value(QStringLiteral("hasImage")).toBool());
}

void WhatSonCppRegressionTests::clipboardEditorPaste_fallsBackForNonImageResource()
{
    QTemporaryDir workspaceDirectory;
    QVERIFY(workspaceDirectory.isValid());

    QString createError;
    const QString hubPath = createMinimalHubFixture(
        workspaceDirectory.path(),
        QStringLiteral("ClipboardEditorPasteDocumentHub.wshub"),
        &createError);
    QVERIFY2(!hubPath.isEmpty(), qPrintable(createError));

    const QByteArray pdfBytes = QByteArrayLiteral("%PDF-1.7\nclipboard document");
    QClipboard* systemClipboard = QGuiApplication::clipboard();
    QVERIFY(systemClipboard != nullptr);
    systemClipboard->clear();
    auto* mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("application/pdf"), pdfBytes);
    systemClipboard->setMimeData(mimeData);

    InAppClipboardManager clipboard;
    clipboard.setCurrentHubPath(hubPath);

    NoteEditorDocumentSession session;
    ClipboardEditorPaste editorPaste;
    const QVariantMap result = editorPaste.pasteImageResourceIntoEditor(
        &clipboard,
        &session,
        QStringLiteral("Alpha"),
        5,
        0);

    QCOMPARE(result.value(QStringLiteral("valid")).toBool(), false);
    QCOMPARE(result.value(QStringLiteral("nativePaste")).toBool(), true);
    QVERIFY(result.value(QStringLiteral("errorMessage")).toString().contains(QStringLiteral("Only image clipboard resources")));
    QVERIFY(clipboard.hasResource());
    QCOMPARE(clipboard.resourceType(), QStringLiteral("document"));

    const QString resourcesFilePath =
        QDir(QDir(hubPath).filePath(QStringLiteral(".wscontents"))).filePath(QStringLiteral("Resources.wsresources"));
    const QString resourcesListText = readUtf8FileForInAppClipboardImportTest(resourcesFilePath);
    QVERIFY(!resourcesListText.contains(QStringLiteral(".wsresource")));
}
