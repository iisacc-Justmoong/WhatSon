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
#include "file/note/WhatSonBookmarkColorPalette.hpp"

#include <QDir>
#include <QFile>
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

    QString makeWsnHeadText(
        const QString& noteId,
        bool bookmarked,
        const QStringList& bookmarkColors,
        const QStringList& tags = {})
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
        text += QStringLiteral("    <project>General</project>\n");
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
        text += QStringLiteral("    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n");
        text += QStringLiteral("    <isPreset>false</isPreset>\n");
        text += QStringLiteral("  </head>\n");
        text += QStringLiteral("</contents>\n");
        return text;
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
        const QString bluePreviewPath = QDir(resourcesPath).filePath(QStringLiteral("blue-preview.png"));
        if (!writeUtf8File(bluePreviewPath, QStringLiteral("png")))
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
                    "    <resource format=\".png\" resourcePath=\"BookmarksVmHub.wsresources/blue-preview.png\" />\n"
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
} // namespace

class HierarchyViewModelsTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void libraryViewModel_supportsCrudContract();
    void projectsViewModel_supportsCrudContract();
    void projectsViewModel_reactsToModelMutation();
    void projectsViewModel_moveFolderBefore_persistsFoldersFileAndDepth();
    void projectsViewModel_applyHierarchyNodes_persistsLvrsEditableMove();
    void bookmarksViewModel_supportsCrudContract();
    void bookmarksViewModel_loadFromWshub_filtersBookmarkedNotesAndMapsHexColor();
    void bookmarksViewModel_searchText_filtersVisibleNotesByBodyContent();
    void bookmarksViewModel_saveCurrentBodyText_rewritesWsnbody();
    void bookmarksViewModel_removeNoteById_reselectsVisibleNeighbor();
    void bookmarksViewModel_formatsDisplayDateWithSystemCalendarStore();
    void resourcesViewModel_supportsCrudContract();
    void progressViewModel_supportsCrudContract();
    void eventViewModel_supportsCrudContract();
    void presetViewModel_supportsCrudContract();
    void tagsViewModel_supportsCrudContract();
    void libraryViewModel_reactsToNoteListModelMutation();
    void projectsModel_appliesCorrectionAndRaisesHookSignal();
    void projectsModel_recomputesChevronByDepth();
    void projectsModel_strictValidation_throwsException();
    void noteListModel_correctsColorAndTextFields();
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

void HierarchyViewModelsTest::projectsViewModel_supportsCrudContract()
{
    ProjectsHierarchyViewModel viewModel;
    viewModel.setProjectNames({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Alpha-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Alpha-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
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

void HierarchyViewModelsTest::projectsViewModel_moveFolderBefore_persistsFoldersFileAndDepth()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString foldersFilePath = tempDir.filePath(QStringLiteral("Folders.wsfolders"));

    ProjectsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Research/Competitor"), QStringLiteral("Competitor"), 1},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        foldersFilePath,
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
        1);

    QVERIFY(viewModel.canAcceptFolderDrop(0, 1, true));
    QVERIFY(viewModel.moveFolder(0, 1, true));

    const QJsonDocument foldersDocument = QJsonDocument::fromJson(readUtf8File(foldersFilePath).toUtf8());
    QVERIFY(foldersDocument.isObject());
    const QJsonArray folderArray = foldersDocument.object().value(QStringLiteral("folders")).toArray();
    QCOMPARE(folderArray.size(), 1);

    const QJsonObject researchObject = folderArray.at(0).toObject();
    QCOMPARE(researchObject.value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    const QJsonArray childArray = researchObject.value(QStringLiteral("children")).toArray();
    QCOMPARE(childArray.size(), 2);
    QCOMPARE(childArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research/Competitor"));
    QCOMPARE(childArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research/Brand"));
}

void HierarchyViewModelsTest::projectsViewModel_applyHierarchyNodes_persistsLvrsEditableMove()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString foldersFilePath = tempDir.filePath(QStringLiteral("Folders.wsfolders"));

    ProjectsHierarchyViewModel viewModel;
    viewModel.applyRuntimeSnapshot(
        {
            {QStringLiteral("Research"), QStringLiteral("Research"), 0},
            {QStringLiteral("Research/Competitor"), QStringLiteral("Competitor"), 1},
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0}
        },
        foldersFilePath,
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
        1);

    const QJsonDocument foldersDocument = QJsonDocument::fromJson(readUtf8File(foldersFilePath).toUtf8());
    QVERIFY(foldersDocument.isObject());
    const QJsonArray folderArray = foldersDocument.object().value(QStringLiteral("folders")).toArray();
    QCOMPARE(folderArray.size(), 1);

    const QJsonObject researchObject = folderArray.at(0).toObject();
    QCOMPARE(researchObject.value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    const QJsonArray childArray = researchObject.value(QStringLiteral("children")).toArray();
    QCOMPARE(childArray.size(), 2);
    QCOMPARE(childArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research/Competitor"));
    QCOMPARE(childArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research/Brand"));
}

void HierarchyViewModelsTest::bookmarksViewModel_supportsCrudContract()
{
    BookmarksHierarchyViewModel viewModel;
    QVERIFY(!viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QVERIFY(!viewModel.viewOptionsEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 9);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Bookmarks-Header")));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 9);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 9);
}

void HierarchyViewModelsTest::bookmarksViewModel_loadFromWshub_filtersBookmarkedNotesAndMapsHexColor()
{
    QString hubPath;
    QVERIFY(prepareBookmarksHub(&hubPath));

    BookmarksHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(qobject_cast<BookmarksNoteListModel*>(viewModel.noteListModel()) != nullptr);

    QCOMPARE(viewModel.itemModel()->rowCount(), 9);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("red"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(6, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("blue"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(8, 0), BookmarksHierarchyModel::LabelRole).toString(),
        QStringLiteral("pink"));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    QStringList primaryTextAndColorPairs;
    for (int row = 0; row < viewModel.noteListModel()->rowCount(); ++row)
    {
        const QModelIndex index = viewModel.noteListModel()->index(row, 0);
        QVERIFY(viewModel.noteListModel()->data(index, BookmarksNoteListModel::BookmarkedRole).toBool());
        const QString primaryText = viewModel.noteListModel()
                                             ->data(index, BookmarksNoteListModel::PrimaryTextRole)
                                             .toString();
        const QString color = viewModel.noteListModel()
                                       ->data(index, BookmarksNoteListModel::BookmarkColorRole)
                                       .toString();
        primaryTextAndColorPairs.push_back(QStringLiteral("%1=%2").arg(primaryText, color));
    }
    primaryTextAndColorPairs.sort();
    QCOMPARE(
        primaryTextAndColorPairs,
        QStringList({QStringLiteral("Blue summary=#3B82F6"), QStringLiteral("Pink summary=#EC4899")}));

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
            QDir(hubPath).filePath(QStringLiteral("BookmarksVmHub.wsresources/blue-preview.png"))).toString());
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

    viewModel.setSelectedIndex(8); // pink
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

void HierarchyViewModelsTest::resourcesViewModel_supportsCrudContract()
{
    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({QStringLiteral("assets/logo.png")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("assets/logo-renamed.png")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("assets/logo-renamed.png"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
}

void HierarchyViewModelsTest::progressViewModel_supportsCrudContract()
{
    ProgressHierarchyViewModel viewModel;
    viewModel.setProgressState(0, {QStringLiteral("Ready"), QStringLiteral("Done")});
    QVERIFY(!viewModel.renameEnabled());
    QVERIFY(!viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.setSelectedIndex(0);
    QVERIFY(!viewModel.deleteFolderEnabled());
    QVERIFY(!viewModel.canRenameItem(0));
    QVERIFY(!viewModel.renameItem(0, QStringLiteral("Progress-Header")));

    viewModel.setSelectedIndex(1);
    QVERIFY(!viewModel.canRenameItem(1));
    QVERIFY(!viewModel.renameItem(1, QStringLiteral("Ready-Renamed")));
    QCOMPARE(viewModel.itemLabel(1), QStringLiteral("Ready"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
}

void HierarchyViewModelsTest::eventViewModel_supportsCrudContract()
{
    EventHierarchyViewModel viewModel;
    viewModel.setEventNames({QStringLiteral("Kickoff")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Kickoff-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Kickoff-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
}

void HierarchyViewModelsTest::presetViewModel_supportsCrudContract()
{
    PresetHierarchyViewModel viewModel;
    viewModel.setPresetNames({QStringLiteral("Executive Summary")});
    QVERIFY(viewModel.renameEnabled());
    QVERIFY(viewModel.createFolderEnabled());
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    viewModel.setSelectedIndex(0);
    QVERIFY(viewModel.deleteFolderEnabled());
    QVERIFY(viewModel.renameItem(0, QStringLiteral("Executive-Summary-Renamed")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Executive-Summary-Renamed"));
    viewModel.createFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
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
    QCOMPARE(viewModel.itemLabel(viewModel.selectedIndex()), QStringLiteral("Untitled"));
    viewModel.deleteSelectedFolder();
    QCOMPARE(viewModel.itemModel()->rowCount(), 2);
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

void HierarchyViewModelsTest::noteListModel_currentSelection_exposesBodyText()
{
    LibraryNoteListModel model;
    QSignalSpy currentBodySpy(&model, &LibraryNoteListModel::currentBodyTextChanged);

    LibraryNoteListItem alpha;
    alpha.id = QStringLiteral("note-alpha");
    alpha.primaryText = QStringLiteral("Alpha preview");
    alpha.bodyText = QStringLiteral("\nAlpha body first line\nAlpha body second line\n");
    alpha.searchableText = alpha.bodyText;

    LibraryNoteListItem beta;
    beta.id = QStringLiteral("note-beta");
    beta.primaryText = QStringLiteral("Beta preview");
    beta.bodyText = QStringLiteral("Beta body summary");
    beta.searchableText = beta.bodyText;

    model.setItems({alpha, beta});

    QCOMPARE(model.currentIndex(), 0);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-alpha"));
    QCOMPARE(model.currentBodyText(), QStringLiteral("\nAlpha body first line\nAlpha body second line\n"));
    QCOMPARE(model.data(model.index(0, 0), LibraryNoteListModel::BodyTextRole).toString(),
             QStringLiteral("\nAlpha body first line\nAlpha body second line\n"));

    model.setCurrentIndex(1);
    QCOMPARE(model.currentIndex(), 1);
    QCOMPARE(model.currentNoteId(), QStringLiteral("note-beta"));
    QCOMPARE(model.currentBodyText(), QStringLiteral("Beta body summary"));

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

    LibraryNoteListItem beta;
    beta.id = QStringLiteral("note-beta");
    beta.primaryText = QStringLiteral("Beta preview");
    beta.searchableText = QStringLiteral("Beta preview\nBody keyword release checklist");

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
