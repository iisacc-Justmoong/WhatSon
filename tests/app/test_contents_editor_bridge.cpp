#include "file/viewer/ContentsBodyResourceRenderer.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QObject>
#include <QTemporaryDir>
#include <QVariantList>
#include <QVariantMap>
#include <QUrl>
#include <QtTest/QtTest>

#include <algorithm>
#include <type_traits>

namespace
{
    bool writeUtf8File(const QString& filePath, const QString& text)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }
        file.write(text.toUtf8());
        return true;
    }

    bool createResourcePackage(
        const QString& resourcesPath,
        const QString& resourceId,
        const QString& assetFileName,
        const QString& assetPayload = QStringLiteral("payload"))
    {
        const QString packageDirectoryPath = QDir(resourcesPath).filePath(resourceId + QStringLiteral(".wsresource"));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return false;
        }

        const QString resourcePath = QStringLiteral("%1/%2")
                                         .arg(QFileInfo(resourcesPath).fileName(), QFileInfo(packageDirectoryPath).fileName());
        WhatSon::Resources::ResourcePackageMetadata metadata = WhatSon::Resources::buildMetadataForAssetFile(
            assetFileName,
            resourceId,
            resourcePath);

        return writeUtf8File(QDir(packageDirectoryPath).filePath(assetFileName), assetPayload)
            && writeUtf8File(
                QDir(packageDirectoryPath).filePath(WhatSon::Resources::metadataFileName()),
                WhatSon::Resources::createResourcePackageMetadataXml(metadata));
    }
} // namespace

static_assert(
    !std::is_final_v<ContentsBodyResourceRenderer>,
    "ContentsBodyResourceRenderer must remain non-final for qmlRegisterType wrapper generation.");

class FakeNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText WRITE setCurrentBodyText NOTIFY currentBodyTextChanged)
    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)

public:
    QString currentNoteId() const
    {
        return m_currentNoteId;
    }

    void setCurrentNoteId(const QString& noteId)
    {
        if (m_currentNoteId == noteId)
        {
            return;
        }
        m_currentNoteId = noteId;
        emit currentNoteIdChanged();
    }

    QString currentBodyText() const
    {
        return m_currentBodyText;
    }

    void setCurrentBodyText(const QString& bodyText)
    {
        if (m_currentBodyText == bodyText)
        {
            return;
        }
        m_currentBodyText = bodyText;
        emit currentBodyTextChanged();
    }

    int itemCount() const noexcept
    {
        return m_itemCount;
    }

    void setItemCount(int count)
    {
        const int normalizedCount = std::max(0, count);
        if (m_itemCount == normalizedCount)
        {
            return;
        }
        m_itemCount = normalizedCount;
        emit itemCountChanged();
    }

    signals  :


    void currentNoteIdChanged();
    void currentBodyTextChanged();
    void itemCountChanged();

private:
    QString m_currentNoteId;
    QString m_currentBodyText;
    int m_itemCount = 0;
};

class FakeContentViewModel final : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text)
    {
        lastSavedNoteId = noteId;
        lastSavedText = text;
        saveBodyTextForNoteCallCount += 1;
        return saveBodyTextForNoteResult;
    }

    Q_INVOKABLE bool saveCurrentBodyText(const QString& text)
    {
        lastSavedText = text;
        saveCurrentBodyTextCallCount += 1;
        return saveCurrentBodyTextResult;
    }

    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId)
    {
        lastReloadedNoteId = noteId.trimmed();
        reloadNoteMetadataForNoteIdCallCount += 1;
        if (applyReloadedSelectionState && attachedNoteListModel != nullptr)
        {
            if (!nextReloadedNoteId.isNull())
            {
                attachedNoteListModel->setCurrentNoteId(nextReloadedNoteId);
            }
            if (!nextReloadedBodyText.isNull())
            {
                attachedNoteListModel->setCurrentBodyText(nextReloadedBodyText);
            }
            if (nextReloadedItemCount >= 0)
            {
                attachedNoteListModel->setItemCount(nextReloadedItemCount);
            }
        }
        return reloadNoteMetadataForNoteIdResult;
    }

    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const
    {
        return noteDirectoryPathByNoteId.value(noteId.trimmed());
    }

    void emitHubFilesystemMutated()
    {
        emit hubFilesystemMutated();
    }

    QString lastSavedNoteId;
    QString lastSavedText;
    QString lastReloadedNoteId;
    QHash<QString, QString> noteDirectoryPathByNoteId;
    FakeNoteListModel* attachedNoteListModel = nullptr;
    QString nextReloadedNoteId;
    QString nextReloadedBodyText;
    int nextReloadedItemCount = -1;
    bool applyReloadedSelectionState = false;
    int saveBodyTextForNoteCallCount = 0;
    int saveCurrentBodyTextCallCount = 0;
    int reloadNoteMetadataForNoteIdCallCount = 0;
    bool saveBodyTextForNoteResult = true;
    bool saveCurrentBodyTextResult = true;
    bool reloadNoteMetadataForNoteIdResult = true;

signals:
    void hubFilesystemMutated();
};

class ContentsEditorAdapterTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void textMetrics_mustTrackLogicalOffsetsAndLineQueries();
    void noteContracts_mustMirrorNoteListSelectionState();
    void noteContracts_refreshSelectedNoteSnapshot_mustReloadSelectedNoteFromContentViewModel();
    void gutterMarkers_mustNormalizeSupportedMarkerTypes();
    void persistence_mustDelegateToContentViewModel();
    void resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody();
    void resourceRenderer_mustRenderDirectResourcePackageSelection();
};

void ContentsEditorAdapterTest::textMetrics_mustTrackLogicalOffsetsAndLineQueries()
{
    ContentsLogicalTextBridge bridge;
    bridge.setText(QStringLiteral("alpha\nbeta\n"));

    const QVariantList offsets = bridge.logicalLineStartOffsets();
    QCOMPARE(offsets.size(), 3);
    QCOMPARE(offsets.at(0).toInt(), 0);
    QCOMPARE(offsets.at(1).toInt(), 6);
    QCOMPARE(offsets.at(2).toInt(), 11);
    QCOMPARE(bridge.logicalLineCount(), 3);
    QCOMPARE(bridge.logicalLineNumberForOffset(0), 1);
    QCOMPARE(bridge.logicalLineNumberForOffset(6), 2);
    QCOMPARE(bridge.logicalLineNumberForOffset(11), 3);
    QCOMPARE(bridge.logicalLineStartOffsetAt(1), 6);
    QCOMPARE(bridge.logicalLineCharacterCountAt(0), 5);
    QCOMPARE(bridge.logicalLineCharacterCountAt(1), 4);
    QCOMPARE(bridge.logicalLineCharacterCountAt(2), 0);
}

void ContentsEditorAdapterTest::noteContracts_mustMirrorNoteListSelectionState()
{
    FakeNoteListModel noteListModel;
    noteListModel.setCurrentNoteId(QStringLiteral("note-42"));
    noteListModel.setCurrentBodyText(QStringLiteral("body text"));
    noteListModel.setItemCount(7);

    ContentsEditorSelectionBridge bridge;
    bridge.setNoteListModel(&noteListModel);

    QVERIFY(bridge.noteSelectionContractAvailable());
    QVERIFY(bridge.noteCountContractAvailable());
    QCOMPARE(bridge.selectedNoteId(), QStringLiteral("note-42"));
    QCOMPARE(bridge.selectedNoteBodyText(), QStringLiteral("body text"));
    QCOMPARE(bridge.visibleNoteCount(), 7);

    noteListModel.setCurrentNoteId(QStringLiteral("note-77"));
    noteListModel.setCurrentBodyText(QStringLiteral("updated body"));
    noteListModel.setItemCount(2);

    QCOMPARE(bridge.selectedNoteId(), QStringLiteral("note-77"));
    QCOMPARE(bridge.selectedNoteBodyText(), QStringLiteral("updated body"));
    QCOMPARE(bridge.visibleNoteCount(), 2);
}

void ContentsEditorAdapterTest::noteContracts_refreshSelectedNoteSnapshot_mustReloadSelectedNoteFromContentViewModel()
{
    FakeNoteListModel noteListModel;
    noteListModel.setCurrentNoteId(QStringLiteral("note-42"));
    noteListModel.setCurrentBodyText(QStringLiteral("body text"));
    noteListModel.setItemCount(7);

    FakeContentViewModel contentViewModel;
    contentViewModel.attachedNoteListModel = &noteListModel;
    contentViewModel.applyReloadedSelectionState = true;
    contentViewModel.nextReloadedBodyText = QStringLiteral("reloaded body");
    contentViewModel.nextReloadedItemCount = 9;
    contentViewModel.reloadNoteMetadataForNoteIdResult = true;

    ContentsEditorSelectionBridge bridge;
    bridge.setNoteListModel(&noteListModel);
    bridge.setContentViewModel(&contentViewModel);

    QSignalSpy bodyTextChangedSpy(&bridge, &ContentsEditorSelectionBridge::selectedNoteBodyTextChanged);
    QSignalSpy visibleNoteCountChangedSpy(&bridge, &ContentsEditorSelectionBridge::visibleNoteCountChanged);

    QVERIFY(bridge.refreshSelectedNoteSnapshot());
    QCOMPARE(contentViewModel.reloadNoteMetadataForNoteIdCallCount, 1);
    QCOMPARE(contentViewModel.lastReloadedNoteId, QStringLiteral("note-42"));
    QCOMPARE(bridge.selectedNoteBodyText(), QStringLiteral("reloaded body"));
    QCOMPARE(bridge.visibleNoteCount(), 9);
    QVERIFY(bodyTextChangedSpy.count() >= 1);
    QVERIFY(visibleNoteCountChangedSpy.count() >= 1);
}

void ContentsEditorAdapterTest::gutterMarkers_mustNormalizeSupportedMarkerTypes()
{
    ContentsGutterMarkerBridge bridge;
    bridge.setGutterMarkers(QVariantList{
        QVariantMap{
            {QStringLiteral("line"), 2},
            {QStringLiteral("type"), QStringLiteral("Changed")}
        },
        QVariantMap{
            {QStringLiteral("startLine"), 5},
            {QStringLiteral("endLine"), 7},
            {QStringLiteral("type"), QStringLiteral("conflict")}
        },
        QVariantMap{
            {QStringLiteral("line"), 9},
            {QStringLiteral("type"), QStringLiteral("ignored")}
        }
    });

    const QVariantList normalized = bridge.normalizedExternalGutterMarkers();
    QCOMPARE(normalized.size(), 2);

    const QVariantMap firstMarker = normalized.at(0).toMap();
    QCOMPARE(firstMarker.value(QStringLiteral("startLine")).toInt(), 2);
    QCOMPARE(firstMarker.value(QStringLiteral("lineSpan")).toInt(), 1);
    QCOMPARE(firstMarker.value(QStringLiteral("type")).toString(), QStringLiteral("changed"));

    const QVariantMap secondMarker = normalized.at(1).toMap();
    QCOMPARE(secondMarker.value(QStringLiteral("startLine")).toInt(), 5);
    QCOMPARE(secondMarker.value(QStringLiteral("lineSpan")).toInt(), 3);
    QCOMPARE(secondMarker.value(QStringLiteral("type")).toString(), QStringLiteral("conflict"));
}

void ContentsEditorAdapterTest::persistence_mustDelegateToContentViewModel()
{
    FakeContentViewModel contentViewModel;
    ContentsEditorSelectionBridge bridge;
    bridge.setContentViewModel(&contentViewModel);

    QVERIFY(bridge.contentPersistenceContractAvailable());
    QVERIFY(bridge.persistEditorTextForNote(QStringLiteral("note-a"), QStringLiteral("text-a")));
    QCOMPARE(contentViewModel.lastSavedNoteId, QStringLiteral("note-a"));
    QCOMPARE(contentViewModel.lastSavedText, QStringLiteral("text-a"));
    QCOMPARE(contentViewModel.saveBodyTextForNoteCallCount, 1);
    QCOMPARE(contentViewModel.saveCurrentBodyTextCallCount, 0);
}

void ContentsEditorAdapterTest::resourceRenderer_mustResolveResourceTagsFromCurrentNoteBody()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("RenderHub.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("RenderHub.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("Render.wsnote"));
    const QString resourcesPath = QDir(hubPath).filePath(QStringLiteral("RenderHub.wsresources"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("preview"),
        QStringLiteral("preview.png"),
        QStringLiteral("png")));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("voice"),
        QStringLiteral("voice.mp3"),
        QStringLiteral("mp3")));

    const QString bodyDocumentText =
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=\"note-render\">\n"
            "  <body>\n"
            "    <resource type=\"image\" format=\".png\" resourcePath=\"RenderHub.wsresources/preview.wsresource\" />\n"
            "    <paragraph>Body line</paragraph>\n"
            "    <resource type=audio format=.mp3 path=RenderHub.wsresources/voice.wsresource>\n"
            "  </body>\n"
            "</contents>\n");
    QVERIFY(writeUtf8File(QDir(noteDirectoryPath).filePath(QStringLiteral("Render.wsnbody")), bodyDocumentText));

    FakeContentViewModel contentViewModel;
    contentViewModel.noteDirectoryPathByNoteId.insert(QStringLiteral("note-render"), noteDirectoryPath);

    ContentsBodyResourceRenderer renderer;
    renderer.setContentViewModel(&contentViewModel);
    renderer.setNoteId(QStringLiteral("note-render"));

    const QVariantList renderedResources = renderer.renderedResources();
    QCOMPARE(renderer.resourceCount(), 2);
    QVERIFY(renderer.hasRenderableResource());
    QCOMPARE(renderedResources.size(), 2);

    const QVariantMap firstResource = renderedResources.at(0).toMap();
    QCOMPARE(firstResource.value(QStringLiteral("renderMode")).toString(), QStringLiteral("image"));
    QCOMPARE(firstResource.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(firstResource.value(QStringLiteral("format")).toString(), QStringLiteral(".png"));
    QCOMPARE(
        firstResource.value(QStringLiteral("source")).toString(),
        QUrl::fromLocalFile(
            QDir(resourcesPath).filePath(QStringLiteral("preview.wsresource/preview.png"))).toString());

    const QVariantMap secondResource = renderedResources.at(1).toMap();
    QCOMPARE(secondResource.value(QStringLiteral("renderMode")).toString(), QStringLiteral("audio"));
    QCOMPARE(secondResource.value(QStringLiteral("type")).toString(), QStringLiteral("audio"));
    QCOMPARE(secondResource.value(QStringLiteral("format")).toString(), QStringLiteral(".mp3"));

    renderer.setMaxRenderCount(1);
    QCOMPARE(renderer.resourceCount(), 1);

    renderer.setMaxRenderCount(3);
    renderer.setNoteId(QString());
    QCOMPARE(renderer.resourceCount(), 0);
    QVERIFY(!renderer.hasRenderableResource());
}

void ContentsEditorAdapterTest::resourceRenderer_mustRenderDirectResourcePackageSelection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString resourcesPath = QDir(tempDir.path()).filePath(QStringLiteral("DirectRender.wsresources"));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("manual"),
        QStringLiteral("manual.pdf"),
        QStringLiteral("pdf")));
    const QString packageDirectoryPath = QDir(resourcesPath).filePath(QStringLiteral("manual.wsresource"));

    FakeContentViewModel contentViewModel;
    contentViewModel.noteDirectoryPathByNoteId.insert(QStringLiteral("resource-manual"), packageDirectoryPath);

    ContentsBodyResourceRenderer renderer;
    renderer.setContentViewModel(&contentViewModel);
    renderer.setNoteId(QStringLiteral("resource-manual"));

    const QVariantList renderedResources = renderer.renderedResources();
    QCOMPARE(renderer.resourceCount(), 1);
    QVERIFY(renderer.hasRenderableResource());
    QCOMPARE(renderedResources.size(), 1);

    const QVariantMap resource = renderedResources.at(0).toMap();
    QCOMPARE(resource.value(QStringLiteral("renderMode")).toString(), QStringLiteral("pdf"));
    QCOMPARE(resource.value(QStringLiteral("type")).toString(), QStringLiteral("document"));
    QCOMPARE(resource.value(QStringLiteral("format")).toString(), QStringLiteral(".pdf"));
    QCOMPARE(
        resource.value(QStringLiteral("resourcePath")).toString(),
        QStringLiteral("DirectRender.wsresources/manual.wsresource"));
    QCOMPARE(
        resource.value(QStringLiteral("source")).toString(),
        QUrl::fromLocalFile(
            QDir(resourcesPath).filePath(QStringLiteral("manual.wsresource/manual.pdf"))).toString());
}

QTEST_MAIN(ContentsEditorAdapterTest)

#include "test_contents_editor_bridge.moc"
