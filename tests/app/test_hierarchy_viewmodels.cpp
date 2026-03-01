#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
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
#include <QTemporaryDir>
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

    QString makeWsnHeadText(
        const QString& noteId,
        const QString& title,
        bool bookmarked,
        const QStringList& bookmarkColors)
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"%1\">\n").arg(noteId);
        text += QStringLiteral("  <head>\n");
        text += QStringLiteral("    <title>%1</title>\n").arg(title);
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
        text += QStringLiteral("    <tags></tags>\n");
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
        const QString noteAPath = QDir(libraryPath).filePath(QStringLiteral("Blue.wsnote"));
        const QString noteBPath = QDir(libraryPath).filePath(QStringLiteral("Hidden.wsnote"));
        const QString noteCPath = QDir(libraryPath).filePath(QStringLiteral("Pink.wsnote"));
        if (!QDir().mkpath(noteAPath) || !QDir().mkpath(noteBPath) || !QDir().mkpath(noteCPath))
        {
            return false;
        }

        const QString indexText = QStringLiteral(
            "{\n"
            "  \"version\": 1,\n"
            "  \"schema\": \"whatson.library.index\",\n"
            "  \"notes\": [\n"
            "    {\"id\": \"note-blue\", \"title\": \"Blue Note\"},\n"
            "    {\"id\": \"note-hidden\", \"title\": \"Hidden Note\"},\n"
            "    {\"id\": \"note-pink\", \"title\": \"Pink Note\"}\n"
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
                QStringLiteral("Blue Note"),
                true,
                {QStringLiteral("blue")})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteAPath).filePath(QStringLiteral("Blue.wsnbody")),
            makeWsnBodyText(QStringLiteral("Blue summary"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteBPath).filePath(QStringLiteral("Hidden.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-hidden"),
                QStringLiteral("Hidden Note"),
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
                QStringLiteral("Pink Note"),
                true,
                {QStringLiteral("#EC4899")})))
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
    void bookmarksViewModel_supportsCrudContract();
    void bookmarksViewModel_loadFromWshub_filtersBookmarkedNotesAndMapsHexColor();
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
    void noteListModel_limitsDescriptionToFiveLines();
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

    QStringList titleAndColorPairs;
    for (int row = 0; row < viewModel.noteListModel()->rowCount(); ++row)
    {
        const QModelIndex index = viewModel.noteListModel()->index(row, 0);
        QVERIFY(viewModel.noteListModel()->data(index, LibraryNoteListModel::BookmarkedRole).toBool());
        const QString title = viewModel.noteListModel()->data(index, LibraryNoteListModel::TitleRole).toString();
        const QString color = viewModel.noteListModel()
                                       ->data(index, LibraryNoteListModel::BookmarkColorRole)
                                       .toString();
        titleAndColorPairs.push_back(QStringLiteral("%1=%2").arg(title, color));
    }
    titleAndColorPairs.sort();
    QCOMPARE(
        titleAndColorPairs,
        QStringList({QStringLiteral("Blue Note=#3B82F6"), QStringLiteral("Pink Note=#EC4899")}));

    viewModel.setSelectedIndex(6); // blue
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::TitleRole)
                 .toString(),
        QStringLiteral("Blue Note"));

    viewModel.setSelectedIndex(8); // pink
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::TitleRole)
                 .toString(),
        QStringLiteral("Pink Note"));
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
    first.title = QStringLiteral("A");
    first.desc = QStringLiteral("A summary");
    items.push_back(first);
    LibraryNoteListItem second;
    second.id = QStringLiteral("note-b");
    second.title = QStringLiteral("B");
    second.desc = QStringLiteral("B summary");
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
    item.title = QStringLiteral("   ");
    item.desc = QStringLiteral(" ");
    item.folders = {QStringLiteral(""), QStringLiteral("folder-a"), QStringLiteral("folder-a")};
    item.bookmarked = true;
    item.bookmarkColor = QStringLiteral("not-a-hex");
    items.push_back(item);

    model.setItems(std::move(items));

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(model.data(index, LibraryNoteListModel::TitleRole).toString(), QString());
    QCOMPARE(model.data(index, LibraryNoteListModel::DescRole).toString(), QString());
    QCOMPARE(model.data(index, LibraryNoteListModel::FoldersRole).toStringList(),
             QStringList({QStringLiteral("folder-a")}));
    QCOMPARE(model.data(index, LibraryNoteListModel::BookmarkColorRole).toString(), QString());
    QVERIFY(model.correctionCount() >= 1);
}

void HierarchyViewModelsTest::noteListModel_limitsDescriptionToFiveLines()
{
    LibraryNoteListModel model;

    LibraryNoteListItem item;
    item.id = QStringLiteral("note-1");
    item.title = QStringLiteral("Title");
    item.desc = QStringLiteral("line1\nline2\nline3\nline4\nline5\nline6\nline7");

    model.setItems({item});

    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0, 0);
    QCOMPARE(
        model.data(index, LibraryNoteListModel::DescRole).toString(),
        QStringLiteral("line1\nline2\nline3\nline4\nline5"));
}

QTEST_APPLESS_MAIN(HierarchyViewModelsTest)

#include "test_hierarchy_viewmodels.moc"
