#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.hpp"
#include "calendar/SystemCalendarStore.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>
#include <stdexcept>

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

    QString readUtf8File(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
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

    QString makeWsnHeadText(
        const QString& noteId,
        bool bookmarked,
        const QStringList& bookmarkColors,
        const QStringList& tags = {},
        int progressValue = 0,
        const QString& projectName = QStringLiteral("General"))
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"%1\">\n").arg(noteId);
        text += QStringLiteral("  <head>\n");
        text += QStringLiteral("    <created>2026-03-01-00-00-00</created>\n");
        text += QStringLiteral("    <author>Tester</author>\n");
        text += QStringLiteral("    <lastModified>2026-03-01-00-00-00</lastModified>\n");
        text += QStringLiteral("    <modifiedBy>Tester</modifiedBy>\n");
        text += QStringLiteral("    <folders></folders>\n");
        text += QStringLiteral("    <project>%1</project>\n").arg(projectName);
        QString bookmarkTag = QStringLiteral("    <bookmarks state=\"%1\"").arg(
            bookmarked ? QStringLiteral("true") : QStringLiteral("false"));
        if (!bookmarkColors.isEmpty())
        {
            bookmarkTag += QStringLiteral(" colors=\"{%1}\"").arg(bookmarkColors.join(QStringLiteral(",")));
        }
        bookmarkTag += QStringLiteral(" />\n");
        text += bookmarkTag;
        text += QStringLiteral("    <tags>\n");
        for (const QString& tag : tags)
        {
            text += QStringLiteral("      <tag>%1</tag>\n").arg(tag);
        }
        text += QStringLiteral("    </tags>\n");
        text += QStringLiteral("    <progress enums=\"{Ready,Pending,InProgress,Done}\">%1</progress>\n").arg(
            progressValue);
        text += QStringLiteral("    <isPreset>false</isPreset>\n");
        text += QStringLiteral("  </head>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QString makeProgressWsprogressText(const QStringList& states, int progressValue = 0)
    {
        QJsonArray stateArray;
        for (const QString& state : states)
        {
            stateArray.push_back(state);
        }

        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QStringLiteral("whatson.progress.state"));
        root.insert(QStringLiteral("progress"), progressValue);
        root.insert(QStringLiteral("states"), stateArray);

        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    QString makeProjectsWsprojText(const QJsonArray& projects)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QStringLiteral("whatson.projects.list"));
        root.insert(QStringLiteral("projects"), projects);
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    QString makeWsnBodyText(const QString& summaryText)
    {
        return QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"test-note\">\n"
                "  <body>\n"
                "    <paragraph>%1</paragraph>\n"
                "  </body>\n"
                "</contents>\n")
            .arg(summaryText);
    }

    QString makeWsnBodyTextFromInnerXml(const QString& bodyInnerXml)
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"test-note\">\n");
        text += QStringLiteral("  <body>\n");
        text += bodyInnerXml;
        if (!bodyInnerXml.endsWith(QLatin1Char('\n')))
        {
            text += QLatin1Char('\n');
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    bool prepareBookmarksHub(QString* outHubPath)
    {
        if (outHubPath == nullptr)
        {
            return false;
        }

        static QTemporaryDir tempDir;
        if (!tempDir.isValid())
        {
            return false;
        }

        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("BookmarksVmHub.wshub"));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("BookmarksVmHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString resourcesPath = QDir(hubPath).filePath(QStringLiteral("BookmarksVmHub.wsresources"));
        const QString noteAPath = QDir(libraryPath).filePath(QStringLiteral("Blue.wsnote"));
        const QString noteBPath = QDir(libraryPath).filePath(QStringLiteral("Hidden.wsnote"));
        const QString noteCPath = QDir(libraryPath).filePath(QStringLiteral("Pink.wsnote"));
        if (!QDir().mkpath(noteAPath)
            || !QDir().mkpath(noteBPath)
            || !QDir().mkpath(noteCPath)
            || !QDir().mkpath(resourcesPath))
        {
            return false;
        }
        if (!createResourcePackage(
            resourcesPath,
            QStringLiteral("blue-preview"),
            QStringLiteral("blue-preview.png"),
            QStringLiteral("png")))
        {
            return false;
        }

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-blue\"},\n"
            "    {\"id\": \"note-hidden\"},\n"
            "    {\"id\": \"note-pink\"}\n"
            "  ]\n"
            "}\n");
        if (!writeUtf8File(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")), indexText))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteAPath).filePath(QStringLiteral("Blue.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-blue"),
                true,
                {QStringLiteral("blue")},
                {QStringLiteral("brand"), QStringLiteral("campaign")})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteAPath).filePath(QStringLiteral("Blue.wsnbody")),
            makeWsnBodyTextFromInnerXml(
                QStringLiteral(
                    "    <resource type=\"image\" format=\".png\" resourcePath=\"BookmarksVmHub.wsresources/blue-preview.wsresource\" />\n"
                    "    <paragraph>Blue summary</paragraph>\n"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteBPath).filePath(QStringLiteral("Hidden.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-hidden"),
                false,
                {QStringLiteral("red")})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteBPath).filePath(QStringLiteral("Hidden.wsnbody")),
            makeWsnBodyText(QStringLiteral("Hidden summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteCPath).filePath(QStringLiteral("Pink.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-pink"),
                true,
                {QStringLiteral("#EC4899")},
                {QStringLiteral("release")})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteCPath).filePath(QStringLiteral("Pink.wsnbody")),
            makeWsnBodyText(QStringLiteral("Pink summary"))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }

    bool prepareProgressHub(QString* outHubPath)
    {
        if (outHubPath == nullptr)
        {
            return false;
        }

        static QTemporaryDir tempDir;
        if (!tempDir.isValid())
        {
            return false;
        }

        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ProgressVmHub.wshub"));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProgressVmHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString readyNotePath = QDir(libraryPath).filePath(QStringLiteral("Ready.wsnote"));
        const QString pendingNotePath = QDir(libraryPath).filePath(QStringLiteral("Pending.wsnote"));
        const QString doneNotePath = QDir(libraryPath).filePath(QStringLiteral("Done.wsnote"));
        if (!QDir().mkpath(readyNotePath)
            || !QDir().mkpath(pendingNotePath)
            || !QDir().mkpath(doneNotePath))
        {
            return false;
        }

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-ready\"},\n"
            "    {\"id\": \"note-pending\"},\n"
            "    {\"id\": \"note-done\"}\n"
            "  ]\n"
            "}\n");
        if (!writeUtf8File(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")), indexText))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Progress.wsprogress")),
            makeProgressWsprogressText({
                QStringLiteral("Ready"),
                QStringLiteral("Pending"),
                QStringLiteral("InProgress"),
                QStringLiteral("Done")
            })))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(readyNotePath).filePath(QStringLiteral("Ready.wsnhead")),
            makeWsnHeadText(QStringLiteral("note-ready"), false, {}, {}, 0))
            || !writeUtf8File(
                QDir(readyNotePath).filePath(QStringLiteral("Ready.wsnbody")),
                makeWsnBodyText(QStringLiteral("Ready summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(pendingNotePath).filePath(QStringLiteral("Pending.wsnhead")),
            makeWsnHeadText(QStringLiteral("note-pending"), false, {}, {}, 3))
            || !writeUtf8File(
                QDir(pendingNotePath).filePath(QStringLiteral("Pending.wsnbody")),
                makeWsnBodyText(QStringLiteral("Pending summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(doneNotePath).filePath(QStringLiteral("Done.wsnhead")),
            makeWsnHeadText(QStringLiteral("note-done"), false, {}, {}, 6))
            || !writeUtf8File(
                QDir(doneNotePath).filePath(QStringLiteral("Done.wsnbody")),
                makeWsnBodyText(QStringLiteral("Done summary"))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }

    bool prepareProjectsHub(QString* outHubPath)
    {
        if (outHubPath == nullptr)
        {
            return false;
        }

        static QTemporaryDir tempDir;
        if (!tempDir.isValid())
        {
            return false;
        }

        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("ProjectsVmHub.wshub"));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProjectsVmHub.wscontents"));
        const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
        const QString alphaAPath = QDir(libraryPath).filePath(QStringLiteral("AlphaA.wsnote"));
        const QString alphaBPath = QDir(libraryPath).filePath(QStringLiteral("AlphaB.wsnote"));
        const QString betaPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
        const QString orphanPath = QDir(libraryPath).filePath(QStringLiteral("Orphan.wsnote"));
        if (!QDir().mkpath(alphaAPath)
            || !QDir().mkpath(alphaBPath)
            || !QDir().mkpath(betaPath)
            || !QDir().mkpath(orphanPath))
        {
            return false;
        }

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-alpha-a\"},\n"
            "    {\"id\": \"note-alpha-b\"},\n"
            "    {\"id\": \"note-beta\"},\n"
            "    {\"id\": \"note-orphan\"}\n"
            "  ]\n"
            "}\n");
        if (!writeUtf8File(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")), indexText))
        {
            return false;
        }

        const QJsonArray projects = {
            QJsonObject{{QStringLiteral("id"), QStringLiteral("Alpha")}, {QStringLiteral("label"), QStringLiteral("Alpha")}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("Beta")}, {QStringLiteral("label"), QStringLiteral("Beta")}}
        };
        if (!writeUtf8File(
                QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj")),
                makeProjectsWsprojText(projects)))
        {
            return false;
        }

        if (!writeUtf8File(
                QDir(alphaAPath).filePath(QStringLiteral("AlphaA.wsnhead")),
                makeWsnHeadText(QStringLiteral("note-alpha-a"), false, {}, {}, 0, QStringLiteral("Alpha")))
            || !writeUtf8File(
                QDir(alphaAPath).filePath(QStringLiteral("AlphaA.wsnbody")),
                makeWsnBodyText(QStringLiteral("Alpha A summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
                QDir(alphaBPath).filePath(QStringLiteral("AlphaB.wsnhead")),
                makeWsnHeadText(QStringLiteral("note-alpha-b"), false, {}, {}, 0, QStringLiteral("Alpha")))
            || !writeUtf8File(
                QDir(alphaBPath).filePath(QStringLiteral("AlphaB.wsnbody")),
                makeWsnBodyText(QStringLiteral("Alpha B summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
                QDir(betaPath).filePath(QStringLiteral("Beta.wsnhead")),
                makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QStringLiteral("Beta")))
            || !writeUtf8File(
                QDir(betaPath).filePath(QStringLiteral("Beta.wsnbody")),
                makeWsnBodyText(QStringLiteral("Beta summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
                QDir(orphanPath).filePath(QStringLiteral("Orphan.wsnhead")),
                makeWsnHeadText(QStringLiteral("note-orphan"), false, {}, {}, 0, QStringLiteral("Missing")))
            || !writeUtf8File(
                QDir(orphanPath).filePath(QStringLiteral("Orphan.wsnbody")),
                makeWsnBodyText(QStringLiteral("Orphan summary"))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }
} // namespace

class HierarchyViewModelsTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void libraryViewModel_supportsCrudContract();
    void libraryViewModel_setItemExpanded_emitsHierarchyNodesChanged();
    void projectsViewModel_supportsCrudContract();
    void projectsViewModel_loadFromWshub_populatesNoteListForSelectedProject();
    void projectsViewModel_reloadNoteMetadataForNoteId_refreshesSelectedProjectProjection();
    void projectsViewModel_reloadNoteMetadataForNoteId_whenHeaderMissing_mustDropStaleProjection();
    void projectsViewModel_loadFromWshub_ignoresStaleIndexProjectWhenHeaderProjectIsEmpty();
    void projectsViewModel_loadFromWshub_ignoresIndexOnlyProjectRecordWithoutHeader();
    void projectsViewModel_selectionRefresh_mustDropNoteWhenHeaderProjectBecomesEmpty();
    void projectsViewModel_reactsToModelMutation();
    void projectsViewModel_applyRuntimeSnapshot_preservesSelectionAcrossUnchangedSnapshot();
    void projectsViewModel_applyRuntimeSnapshot_whenHierarchyUnchanged_refreshesSelectedProjectNotes();
    void projectsViewModel_requestViewModelHook_refreshesSelectedProjectProjection();
    void projectsViewModel_moveFolderBefore_persistsFoldersFileAndDepth();
    void projectsViewModel_applyHierarchyNodes_persistsLvrsEditableMove();
    void bookmarksViewModel_supportsCrudContract();
    void bookmarksViewModel_loadFromWshub_filtersBookmarkedNotesAndMapsHexColor();
    void bookmarksViewModel_searchText_filtersVisibleNotesByBodyContent();
    void bookmarksViewModel_saveCurrentBodyText_rewritesWsnbody();
    void bookmarksViewModel_removeNoteById_reselectsVisibleNeighbor();
    void bookmarksViewModel_formatsDisplayDateWithSystemCalendarStore();
    void resourcesViewModel_exposesSupportedTypeTree();
    void resourcesViewModel_applyRuntimeSnapshot_preservesExpandedBucketState();
    void progressViewModel_supportsCrudContract();
    void progressViewModel_applyRuntimeSnapshot_preservesSelectionAndVisibleNotes();
    void eventViewModel_supportsCrudContract();
    void eventViewModel_applyRuntimeSnapshot_preservesExpandedBucketState();
    void presetViewModel_supportsCrudContract();
    void presetViewModel_applyRuntimeSnapshot_preservesExpandedBucketState();
    void tagsViewModel_supportsCrudContract();
    void hierarchyViewModels_exposeCapabilityInterfaces();
    void hierarchyViewModels_hierarchyModel_exposesCountRoleAcrossDomains();
    void tagsViewModel_applyRuntimeSnapshot_preservesExpandedStateAcrossHierarchyRefresh();
    void libraryViewModel_reactsToNoteListModelMutation();
    void projectsModel_appliesCorrectionAndRaisesHookSignal();
    void projectsModel_recomputesChevronByDepth();
    void projectsModel_strictValidation_throwsException();
    void noteListModel_correctsColorAndTextFields();
    void noteListModel_sortsNewestModifiedItemFirst();
    void noteListModel_currentSelection_exposesBodyText();
    void noteListModel_searchText_filtersAgainstSearchableBodyContent();
    void noteListModel_searchText_mustIgnoreInternalIdFallback();
    void noteListModel_limitsPrimaryTextToFiveLines();
    void noteListModel_exposesImageRoles();
    void libraryViewModel_formatsDisplayDateWithSystemCalendarStore();
};

void HierarchyViewModelsTest::libraryViewModel_supportsCrudContract()
{
    LibraryHierarchyViewModel viewModel;
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QVERIFY(!viewModel.deleteFolderEnabled());

    viewModel.setDepthItems(QVariantList{
        QVariantMap{{"label", QStringLiteral("Root")}, {"depth", 0}}
    });
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("RenamedRoot")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("RenamedRoot"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ExpandedRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);

    viewModel.setDepthItems(QVariantList{
        QVariantMap{{"label", QStringLiteral("Library (1)")}, {"depth", 0}, {"accent", true}},
        QVariantMap{{"label", QStringLiteral("Leaf")}, {"depth", 1}, {"accent", false}}
    });
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("BlockedHeaderRename")));
}

void HierarchyViewModelsTest::libraryViewModel_setItemExpanded_emitsHierarchyNodesChanged()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{{"label", QStringLiteral("Root")}, {"depth", 0}},
        QVariantMap{{"label", QStringLiteral("Child")}, {"depth", 1}}
    });

    const QVariantList initialNodes = viewModel.hierarchyNodes();
    int expandableIndex = -1;
    for (int index = 0; index < initialNodes.size(); ++index)
    {
        const QVariantMap node = initialNodes.at(index).toMap();
        if (node.value(QStringLiteral("showChevron")).toBool()
            && !node.value(QStringLiteral("accent")).toBool())
        {
            expandableIndex = index;
            break;
        }
    }
    QVERIFY(expandableIndex >= 0);

    QSignalSpy hierarchyNodesSpy(&viewModel, SIGNAL(hierarchyNodesChanged()));
    QVERIFY(hierarchyNodesSpy.isValid());

    QVERIFY(viewModel.setItemExpanded(expandableIndex, true));
    QCOMPARE(hierarchyNodesSpy.count(), 1);
    QCOMPARE(
        viewModel.hierarchyNodes().at(expandableIndex).toMap().value(QStringLiteral("expanded")).toBool(),
        true);
}

void HierarchyViewModelsTest::projectsViewModel_supportsCrudContract()
{
    ProjectsHierarchyViewModel viewModel;
    viewModel.setProjectNames({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QVERIFY(viewModel.noteListModel() != nullptr);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProjectsHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("customFolder"));
    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    QCOMPARE(hierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("Alpha"));
    QCOMPARE(hierarchyModel.at(0).toMap().value(QStringLiteral("iconName")).toString(), QStringLiteral("customFolder"));
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Alpha-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Alpha-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), ProjectsHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::projectsViewModel_loadFromWshub_populatesNoteListForSelectedProject()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));
    const QString betaNoteDirectoryPath = QDir(hubPath).filePath(
        QStringLiteral("ProjectsVmHub.wscontents/Library.wslibrary/Beta.wsnote"));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(viewModel.noteListModel() != nullptr);

    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-alpha-a"));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(1, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-alpha-b"));

    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-beta"));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::PrimaryTextRole).toString(),
        QStringLiteral("Beta summary"));
    QCOMPARE(
        QDir::cleanPath(viewModel.noteDirectoryPathForNoteId(QStringLiteral("note-beta"))),
        QDir::cleanPath(betaNoteDirectoryPath));
}

void HierarchyViewModelsTest::projectsViewModel_reloadNoteMetadataForNoteId_refreshesSelectedProjectProjection()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const auto countByLabel = [&viewModel](const QString& label) -> int
    {
        const QVariantList hierarchyModel = viewModel.hierarchyModel();
        for (const QVariant& entry : hierarchyModel)
        {
            const QVariantMap node = entry.toMap();
            if (node.value(QStringLiteral("label")).toString() != label)
            {
                continue;
            }

            bool ok = false;
            const int value = node.value(QStringLiteral("count")).toInt(&ok);
            if (!ok)
            {
                return -1;
            }
            return value;
        }
        return -1;
    };

    QCOMPARE(countByLabel(QStringLiteral("Alpha")), 2);
    QCOMPARE(countByLabel(QStringLiteral("Beta")), 1);

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    const QString betaHeaderPath = QDir(hubPath).filePath(
        QStringLiteral("ProjectsVmHub.wscontents/Library.wslibrary/Beta.wsnote/Beta.wsnhead"));
    QVERIFY2(
        writeUtf8File(
            betaHeaderPath,
            makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QStringLiteral("Alpha"))),
        "Beta.wsnhead must be rewritten");

    QVERIFY(viewModel.reloadNoteMetadataForNoteId(QStringLiteral("note-beta")));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(2, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-beta"));
    QCOMPARE(countByLabel(QStringLiteral("Alpha")), 3);
    QCOMPARE(countByLabel(QStringLiteral("Beta")), 0);
}

void HierarchyViewModelsTest::projectsViewModel_reloadNoteMetadataForNoteId_whenHeaderMissing_mustDropStaleProjection()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);

    const QString betaHeaderPath = QDir(hubPath).filePath(
        QStringLiteral("ProjectsVmHub.wscontents/Library.wslibrary/Beta.wsnote/Beta.wsnhead"));
    QVERIFY(QFileInfo(betaHeaderPath).isFile());
    QVERIFY(QFile::remove(betaHeaderPath));

    QVERIFY(!viewModel.reloadNoteMetadataForNoteId(QStringLiteral("note-beta")));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
}

void HierarchyViewModelsTest::projectsViewModel_loadFromWshub_ignoresStaleIndexProjectWhenHeaderProjectIsEmpty()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProjectsVmHub.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString projectsFilePath = QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj"));
    const QString betaHeaderPath = QDir(libraryPath).filePath(
        QStringLiteral("Beta.wsnote/Beta.wsnhead"));
    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    const QJsonArray projects = {
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Alpha")}, {QStringLiteral("label"), QStringLiteral("Alpha")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Beta")}, {QStringLiteral("label"), QStringLiteral("Beta")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Untitled")}, {QStringLiteral("label"), QStringLiteral("Untitled")}}
    };
    QVERIFY(writeUtf8File(projectsFilePath, makeProjectsWsprojText(projects)));

    const QString staleIndexText = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.library.index\",\n"
        "  \"notes\": [\n"
        "    {\"id\": \"note-alpha-a\"},\n"
        "    {\"id\": \"note-alpha-b\"},\n"
        "    {\"id\": \"note-beta\", \"project\": \"Untitled\"},\n"
        "    {\"id\": \"note-orphan\"}\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(indexPath, staleIndexText));
    QVERIFY(writeUtf8File(betaHeaderPath, makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QString())));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));

    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
}

void HierarchyViewModelsTest::projectsViewModel_loadFromWshub_ignoresIndexOnlyProjectRecordWithoutHeader()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProjectsVmHub.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString projectsFilePath = QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj"));
    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    const QJsonArray projects = {
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Alpha")}, {QStringLiteral("label"), QStringLiteral("Alpha")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Beta")}, {QStringLiteral("label"), QStringLiteral("Beta")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Untitled")}, {QStringLiteral("label"), QStringLiteral("Untitled")}}
    };
    QVERIFY(writeUtf8File(projectsFilePath, makeProjectsWsprojText(projects)));

    const QString staleIndexText = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.library.index\",\n"
        "  \"notes\": [\n"
        "    {\"id\": \"note-alpha-a\"},\n"
        "    {\"id\": \"note-alpha-b\"},\n"
        "    {\"id\": \"note-beta\"},\n"
        "    {\"id\": \"note-orphan\"},\n"
        "    {\"id\": \"index-only-ghost\", \"project\": \"Untitled\"}\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(indexPath, staleIndexText));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));

    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
}

void HierarchyViewModelsTest::projectsViewModel_selectionRefresh_mustDropNoteWhenHeaderProjectBecomesEmpty()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProjectsVmHub.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString projectsFilePath = QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj"));
    const QString betaHeaderPath = QDir(libraryPath).filePath(
        QStringLiteral("Beta.wsnote/Beta.wsnhead"));
    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    const QJsonArray projects = {
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Alpha")}, {QStringLiteral("label"), QStringLiteral("Alpha")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Beta")}, {QStringLiteral("label"), QStringLiteral("Beta")}},
        QJsonObject{{QStringLiteral("id"), QStringLiteral("Untitled")}, {QStringLiteral("label"), QStringLiteral("Untitled")}}
    };
    QVERIFY(writeUtf8File(projectsFilePath, makeProjectsWsprojText(projects)));

    const QString staleIndexText = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.library.index\",\n"
        "  \"notes\": [\n"
        "    {\"id\": \"note-alpha-a\"},\n"
        "    {\"id\": \"note-alpha-b\"},\n"
        "    {\"id\": \"note-beta\", \"project\": \"Untitled\"},\n"
        "    {\"id\": \"note-orphan\"}\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(indexPath, staleIndexText));
    QVERIFY(writeUtf8File(betaHeaderPath, makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QStringLiteral("Untitled"))));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-beta"));

    QVERIFY(writeUtf8File(betaHeaderPath, makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QString())));

    viewModel.setSelectedIndex(0);
    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
}

void HierarchyViewModelsTest::projectsViewModel_reactsToModelMutation()
{
    ProjectsHierarchyViewModel viewModel;
    viewModel.setProjectNames({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.itemCount(), 2);

    QSignalSpy itemCountSpy(&viewModel, &ProjectsHierarchyViewModel::itemCountChanged);
    QVector<ProjectsHierarchyItem> externalItems;
    ProjectsHierarchyItem root;
    root.depth = 0;
    root.accent = true;
    root.expanded = true;
    root.label = QStringLiteral("Projects (0)");
    root.showChevron = false;
    externalItems.push_back(root);

    viewModel.itemModel()->setItems(externalItems);

    QCOMPARE(viewModel.itemCount(), 1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QVERIFY(itemCountSpy.count() > 0);
}

void HierarchyViewModelsTest::projectsViewModel_applyRuntimeSnapshot_preservesSelectionAcrossUnchangedSnapshot()
{
    ProjectsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        QStringLiteral("/tmp/ProjectLists.wsproj"),
        true);

    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.selectedIndex(), 1);

    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        QStringLiteral("/tmp/ProjectLists.wsproj"),
        true);

    QCOMPARE(viewModel.selectedIndex(), 1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
}

void HierarchyViewModelsTest::projectsViewModel_applyRuntimeSnapshot_whenHierarchyUnchanged_refreshesSelectedProjectNotes()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("ProjectsVmHub.wscontents"));
    const QString projectsFilePath = QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj"));
    const QString betaHeaderPath = QDir(contentsPath).filePath(
        QStringLiteral("Library.wslibrary/Beta.wsnote/Beta.wsnhead"));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    QVERIFY(writeUtf8File(
        betaHeaderPath,
        makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QStringLiteral("Alpha"))));

    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Alpha"), QStringLiteral("Alpha"), 0},
            {QStringLiteral("Beta"), QStringLiteral("Beta"), 0}
        },
        projectsFilePath,
        true);

    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);

    bool containsBeta = false;
    for (int row = 0; row < viewModel.noteListModel()->rowCount(); ++row)
    {
        if (viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(row, 0),
            LibraryNoteListModel::NoteIdRole).toString() == QStringLiteral("note-beta"))
        {
            containsBeta = true;
            break;
        }
    }
    QVERIFY(containsBeta);
}

void HierarchyViewModelsTest::projectsViewModel_requestViewModelHook_refreshesSelectedProjectProjection()
{
    QString hubPath;
    QVERIFY(prepareProjectsHub(&hubPath));

    const QString betaHeaderPath = QDir(hubPath).filePath(
        QStringLiteral("ProjectsVmHub.wscontents/Library.wslibrary/Beta.wsnote/Beta.wsnhead"));

    ProjectsHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);

    QVERIFY(writeUtf8File(
        betaHeaderPath,
        makeWsnHeadText(QStringLiteral("note-beta"), false, {}, {}, 0, QString())));

    QSignalSpy hookSpy(&viewModel, &ProjectsHierarchyViewModel::viewModelHookRequested);
    QVERIFY(hookSpy.isValid());

    viewModel.requestViewModelHook();

    QCOMPARE(hookSpy.count(), 1);
    QVERIFY(viewModel.loadSucceeded());
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
}

void HierarchyViewModelsTest::projectsViewModel_moveFolderBefore_persistsFoldersFileAndDepth()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString projectsFilePath = tempDir.filePath(QStringLiteral("ProjectLists.wsproj"));

    ProjectsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Research/Competitor"), QStringLiteral("Competitor"), 1},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        projectsFilePath,
        true);

    QVERIFY(viewModel.canAcceptFolderDropBefore(2, 0));
    QVERIFY(viewModel.moveFolderBefore(2, 0));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProjectsHierarchyModel::DepthRole).toInt(),
        0);

    QVERIFY(!viewModel.canAcceptFolderDrop(0, 1, true));
    QVERIFY(!viewModel.moveFolder(0, 1, true));

    const QJsonDocument projectsDocument = QJsonDocument::fromJson(readUtf8File(projectsFilePath).toUtf8());
    QVERIFY(projectsDocument.isObject());
    const QJsonArray projectArray = projectsDocument.object().value(QStringLiteral("projects")).toArray();
    QCOMPARE(projectArray.size(), 3);
    QCOMPARE(projectArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));
    QCOMPARE(projectArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    QCOMPARE(projectArray.at(2).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Competitor"));
}

void HierarchyViewModelsTest::projectsViewModel_applyHierarchyNodes_persistsLvrsEditableMove()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString projectsFilePath = tempDir.filePath(QStringLiteral("ProjectLists.wsproj"));

    ProjectsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Research/Competitor"), QStringLiteral("Competitor"), 1},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        projectsFilePath,
        true);

    QVariantList hierarchyNodes = viewModel.depthItems();
    QCOMPARE(hierarchyNodes.size(), 3);
    for (int index = 0; index < hierarchyNodes.size(); ++index)
    {
        QVariantMap entry = hierarchyNodes.at(index).toMap();
        entry.insert(QStringLiteral("sourceIndex"), index);
        hierarchyNodes[index] = entry;
    }

    QVariantMap researchNode = hierarchyNodes.at(0).toMap();
    QVariantMap competitorNode = hierarchyNodes.at(1).toMap();
    QVariantMap brandNode = hierarchyNodes.at(2).toMap();
    brandNode.insert(QStringLiteral("depth"), 1);

    QVariantList reorderedNodes;
    reorderedNodes << researchNode << competitorNode << brandNode;

    const QString activeItemKey = brandNode.value(QStringLiteral("key")).toString();
    QVERIFY(!activeItemKey.isEmpty());
    QVERIFY(viewModel.applyHierarchyNodes(reorderedNodes, activeItemKey));

    QCOMPARE(viewModel.selectedIndex(), 2);
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProjectsHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProjectsHierarchyModel::DepthRole).toInt(),
        0);

    const QJsonDocument projectsDocument = QJsonDocument::fromJson(readUtf8File(projectsFilePath).toUtf8());
    QVERIFY(projectsDocument.isObject());
    const QJsonArray projectArray = projectsDocument.object().value(QStringLiteral("projects")).toArray();
    QCOMPARE(projectArray.size(), 3);
    QCOMPARE(projectArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    QCOMPARE(projectArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Competitor"));
    QCOMPARE(projectArray.at(2).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));
}

void HierarchyViewModelsTest::bookmarksViewModel_supportsCrudContract()
{
    BookmarksHierarchyViewModel viewModel;
    QVERIFY(!viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QVERIFY(!viewModel.viewOptionsEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), BookmarksHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("bookmarksbookmark"));
    const QString redIconSource =
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), BookmarksHierarchyModel::IconSourceRole)
            .toString();
    QVERIFY(!redIconSource.isEmpty());
    QVERIFY(redIconSource.contains(QStringLiteral("%23EF4444")));
    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    QVERIFY(!hierarchyModel.isEmpty());
    QCOMPARE(
        hierarchyModel.at(0).toMap().value(QStringLiteral("iconName")).toString(),
        QStringLiteral("bookmarksbookmark"));
    QVERIFY(hierarchyModel.at(0).toMap().contains(QStringLiteral("iconSource")));
    QVERIFY(
        hierarchyModel.at(0).toMap().value(QStringLiteral("iconSource")).toString().contains(
            QStringLiteral("%23EF4444")));
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Bookmarks-Header")));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
}

void HierarchyViewModelsTest::bookmarksViewModel_loadFromWshub_filtersBookmarkedNotesAndMapsHexColor()
{
    QString hubPath;
    QVERIFY(prepareBookmarksHub(&hubPath));

    BookmarksHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(qobject_cast<BookmarksNoteListModel*>(viewModel.noteListModel()) != nullptr);

    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("Red"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("Blue"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(7, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("Indigo"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(9, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("Pink"));
    QVERIFY(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), BookmarksHierarchyModel::IconSourceRole)
            .toString()
            .contains(QStringLiteral("%233B82F6")));
    QVERIFY(
        viewModel.itemModel()->data(viewModel.itemModel()->index(9, 0), BookmarksHierarchyModel::IconSourceRole)
            .toString()
            .contains(QStringLiteral("%23EC4899")));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    QCOMPARE(viewModel.noteListModel()->roleNames().value(BookmarksNoteListModel::NoteIdRole), QByteArray("noteId"));
    QCOMPARE(viewModel.noteListModel()->currentIndex(), -1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QString());
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QString());

    QStringList primaryTextAndColorPairs;
    QStringList noteIds;
    for (int row = 0; row < viewModel.noteListModel()->rowCount(); ++row)
    {
        const QModelIndex index = viewModel.noteListModel()->index(row, 0);
        QVERIFY(viewModel.noteListModel()->data(index, BookmarksNoteListModel::BookmarkedRole).toBool());
        noteIds.push_back(viewModel.noteListModel()->data(index, BookmarksNoteListModel::NoteIdRole).toString());
        const QString primaryText = viewModel.noteListModel()
                                             ->data(index, BookmarksNoteListModel::PrimaryTextRole)
                                             .toString();
        const QString color = viewModel.noteListModel()
                                       ->data(index, BookmarksNoteListModel::BookmarkColorRole)
                                       .toString();
        primaryTextAndColorPairs.push_back(QStringLiteral("%1=%2").arg(primaryText, color));
    }
    primaryTextAndColorPairs.sort();
    noteIds.sort();
    QCOMPARE(
        primaryTextAndColorPairs,
        QStringList({QStringLiteral("Blue summary=#3B82F6"), QStringLiteral("Pink summary=#EC4899")}));
    QCOMPARE(noteIds, QStringList({QStringLiteral("note-blue"), QStringLiteral("note-pink")}));

    viewModel.setSelectedIndex(6); // blue
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::PrimaryTextRole).toString(),
        QStringLiteral("Blue summary"));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::DisplayDateRole).toString(),
        QStringLiteral("2026-03-01"));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::ImageRole).toBool(),
        true);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::ImageSourceRole).toString(),
        QUrl::fromLocalFile(
            QDir(hubPath).filePath(
                QStringLiteral("BookmarksVmHub.wsresources/blue-preview.wsresource/blue-preview.png"))).toString());
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::TagsRole).toStringList(),
        QStringList({QStringLiteral("brand"), QStringLiteral("campaign")}));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::FoldersRole).toStringList(),
        QStringList({QStringLiteral("Draft")}));

    viewModel.setSelectedIndex(9); // pink
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::PrimaryTextRole).toString(),
        QStringLiteral("Pink summary"));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::TagsRole).toStringList(),
        QStringList({QStringLiteral("release")}));
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::FoldersRole).toStringList(),
        QStringList({QStringLiteral("Draft")}));
}

void HierarchyViewModelsTest::bookmarksViewModel_searchText_filtersVisibleNotesByBodyContent()
{
    QString hubPath;
    QVERIFY(prepareBookmarksHub(&hubPath));

    BookmarksHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.noteListModel()->setSearchText(QStringLiteral("pink summary"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::IdRole).toString(),
        QStringLiteral("note-pink"));

    viewModel.setSelectedIndex(6); // blue
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.noteListModel()->setSearchText(QStringLiteral("blue summary"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            BookmarksNoteListModel::IdRole).toString(),
        QStringLiteral("note-blue"));
}

void HierarchyViewModelsTest::bookmarksViewModel_saveCurrentBodyText_rewritesWsnbody()
{
    QString hubPath;
    QVERIFY(prepareBookmarksHub(&hubPath));

    BookmarksHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(6); // blue
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    viewModel.noteListModel()->setCurrentIndex(0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-blue"));

    const QString editedBody = QStringLiteral("\nBlue edited first line\nBlue edited second line\n");
    QVERIFY(viewModel.saveBodyTextForNote(QStringLiteral("note-blue"), editedBody));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), editedBody);

    const QString bodyPath = QDir(hubPath).filePath(
        QStringLiteral("BookmarksVmHub.wscontents/Library.wslibrary/Blue.wsnote/Blue.wsnbody"));
    const QString savedBodyXml = readUtf8File(bodyPath);
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph></paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Blue edited first line</paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Blue edited second line</paragraph>")));
}

void HierarchyViewModelsTest::bookmarksViewModel_removeNoteById_reselectsVisibleNeighbor()
{
    QString hubPath;
    QVERIFY(prepareBookmarksHub(&hubPath));

    BookmarksHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    viewModel.noteListModel()->setCurrentIndex(0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-blue"));

    QVERIFY(viewModel.removeNoteById(QStringLiteral("note-blue")));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(viewModel.noteListModel()->currentIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-pink"));
}

void HierarchyViewModelsTest::bookmarksViewModel_formatsDisplayDateWithSystemCalendarStore()
{
    SystemCalendarStore calendarStore;
    BookmarksHierarchyViewModel viewModel;
    viewModel.setSystemCalendarStore(&calendarStore);

    LibraryNoteRecord note;
    note.noteId = QStringLiteral("bookmark-note");
    note.bodyPlainText = QStringLiteral("Bookmark body");
    note.bodyFirstLine = QStringLiteral("Bookmark body");
    note.createdAt = QStringLiteral("2026-03-08-09-30-00");
    note.lastModifiedAt = QStringLiteral("2026-03-09-10-20-00");
    note.bookmarked = true;
    note.bookmarkColors = {QStringLiteral("blue")};

    viewModel.applyRuntimeSnapshot({note}, true);

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                     viewModel.noteListModel()->index(0, 0),
                     BookmarksNoteListModel::DisplayDateRole)
                 .toString(),
        calendarStore.formatNoteDate(note.lastModifiedAt, note.createdAt));
}

void HierarchyViewModelsTest::resourcesViewModel_exposesSupportedTypeTree()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString resourcesPath = QDir(tempDir.path()).filePath(QStringLiteral("ResourcesVm.wsresources"));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("logo"),
        QStringLiteral("logo.png"),
        QStringLiteral("png")));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("brief"),
        QStringLiteral("brief.pdf"),
        QStringLiteral("pdf")));

    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({
        QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource")),
        QDir(resourcesPath).filePath(QStringLiteral("brief.wsresource"))
    });
    QVERIFY(!viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QCOMPARE(
        viewModel.resourcePaths(),
        QStringList({
            QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource")),
            QDir(resourcesPath).filePath(QStringLiteral("brief.wsresource"))
        }));
    QCOMPARE(viewModel.itemModel()->rowCount(), 6);

    auto findIndexByKey = [&viewModel](const QString& key)
    {
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            if (viewModel.itemModel()->data(
                    viewModel.itemModel()->index(row, 0),
                    ResourcesHierarchyModel::KeyRole).toString() == key)
            {
                return row;
            }
        }
        return -1;
    };

    const int documentBucketIndex = findIndexByKey(QStringLiteral("bucket:document"));
    const int imageBucketIndex = findIndexByKey(QStringLiteral("bucket:image"));
    const int imageFormatIndex = findIndexByKey(QStringLiteral("format:image:.png"));
    const int imageAssetIndex = findIndexByKey(QStringLiteral("asset:ResourcesVm.wsresources/logo.wsresource"));
    QVERIFY(documentBucketIndex >= 0);
    QVERIFY(imageBucketIndex >= 0);
    QVERIFY(imageFormatIndex >= 0);
    QVERIFY(imageAssetIndex >= 0);

    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageBucketIndex, 0),
            ResourcesHierarchyModel::LabelRole).toString(),
        QStringLiteral("Image"));
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageBucketIndex, 0),
            ResourcesHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageBucketIndex, 0),
            ResourcesHierarchyModel::ShowChevronRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageFormatIndex, 0),
            ResourcesHierarchyModel::LabelRole).toString(),
        QStringLiteral(".png"));
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageFormatIndex, 0),
            ResourcesHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageFormatIndex, 0),
            ResourcesHierarchyModel::ShowChevronRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageAssetIndex, 0),
            ResourcesHierarchyModel::LabelRole).toString(),
        QStringLiteral("logo"));
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageAssetIndex, 0),
            ResourcesHierarchyModel::DepthRole).toInt(),
        2);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageAssetIndex, 0),
            ResourcesHierarchyModel::TypeRole).toString(),
        QStringLiteral("image"));
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageAssetIndex, 0),
            ResourcesHierarchyModel::ResourcePathRole).toString(),
        QStringLiteral("ResourcesVm.wsresources/logo.wsresource"));
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageAssetIndex, 0),
            ResourcesHierarchyModel::AssetPathRole).toString(),
        QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource/logo.png")));

    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    auto findNodeByKey = [&hierarchyModel](const QString& key)
    {
        for (const QVariant& entry : hierarchyModel)
        {
            const QVariantMap map = entry.toMap();
            if (map.value(QStringLiteral("key")).toString() == key)
            {
                return map;
            }
        }
        return QVariantMap{};
    };

    const QVariantMap imageNode = findNodeByKey(QStringLiteral("bucket:image"));
    const QVariantMap formatNode = findNodeByKey(QStringLiteral("format:image:.png"));
    const QVariantMap assetNode = findNodeByKey(QStringLiteral("asset:ResourcesVm.wsresources/logo.wsresource"));
    QCOMPARE(imageNode.value(QStringLiteral("iconName")).toString(), QStringLiteral("virtualFolder"));
    QCOMPARE(formatNode.value(QStringLiteral("label")).toString(), QStringLiteral(".png"));
    QCOMPARE(assetNode.value(QStringLiteral("iconName")).toString(), QStringLiteral("imageToImage"));
    QCOMPARE(assetNode.value(QStringLiteral("resourceId")).toString(), QStringLiteral("logo"));
    QCOMPARE(assetNode.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));

    viewModel.setSelectedIndex(imageBucketIndex);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.canRenameItem(imageBucketIndex));
    QVERIFY(!viewModel.renameItem(imageBucketIndex, QStringLiteral("Image-Renamed")));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), hierarchyModel.size());
    viewModel.setSelectedIndex(imageFormatIndex);
    QVERIFY(!viewModel.canRenameItem(imageFormatIndex));
    QVERIFY(!viewModel.renameItem(imageFormatIndex, QStringLiteral(".png-renamed")));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), hierarchyModel.size());
}

void HierarchyViewModelsTest::resourcesViewModel_applyRuntimeSnapshot_preservesExpandedBucketState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString resourcesPath = QDir(tempDir.path()).filePath(QStringLiteral("RuntimeResources.wsresources"));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("logo"),
        QStringLiteral("logo.png"),
        QStringLiteral("png")));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("manual"),
        QStringLiteral("manual.pdf"),
        QStringLiteral("pdf")));

    ResourcesHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource"))},
        QStringLiteral("/tmp/Resources.wsresources"),
        true);

    QVERIFY(viewModel.setItemExpanded(0, true));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ResourcesHierarchyModel::ExpandedRole)
            .toBool(),
        true);

    viewModel.applyRuntimeSnapshot(
        {
            QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource")),
            QDir(resourcesPath).filePath(QStringLiteral("manual.wsresource"))
        },
        QStringLiteral("/tmp/Resources.wsresources"),
        true);

    int imageBucketIndex = -1;
    for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
    {
        if (viewModel.itemModel()->data(
                viewModel.itemModel()->index(row, 0),
                ResourcesHierarchyModel::KeyRole).toString() == QStringLiteral("bucket:image"))
        {
            imageBucketIndex = row;
            break;
        }
    }
    QVERIFY(imageBucketIndex >= 0);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(imageBucketIndex, 0),
            ResourcesHierarchyModel::ExpandedRole)
            .toBool(),
        true);
}

void HierarchyViewModelsTest::progressViewModel_supportsCrudContract()
{
    QString hubPath;
    QVERIFY(prepareProgressHub(&hubPath));

    ProgressHierarchyViewModel viewModel;
    QVERIFY(viewModel.loadFromWshub(hubPath));
    QVERIFY(!viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("First draft"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProgressHierarchyModel::ShowChevronRole)
                 .toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProgressHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("inlineinlineEdit"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("In Progress"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProgressHierarchyModel::ShowChevronRole)
                 .toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), ProgressHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("progressresume"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("Reviewing"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), ProgressHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), ProgressHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("showLogs"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("Done"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), ProgressHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), ProgressHierarchyModel::IconNameRole)
                 .toString(),
        QStringLiteral("validator"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(7, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("Lagacy"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(9, 0), ProgressHierarchyModel::LabelRole).toString(),
        QStringLiteral("Delete review"));
    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    QCOMPARE(hierarchyModel.size(), 10);
    QCOMPARE(hierarchyModel.at(0).toMap().value(QStringLiteral("iconName")).toString(), QStringLiteral("inlineinlineEdit"));
    QCOMPARE(hierarchyModel.at(6).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("Done"));
    QCOMPARE(hierarchyModel.at(6).toMap().value(QStringLiteral("progressValue")).toInt(), 6);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.canRenameItem(0));
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Progress-Header")));

    viewModel.setSelectedIndex(6);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-done"));
    QVERIFY(!viewModel.canRenameItem(6));
    QVERIFY(!viewModel.renameItem(6, QStringLiteral("Done-Renamed")));
    QCOMPARE(viewModel.itemLabel(6), QStringLiteral("Done"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
}

void HierarchyViewModelsTest::progressViewModel_applyRuntimeSnapshot_preservesSelectionAndVisibleNotes()
{
    QString hubPath;
    QVERIFY(prepareProgressHub(&hubPath));

    ProgressHierarchyViewModel viewModel;
    QVERIFY(viewModel.loadFromWshub(hubPath));

    QVERIFY(viewModel.setItemExpanded(0, true));
    viewModel.setSelectedIndex(6);
    QCOMPARE(viewModel.selectedIndex(), 6);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProgressHierarchyModel::ExpandedRole)
            .toBool(),
        true);

    viewModel.applyRuntimeSnapshot(
        0,
        {
            QStringLiteral("Ready"),
            QStringLiteral("Pending"),
            QStringLiteral("InProgress"),
            QStringLiteral("Done")
        },
        QDir(QDir(hubPath).filePath(QStringLiteral("ProgressVmHub.wscontents"))).filePath(
            QStringLiteral("Progress.wsprogress")),
        true);

    QCOMPARE(viewModel.selectedIndex(), 6);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), ProgressHierarchyModel::ExpandedRole)
            .toBool(),
        true);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        QStringLiteral("note-done"));
}

void HierarchyViewModelsTest::eventViewModel_supportsCrudContract()
{
    EventHierarchyViewModel viewModel;
    viewModel.setEventNames({QStringLiteral("Kickoff")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Kickoff-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Kickoff-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
}

void HierarchyViewModelsTest::eventViewModel_applyRuntimeSnapshot_preservesExpandedBucketState()
{
    EventHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot({QStringLiteral("Launch")}, QStringLiteral("/tmp/Event.wsevent"), true);

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.selectedIndex(), 0);

    viewModel.applyRuntimeSnapshot(
        {QStringLiteral("Launch"), QStringLiteral("Workshop")},
        QStringLiteral("/tmp/Event.wsevent"),
        true);

    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(viewModel.selectedIndex(), 0),
            EventHierarchyModel::LabelRole).toString(),
        QStringLiteral("Launch"));
}

void HierarchyViewModelsTest::presetViewModel_supportsCrudContract()
{
    PresetHierarchyViewModel viewModel;
    viewModel.setPresetNames({QStringLiteral("Executive Summary")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Executive-Summary-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Executive-Summary-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
}

void HierarchyViewModelsTest::presetViewModel_applyRuntimeSnapshot_preservesExpandedBucketState()
{
    PresetHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot({QStringLiteral("Classic")}, QStringLiteral("/tmp/Preset.wspreset"), true);

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.selectedIndex(), 0);

    viewModel.applyRuntimeSnapshot(
        {QStringLiteral("Classic"), QStringLiteral("Minimal")},
        QStringLiteral("/tmp/Preset.wspreset"),
        true);

    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(viewModel.selectedIndex(), 0),
            PresetHierarchyModel::LabelRole).toString(),
        QStringLiteral("Classic"));
}

void HierarchyViewModelsTest::tagsViewModel_supportsCrudContract()
{
    TagsHierarchyViewModel viewModel;
    viewModel.setTagDepthEntries({
        {QStringLiteral("brand"), QStringLiteral("Brand"), 0},
        {QStringLiteral("brand/social"), QStringLiteral("Social"), 1}
    });
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);

    viewModel.setSelectedIndex(1);
    QVERIFY(viewModel.renameItem(1, QStringLiteral("Social-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Social-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), TagsHierarchyModel::ExpandedRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), TagsHierarchyModel::DepthRole).toInt(),
        2);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
}

void HierarchyViewModelsTest::hierarchyViewModels_exposeCapabilityInterfaces()
{
    LibraryHierarchyViewModel libraryViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&libraryViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&libraryViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&libraryViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyReorderCapability*>(&libraryViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyNoteDropCapability*>(&libraryViewModel) != nullptr);

    ProjectsHierarchyViewModel projectsViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&projectsViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&projectsViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&projectsViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyReorderCapability*>(&projectsViewModel) != nullptr);

    BookmarksHierarchyViewModel bookmarksViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&bookmarksViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&bookmarksViewModel) != nullptr);

    ResourcesHierarchyViewModel resourcesViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&resourcesViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&resourcesViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&resourcesViewModel) != nullptr);

    ProgressHierarchyViewModel progressViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&progressViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&progressViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&progressViewModel) != nullptr);

    EventHierarchyViewModel eventViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&eventViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&eventViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&eventViewModel) != nullptr);

    PresetHierarchyViewModel presetViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&presetViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&presetViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&presetViewModel) != nullptr);

    TagsHierarchyViewModel tagsViewModel;
    QVERIFY(qobject_cast<IHierarchyRenameCapability*>(&tagsViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyCrudCapability*>(&tagsViewModel) != nullptr);
    QVERIFY(qobject_cast<IHierarchyExpansionCapability*>(&tagsViewModel) != nullptr);
}

void HierarchyViewModelsTest::hierarchyViewModels_hierarchyModel_exposesCountRoleAcrossDomains()
{
    const auto assertCountRole = [](const QVariantList& hierarchyModel)
    {
        QVERIFY(!hierarchyModel.isEmpty());
        for (const QVariant& entry : hierarchyModel)
        {
            const QVariantMap node = entry.toMap();
            QVERIFY(node.contains(QStringLiteral("count")));
            bool ok = false;
            const int countValue = node.value(QStringLiteral("count")).toInt(&ok);
            QVERIFY(ok);
            QVERIFY(countValue >= 0);
        }
    };

    LibraryHierarchyViewModel libraryViewModel;
    libraryViewModel.setDepthItems(QVariantList{
        QVariantMap{{QStringLiteral("label"), QStringLiteral("Root")}, {QStringLiteral("depth"), 0}}
    });
    assertCountRole(libraryViewModel.hierarchyModel());

    ProjectsHierarchyViewModel projectsViewModel;
    projectsViewModel.setProjectNames({QStringLiteral("Alpha")});
    assertCountRole(projectsViewModel.hierarchyModel());

    BookmarksHierarchyViewModel bookmarksViewModel;
    assertCountRole(bookmarksViewModel.hierarchyModel());

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString resourcesPath = QDir(tempDir.path()).filePath(QStringLiteral("CountRole.wsresources"));
    QVERIFY(QDir().mkpath(resourcesPath));
    QVERIFY(createResourcePackage(
        resourcesPath,
        QStringLiteral("logo"),
        QStringLiteral("logo.png"),
        QStringLiteral("png")));

    ResourcesHierarchyViewModel resourcesViewModel;
    resourcesViewModel.setResourcePaths({
        QDir(resourcesPath).filePath(QStringLiteral("logo.wsresource"))
    });
    assertCountRole(resourcesViewModel.hierarchyModel());

    ProgressHierarchyViewModel progressViewModel;
    assertCountRole(progressViewModel.hierarchyModel());

    EventHierarchyViewModel eventViewModel;
    eventViewModel.setEventNames({QStringLiteral("Kickoff")});
    assertCountRole(eventViewModel.hierarchyModel());

    PresetHierarchyViewModel presetViewModel;
    presetViewModel.setPresetNames({QStringLiteral("Classic")});
    assertCountRole(presetViewModel.hierarchyModel());

    TagsHierarchyViewModel tagsViewModel;
    tagsViewModel.setTagDepthEntries({
        {QStringLiteral("brand"), QStringLiteral("Brand"), 0}
    });
    assertCountRole(tagsViewModel.hierarchyModel());
}

void HierarchyViewModelsTest::tagsViewModel_applyRuntimeSnapshot_preservesExpandedStateAcrossHierarchyRefresh()
{
    TagsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("brand"), QStringLiteral("Brand"), 0},
            {QStringLiteral("campaign"), QStringLiteral("Campaign"), 1}
        },
        QStringLiteral("/tmp/Tags.wstags"),
        true);

    QVERIFY(viewModel.setItemExpanded(0, true));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), TagsHierarchyModel::ExpandedRole).toBool(),
        true);

    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("brand"), QStringLiteral("Brand"), 0},
            {QStringLiteral("campaign"), QStringLiteral("Campaign"), 1},
            {QStringLiteral("launch"), QStringLiteral("Launch"), 1}
        },
        QStringLiteral("/tmp/Tags.wstags"),
        true);

    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), TagsHierarchyModel::ExpandedRole).toBool(),
        true);
}

void HierarchyViewModelsTest::libraryViewModel_reactsToNoteListModelMutation()
{
    LibraryHierarchyViewModel viewModel;
    QCOMPARE(viewModel.noteItemCount(), 0);

    QSignalSpy noteCountSpy(&viewModel, &LibraryHierarchyViewModel::noteItemCountChanged);
    QVector<LibraryNoteListItem> items;
    LibraryNoteListItem first;
    first.id = QStringLiteral("note-a");
    first.primaryText = QStringLiteral("A");
    items.push_back(first);
    LibraryNoteListItem second;
    second.id = QStringLiteral("note-b");
    second.primaryText = QStringLiteral("B");
    items.push_back(second);

    viewModel.noteListModel()->setItems(items);

    QCOMPARE(viewModel.noteItemCount(), 2);
    QVERIFY(noteCountSpy.count() > 0);
}

void HierarchyViewModelsTest::projectsModel_appliesCorrectionAndRaisesHookSignal()
{
    ProjectsHierarchyModel model;
    QSignalSpy issueSpy(&model, &ProjectsHierarchyModel::validationIssueRaised);
    QSignalSpy correctedSpy(&model, &ProjectsHierarchyModel::itemCorrected);

    QVector<ProjectsHierarchyItem> items;
    ProjectsHierarchyItem item;
    item.depth = -3;
    item.label = QStringLiteral("   ");
    items.push_back(item);

    model.setItems(std::move(items));

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(model.data(index, ProjectsHierarchyModel::DepthRole).toInt(), 0);
    QCOMPARE(model.data(index, ProjectsHierarchyModel::LabelRole).toString(), QString());
    QVERIFY(model.correctionCount() >= 1);
    QVERIFY(issueSpy.count() >= 1);
    QVERIFY(correctedSpy.count() >= 1);
}

void HierarchyViewModelsTest::projectsModel_recomputesChevronByDepth()
{
    ProjectsHierarchyModel model;

    QVector<ProjectsHierarchyItem> items;
    ProjectsHierarchyItem parent;
    parent.depth = 0;
    parent.label = QStringLiteral("Parent");
    parent.showChevron = false;
    items.push_back(parent);

    ProjectsHierarchyItem child;
    child.depth = 1;
    child.label = QStringLiteral("Child");
    child.showChevron = true;
    items.push_back(child);

    ProjectsHierarchyItem leaf;
    leaf.depth = 0;
    leaf.label = QStringLiteral("Leaf");
    leaf.showChevron = true;
    items.push_back(leaf);

    model.setItems(std::move(items));

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.data(model.index(0, 0), ProjectsHierarchyModel::ItemKeyRole).toString(), QStringLiteral("Parent"));
    QCOMPARE(
        model.data(model.index(1, 0), ProjectsHierarchyModel::ItemKeyRole).toString(),
        QStringLiteral("Parent/Child"));
    QCOMPARE(model.data(model.index(2, 0), ProjectsHierarchyModel::ItemKeyRole).toString(), QStringLiteral("Leaf"));
    QCOMPARE(model.data(model.index(0, 0), ProjectsHierarchyModel::IconNameRole).toString(), QStringLiteral("customFolder"));
    QCOMPARE(model.data(model.index(1, 0), ProjectsHierarchyModel::IconNameRole).toString(), QStringLiteral("customFolder"));
    QCOMPARE(model.data(model.index(2, 0), ProjectsHierarchyModel::IconNameRole).toString(), QStringLiteral("customFolder"));
    QCOMPARE(model.data(model.index(0, 0), ProjectsHierarchyModel::ShowChevronRole).toBool(), true);
    QCOMPARE(model.data(model.index(1, 0), ProjectsHierarchyModel::ShowChevronRole).toBool(), false);
    QCOMPARE(model.data(model.index(2, 0), ProjectsHierarchyModel::ShowChevronRole).toBool(), false);
}

void HierarchyViewModelsTest::projectsModel_strictValidation_throwsException()
{
    ProjectsHierarchyModel model;
    model.setStrictValidation(true);

    QVector<ProjectsHierarchyItem> items;
    ProjectsHierarchyItem item;
    item.depth = -1;
    item.label.clear();
    items.push_back(item);

    bool thrown = false;
    try
    {
        model.setItems(std::move(items));
    }
    catch (const std::runtime_error&)
    {
        thrown = true;
    }
    QVERIFY(thrown);
    QCOMPARE(model.rowCount(), 0);
}

void HierarchyViewModelsTest::noteListModel_correctsColorAndTextFields()
{
    LibraryNoteListModel model;

    QVector<LibraryNoteListItem> items;
    LibraryNoteListItem item;
    item.id = QStringLiteral("note-1");
    item.primaryText = QStringLiteral("   ");
    item.displayDate = QStringLiteral(" 2026-03-07 ");
    item.folders = {QStringLiteral(""), QStringLiteral("folder-a"), QStringLiteral("folder-a")};
    item.tags = {QStringLiteral(""), QStringLiteral("tag-a"), QStringLiteral("tag-a")};
    item.bookmarked = true;
    item.bookmarkColor = QStringLiteral("not-a-hex");
    items.push_back(item);

    model.setItems(std::move(items));

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(model.data(index, LibraryNoteListModel::PrimaryTextRole).toString(), QString());
    QCOMPARE(model.data(index, LibraryNoteListModel::DisplayDateRole).toString(), QStringLiteral("2026-03-07"));
    QCOMPARE(model.data(index, LibraryNoteListModel::FoldersRole).toStringList(),
             QStringList({QStringLiteral("folder-a")}));
    QCOMPARE(model.data(index, LibraryNoteListModel::TagsRole).toStringList(),
             QStringList({QStringLiteral("tag-a")}));
    QCOMPARE(model.data(index, LibraryNoteListModel::BookmarkColorRole).toString(), QString());
    QCOMPARE(model.roleNames().value(LibraryNoteListModel::NoteIdRole), QByteArray("noteId"));
    QCOMPARE(model.data(index, LibraryNoteListModel::NoteIdRole).toString(), QStringLiteral("note-1"));
    QVERIFY(model.correctionCount() >= 1);
}

void HierarchyViewModelsTest::noteListModel_sortsNewestModifiedItemFirst()
{
    LibraryNoteListModel model;

    LibraryNoteListItem oldest;
    oldest.id = QStringLiteral("note-oldest");
    oldest.primaryText = QStringLiteral("Oldest");
    oldest.createdAt = QStringLiteral("2026-03-01-00-00-00");
    oldest.lastModifiedAt = QStringLiteral("2026-03-01-00-00-00");

    LibraryNoteListItem newest;
    newest.id = QStringLiteral("note-newest");
    newest.primaryText = QStringLiteral("Newest");
    newest.createdAt = QStringLiteral("2026-03-02-00-00-00");
    newest.lastModifiedAt = QStringLiteral("2026-03-03-00-00-00");

    LibraryNoteListItem fallbackCreated;
    fallbackCreated.id = QStringLiteral("note-created-fallback");
    fallbackCreated.primaryText = QStringLiteral("Created fallback");
    fallbackCreated.createdAt = QStringLiteral("2026-03-02-12-00-00");
    fallbackCreated.lastModifiedAt = QString();

    model.setItems({oldest, newest, fallbackCreated});

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.data(model.index(0, 0), LibraryNoteListModel::IdRole).toString(), QStringLiteral("note-newest"));
    QCOMPARE(
        model.data(model.index(1, 0), LibraryNoteListModel::IdRole).toString(),
        QStringLiteral("note-created-fallback"));
    QCOMPARE(model.data(model.index(2, 0), LibraryNoteListModel::IdRole).toString(), QStringLiteral("note-oldest"));
}

void HierarchyViewModelsTest::noteListModel_currentSelection_exposesBodyText()
{
    LibraryNoteListModel model;
    QSignalSpy currentBodySpy(&model, &LibraryNoteListModel::currentBodyTextChanged);

    LibraryNoteListItem alpha;
    alpha.id = QStringLiteral("note-alpha");
    alpha.primaryText = QStringLiteral("Alpha preview");
    alpha.bodyText = QStringLiteral("\nAlpha body first line\nAlpha body second line\n");
    alpha.searchableText = alpha.bodyText;
    alpha.createdAt = QStringLiteral("2026-03-01-00-00-00");
    alpha.lastModifiedAt = QStringLiteral("2026-03-01-00-00-00");

    LibraryNoteListItem beta;
    beta.id = QStringLiteral("note-beta");
    beta.primaryText = QStringLiteral("Beta preview");
    beta.bodyText = QStringLiteral("Beta body summary");
    beta.searchableText = beta.bodyText;
    beta.createdAt = QStringLiteral("2026-03-02-00-00-00");
    beta.lastModifiedAt = QStringLiteral("2026-03-03-00-00-00");

    model.setItems({alpha, beta});

    QCOMPARE(model.currentIndex(), -1);
    QCOMPARE(model.currentNoteId(), QString());
    QCOMPARE(model.currentBodyText(), QString());
    QCOMPARE(
        model.data(model.index(0, 0), LibraryNoteListModel::BodyTextRole).toString(),
        QStringLiteral("Beta body summary"));

    model.setCurrentIndex(0);
    QCOMPARE(model.currentIndex(), 0);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-beta"));
    QCOMPARE(model.currentBodyText(), QStringLiteral("Beta body summary"));

    model.setCurrentIndex(1);
    QCOMPARE(model.currentIndex(), 1);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-alpha"));
    QCOMPARE(model.currentBodyText(), QStringLiteral("\nAlpha body first line\nAlpha body second line\n"));

    model.setSearchText(QStringLiteral("alpha"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.currentIndex(), 0);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-alpha"));
    QCOMPARE(model.currentBodyText(), QStringLiteral("\nAlpha body first line\nAlpha body second line\n"));
    QVERIFY(currentBodySpy.count() >= 2);
}

void HierarchyViewModelsTest::noteListModel_searchText_filtersAgainstSearchableBodyContent()
{
    LibraryNoteListModel model;

    LibraryNoteListItem alpha;
    alpha.id = QStringLiteral("note-alpha");
    alpha.primaryText = QStringLiteral("Alpha preview");
    alpha.searchableText = QStringLiteral("Alpha preview\nBody keyword moonshot launch plan");
    alpha.createdAt = QStringLiteral("2026-03-01-00-00-00");
    alpha.lastModifiedAt = QStringLiteral("2026-03-01-00-00-00");

    LibraryNoteListItem beta;
    beta.id = QStringLiteral("note-beta");
    beta.primaryText = QStringLiteral("Beta preview");
    beta.searchableText = QStringLiteral("Beta preview\nBody keyword release checklist");
    beta.createdAt = QStringLiteral("2026-03-02-00-00-00");
    beta.lastModifiedAt = QStringLiteral("2026-03-03-00-00-00");

    model.setItems({alpha, beta});
    QCOMPARE(model.rowCount(), 2);

    model.setSearchText(QStringLiteral("moonshot launch"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), LibraryNoteListModel::IdRole).toString(), QStringLiteral("note-alpha"));

    model.setSearchText(QStringLiteral("release"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), LibraryNoteListModel::IdRole).toString(), QStringLiteral("note-beta"));

    model.setSearchText(QStringLiteral("   "));
    QCOMPARE(model.rowCount(), 2);
}

void HierarchyViewModelsTest::noteListModel_searchText_mustIgnoreInternalIdFallback()
{
    LibraryNoteListModel model;

    LibraryNoteListItem blank;
    blank.id = QStringLiteral("internal-note-id");
    blank.primaryText = QString();
    blank.bodyText = QString();
    blank.searchableText = QString();

    LibraryNoteListItem visible;
    visible.id = QStringLiteral("visible-note-id");
    visible.primaryText = QStringLiteral("Visible preview");
    visible.bodyText = QStringLiteral("Visible body text");
    visible.searchableText = QString();

    model.setItems({blank, visible});
    QCOMPARE(model.rowCount(), 2);

    model.setSearchText(QStringLiteral("internal-note-id"));
    QCOMPARE(model.rowCount(), 0);

    model.setSearchText(QStringLiteral("Visible preview"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), LibraryNoteListModel::IdRole).toString(), QStringLiteral("visible-note-id"));
}

void HierarchyViewModelsTest::noteListModel_limitsPrimaryTextToFiveLines()
{
    LibraryNoteListModel model;

    LibraryNoteListItem item;
    item.id = QStringLiteral("note-1");
    item.primaryText = QStringLiteral("line1\nline2\nline3\nline4\nline5\nline6\nline7");

    model.setItems({item});

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(
        model.data(index, LibraryNoteListModel::PrimaryTextRole).toString(),
        QStringLiteral("line1\nline2\nline3\nline4\nline5"));
}

void HierarchyViewModelsTest::noteListModel_exposesImageRoles()
{
    LibraryNoteListModel model;
    const QString previewPath = QDir::temp().filePath(QStringLiteral("whatson-preview.png"));

    LibraryNoteListItem item;
    item.id = QStringLiteral("note-image");
    item.primaryText = QStringLiteral("Image preview");
    item.image = true;
    item.imageSource = previewPath;

    model.setItems({item});

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(model.data(index, LibraryNoteListModel::ImageRole).toBool(), true);
    QCOMPARE(
        model.data(index, LibraryNoteListModel::ImageSourceRole).toString(),
        QUrl::fromLocalFile(previewPath).toString());
}

void HierarchyViewModelsTest::libraryViewModel_formatsDisplayDateWithSystemCalendarStore()
{
    SystemCalendarStore calendarStore;
    LibraryHierarchyViewModel viewModel;
    viewModel.setSystemCalendarStore(&calendarStore);

    LibraryNoteRecord note;
    note.noteId = QStringLiteral("library-note");
    note.bodyPlainText = QStringLiteral("Library body");
    note.bodyFirstLine = QStringLiteral("Library body");
    note.createdAt = QStringLiteral("2026-03-05-08-00-00");
    note.lastModifiedAt = QStringLiteral("2026-03-11-18-45-00");

    viewModel.applyRuntimeSnapshot(
        QStringLiteral("/tmp/TestLocale.wshub"),
        {note},
        {},
        {},
        {},
        QString(),
        true);

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                     viewModel.noteListModel()->index(0, 0),
                     LibraryNoteListModel::DisplayDateRole)
                 .toString(),
        calendarStore.formatNoteDate(note.lastModifiedAt, note.createdAt));
}

QTEST_APPLESS_MAIN(HierarchyViewModelsTest)

#include "test_hierarchy_viewmodels.moc"
