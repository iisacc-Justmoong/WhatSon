#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "file/hub/WhatSonHubStore.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class LibraryHierarchyViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaultState_isEmptyAndCreatable();
    void setDepthItems_parsesDepthAndDpethKeys();
    void setDepthItems_setsChevronOnlyWhenChildExists();
    void renameItem_updatesDisplayName();
    void createFolder_insertsAsChildOfSelectedSubtree();
    void createFolder_whenSystemBucketSelected_insertsAfterSystemFolders();
    void deleteSelectedFolder_removesDescendantSubtree();
    void loadFromWshub_buildsAllDraftTodayBuckets();
    void loadFromWshub_protectsAllDraftTodaySystemFolders();
    void loadFromWshub_populatesNoteListModelAndSwitchesBySelectedBucket();
    void loadFromWshub_noteListModel_exposesCurrentBodyTextFromWsnbody();
    void saveCurrentBodyText_rewritesWsnbodyAndPreservesLogicalLines();
    void loadFromWshub_filtersNoteListBySearchText_usingBodyPlainTextBeyondVisiblePreview();
    void loadFromWshub_usesBodyFirstLineForPrimaryText();
    void loadFromWshub_usesFoldersFileForSidebarItems();
    void loadFromWshub_filtersNoteListBySelectedFolder_whenFoldersHierarchyLoaded();
    void loadFromWshub_filtersNoteListByExactFolderPath_whenLeafLabelsRepeat();
    void loadFromWshub_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders();
    void deleteSelectedFolder_reappliesSelectionToVisibleNeighborForNoteList();
    void loadFromWshub_treatsUserFolderNamedAllAsRegularFolder();
    void loadFromWshub_readsDynamicWslibraryDirectory();
    void setDepthItems_emptyInput_preservesIndexedBuckets();
    void applyRuntimeSnapshot_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders();
    void assignNoteToFolder_updatesHeaderAndRefreshesDraftSelection();
    void moveFolder_reparentsSubtreeAndAllowsRootExtraction();
};

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
        const QString& createdAt,
        const QString& lastModifiedAt,
        const QStringList& folders,
        bool bookmarked = false,
        const QStringList& bookmarkColors = {},
        const QStringList& tags = {})
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"%1\">\n").arg(noteId);
        text += QStringLiteral("  <head>\n");
        text += QStringLiteral("    <created>%1</created>\n").arg(createdAt);
        text += QStringLiteral("    <lastModified>%1</lastModified>\n").arg(lastModifiedAt);
        text += QStringLiteral("    <folders>\n");
        for (const QString& folder : folders)
        {
            text += QStringLiteral("      <folder>%1</folder>\n").arg(folder);
        }
        text += QStringLiteral("    </folders>\n");
        QString bookmarkTag = QStringLiteral("    <bookmarks state=%1").arg(
            bookmarked ? QStringLiteral("true") : QStringLiteral("false"));
        if (!bookmarkColors.isEmpty())
        {
            bookmarkTag += QStringLiteral(" colors={%1}").arg(bookmarkColors.join(QStringLiteral(",")));
        }
        bookmarkTag += QStringLiteral(" />\n");
        text += bookmarkTag;
        text += QStringLiteral("    <tags>\n");
        for (const QString& tag : tags)
        {
            text += QStringLiteral("      <tag>%1</tag>\n").arg(tag);
        }
        text += QStringLiteral("    </tags>\n");
        text += QStringLiteral("  </head>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QString makeWsnBodyText(const QString& bodyInnerXml)
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

    bool prepareIndexedLibraryHub(
        QString* outHubPath,
        const QString& libraryDirectoryName = QStringLiteral("Library.wslibrary"),
        bool useBodyFirstLineForGamma = false)
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

        QString hubStem = libraryDirectoryName;
        hubStem.replace(QLatin1Char('/'), QLatin1Char('_'));
        const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("LibraryHub_%1.wshub").arg(hubStem));
        QDir(hubPath).removeRecursively();

        const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("LibraryHub_%1.wscontents").arg(hubStem));
        const QString libraryPath = QDir(contentsPath).filePath(libraryDirectoryName);
        const QString noteAPath = QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote"));
        const QString noteBPath = QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"));
        const QString noteCPath = QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote"));
        if (!QDir().mkpath(noteAPath) || !QDir().mkpath(noteBPath) || !QDir().mkpath(noteCPath))
        {
            return false;
        }

        const QString todayText = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral(
            "-12-00-00");
        const QString oldText = QStringLiteral("2024-01-01-00-00-00");

        QJsonArray notesArray;
        notesArray.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("note-a")},
            {QStringLiteral("lastModified"), oldText}
        });
        notesArray.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("note-b")},
            {QStringLiteral("lastModified"), todayText}
        });
        notesArray.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("note-c")},
            {QStringLiteral("lastModified"), oldText}
        });

        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QStringLiteral("whatson.library.index"));
        root.insert(QStringLiteral("notes"), notesArray);

        if (!writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteAPath).filePath(QStringLiteral("Alpha.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-a"),
                todayText,
                oldText,
                {})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteAPath).filePath(QStringLiteral("Alpha.wsnbody")),
            makeWsnBodyText(QStringLiteral("    <paragraph>Alpha body summary.</paragraph>\n"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteBPath).filePath(QStringLiteral("Beta.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-b"),
                oldText,
                todayText,
                {QStringLiteral("Workspace")},
                true,
                {QStringLiteral("blue")},
                {QStringLiteral("Only"), QStringLiteral("1 Line")})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteBPath).filePath(QStringLiteral("Beta.wsnbody")),
            makeWsnBodyText(QStringLiteral("    <paragraph>Beta <bold>body</bold> summary.</paragraph>\n"))))
        {
            return false;
        }

        if (!writeUtf8File(
            QDir(noteCPath).filePath(QStringLiteral("Gamma.wsnhead")),
            makeWsnHeadText(
                QStringLiteral("note-c"),
                oldText,
                oldText,
                {})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteCPath).filePath(QStringLiteral("Gamma.wsnbody")),
            useBodyFirstLineForGamma
                ? makeWsnBodyText(
                    QStringLiteral(
                        "    <Bold>Body Fallback Title</Bold>\n    <paragraph>Gamma body summary.</paragraph>\n"))
                : makeWsnBodyText(QStringLiteral("    <paragraph>Gamma body summary.</paragraph>\n"))))
        {
            return false;
        }

        *outHubPath = hubPath;
        return true;
    }
} // namespace

void LibraryHierarchyViewModelTest::defaultState_isEmptyAndCreatable()
{
    LibraryHierarchyViewModel viewModel;
    QCOMPARE(viewModel.itemModel()->rowCount(), 0);
    QCOMPARE(viewModel.selectedIndex(), -1);

    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
}

void LibraryHierarchyViewModelTest::setDepthItems_parsesDepthAndDpethKeys()
{
    LibraryHierarchyViewModel viewModel;

    const QVariantList depthArray{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("Child")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("GrandChild")},
            {"dpeth", 2}
        }
    };

    viewModel.setDepthItems(depthArray);

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::IndentLevelRole)
                 .toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        2);
}

void LibraryHierarchyViewModelTest::setDepthItems_setsChevronOnlyWhenChildExists()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("Child")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("Leaf")},
            {"depth", 0}
        }
    });

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ShowChevronRole).
                  toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::ShowChevronRole).
                  toBool(),
        false);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::ShowChevronRole).
                  toBool(),
        false);
}

void LibraryHierarchyViewModelTest::renameItem_updatesDisplayName()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Folder1")},
            {"depth", 0}
        }
    });

    QVERIFY(viewModel.renameItem(0, QStringLiteral("Renamed Folder")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Renamed Folder"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Renamed Folder"));

    QVERIFY(!viewModel.renameItem(0, QStringLiteral("   ")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Renamed Folder"));
}

void LibraryHierarchyViewModelTest::createFolder_insertsAsChildOfSelectedSubtree()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("RootChild")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("Sibling")},
            {"depth", 0}
        }
    });
    viewModel.setSelectedIndex(0);

    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    QCOMPARE(viewModel.selectedIndex(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
}

void LibraryHierarchyViewModelTest::createFolder_whenSystemBucketSelected_insertsAfterSystemFolders()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setSelectedIndex(0);
    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    QCOMPARE(viewModel.selectedIndex(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All Library"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
}

void LibraryHierarchyViewModelTest::deleteSelectedFolder_removesDescendantSubtree()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("Child")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("GrandChild")},
            {"depth", 2}
        },
        QVariantMap{
            {"label", QStringLiteral("Sibling")},
            {"depth", 0}
        }
    });
    viewModel.setSelectedIndex(0);

    viewModel.deleteSelectedFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Sibling"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_buildsAllDraftTodayBuckets()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All Library"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Draft"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Today"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
}

void LibraryHierarchyViewModelTest::loadFromWshub_protectsAllDraftTodaySystemFolders()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    for (int row = 0; row < 3; ++row)
    {
        QVERIFY(!viewModel.canRenameItem(row));
        viewModel.setSelectedIndex(row);
        QVERIFY(!viewModel.deleteFolderEnabled());
    }
}

void LibraryHierarchyViewModelTest::setDepthItems_emptyInput_preservesIndexedBuckets()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setDepthItems({});
    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All Library"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_populatesNoteListModelAndSwitchesBySelectedBucket()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::PrimaryTextRole)
                 .toString(),
        QStringLiteral("Alpha body summary."));
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::BookmarkedRole)
                 .toBool(),
        true);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::BookmarkColorRole)
                 .toString(),
        QStringLiteral("#3B82F6"));
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::DisplayDateRole)
                 .toString(),
        QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::TagsRole)
                 .toStringList(),
        QStringList({QStringLiteral("Only"), QStringLiteral("1 Line")}));

    viewModel.setSelectedIndex(1);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    viewModel.setSelectedIndex(0);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::PrimaryTextRole)
                 .toString(),
        QStringLiteral("Beta body summary."));
}

void LibraryHierarchyViewModelTest::loadFromWshub_noteListModel_exposesCurrentBodyTextFromWsnbody()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->currentIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-a"));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QStringLiteral("Alpha body summary."));

    viewModel.noteListModel()->setCurrentIndex(1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-b"));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QStringLiteral("Beta body summary."));
}

void LibraryHierarchyViewModelTest::saveCurrentBodyText_rewritesWsnbodyAndPreservesLogicalLines()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-a"));

    const QString editedBody = QStringLiteral("\nEdited first line\nEdited second line\n");
    QVERIFY(viewModel.saveCurrentBodyText(editedBody));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), editedBody);

    const QString bodyPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnbody"));
    const QString savedBodyXml = readUtf8File(bodyPath);
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph></paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Edited first line</paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Edited second line</paragraph>")));
}

void LibraryHierarchyViewModelTest::loadFromWshub_filtersNoteListBySearchText_usingBodyPlainTextBeyondVisiblePreview()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());

    const QString alphaBodyPath = QDir(hubDir.filePath(contentsDirs.first())).filePath(
        QStringLiteral("Library.wslibrary/Alpha.wsnote/Alpha.wsnbody"));
    QVERIFY(writeUtf8File(
        alphaBodyPath,
        makeWsnBodyText(
            QStringLiteral(
                "    <paragraph>line1</paragraph>\n"
                "    <paragraph>line2</paragraph>\n"
                "    <paragraph>line3</paragraph>\n"
                "    <paragraph>line4</paragraph>\n"
                "    <paragraph>line5</paragraph>\n"
                "    <paragraph>moonshot discovery</paragraph>\n"))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const QString alphaPrimaryText = viewModel.noteListModel()
                                              ->data(
                                                  viewModel.noteListModel()->index(0, 0),
                                                  LibraryNoteListModel::PrimaryTextRole)
                                              .toString();
    QVERIFY(!alphaPrimaryText.contains(QStringLiteral("moonshot discovery")));

    viewModel.noteListModel()->setSearchText(QStringLiteral("moonshot discovery"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.noteListModel()->setSearchText(QStringLiteral("   "));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

void LibraryHierarchyViewModelTest::loadFromWshub_usesBodyFirstLineForPrimaryText()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath, QStringLiteral("Library.wslibrary"), true));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    const QString gammaPrimaryText = viewModel.noteListModel()
                                              ->data(
                                                  viewModel.noteListModel()->index(2, 0),
                                                  LibraryNoteListModel::PrimaryTextRole)
                                              .toString();
    QVERIFY(gammaPrimaryText.startsWith(QStringLiteral("Body Fallback Title")));
    QVERIFY(
        gammaPrimaryText.contains(QStringLiteral("Gamma body summary.")));
}

void LibraryHierarchyViewModelTest::loadFromWshub_usesFoldersFileForSidebarItems()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());

    const QString foldersFilePath = QDir(hubDir.filePath(contentsDirs.first())).filePath(QStringLiteral(
        "Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Research/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Brand\",\n"
        "      \"label\": \"Brand\"\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 6);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All Library"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Draft"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Today"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(5, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::ShowChevronRole)
                 .toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::ShowChevronRole)
                 .toBool(),
        false);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

void LibraryHierarchyViewModelTest::loadFromWshub_filtersNoteListBySelectedFolder_whenFoldersHierarchyLoaded()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Research/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Brand\",\n"
        "      \"label\": \"Brand\"\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Research/Competitor")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Brand")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);

    auto findIndexByLabel = [&viewModel](const QString& label) -> int
    {
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            if (viewModel.itemModel()->data(viewModel.itemModel()->index(row, 0), LibraryHierarchyModel::LabelRole).
                          toString()
                == label)
            {
                return row;
            }
        }
        return -1;
    };

    const int researchIndex = findIndexByLabel(QStringLiteral("Research"));
    const int competitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    const int brandIndex = findIndexByLabel(QStringLiteral("Brand"));
    const int allIndex = findIndexByLabel(QStringLiteral("All Library"));
    QVERIFY(researchIndex >= 0);
    QVERIFY(competitorIndex >= 0);
    QVERIFY(brandIndex >= 0);
    QVERIFY(allIndex >= 0);

    viewModel.setSelectedIndex(allIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);

    viewModel.setSelectedIndex(researchIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::PrimaryTextRole).
                  toString(),
        QStringLiteral("Alpha body summary."));

    viewModel.setSelectedIndex(competitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::PrimaryTextRole).
                  toString(),
        QStringLiteral("Alpha body summary."));

    viewModel.setSelectedIndex(brandIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::PrimaryTextRole).
                  toString(),
        QStringLiteral("Beta body summary."));
}

void LibraryHierarchyViewModelTest::loadFromWshub_filtersNoteListByExactFolderPath_whenLeafLabelsRepeat()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Research/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Operations\",\n"
        "      \"label\": \"Operations\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Operations/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Research/Competitor")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Operations/Competitor")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    auto findNthIndexByLabel = [&viewModel](const QString& label, int occurrence) -> int
    {
        int seen = 0;
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            const QString rowLabel = viewModel.itemModel()
                                              ->data(viewModel.itemModel()->index(row, 0),
                                                     LibraryHierarchyModel::LabelRole)
                                              .toString();
            if (rowLabel != label)
            {
                continue;
            }
            ++seen;
            if (seen == occurrence)
            {
                return row;
            }
        }
        return -1;
    };

    const int researchIndex = findNthIndexByLabel(QStringLiteral("Research"), 1);
    const int operationsIndex = findNthIndexByLabel(QStringLiteral("Operations"), 1);
    const int firstCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 1);
    const int secondCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 2);
    QVERIFY(researchIndex >= 0);
    QVERIFY(operationsIndex >= 0);
    QVERIFY(firstCompetitorIndex >= 0);
    QVERIFY(secondCompetitorIndex >= 0);

    viewModel.setSelectedIndex(researchIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(operationsIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));

    viewModel.setSelectedIndex(firstCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
}

void LibraryHierarchyViewModelTest::
loadFromWshub_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Research/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Operations\",\n"
        "      \"label\": \"Operations\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Operations/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Research"), QStringLiteral("/Competitor")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Operations"), QStringLiteral("/Competitor")})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    auto findNthIndexByLabel = [&viewModel](const QString& label, int occurrence) -> int
    {
        int seen = 0;
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            const QString rowLabel = viewModel.itemModel()
                                              ->data(viewModel.itemModel()->index(row, 0),
                                                     LibraryHierarchyModel::LabelRole)
                                              .toString();
            if (rowLabel != label)
            {
                continue;
            }
            ++seen;
            if (seen == occurrence)
            {
                return row;
            }
        }
        return -1;
    };

    const int researchIndex = findNthIndexByLabel(QStringLiteral("Research"), 1);
    const int operationsIndex = findNthIndexByLabel(QStringLiteral("Operations"), 1);
    const int firstCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 1);
    const int secondCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 2);
    QVERIFY(researchIndex >= 0);
    QVERIFY(operationsIndex >= 0);
    QVERIFY(firstCompetitorIndex >= 0);
    QVERIFY(secondCompetitorIndex >= 0);

    viewModel.setSelectedIndex(researchIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(operationsIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));

    viewModel.setSelectedIndex(firstCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
}

void LibraryHierarchyViewModelTest::deleteSelectedFolder_reappliesSelectionToVisibleNeighborForNoteList()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\"\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Brand\",\n"
        "      \"label\": \"Brand\"\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Brand")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    auto findIndexByLabel = [&viewModel](const QString& label) -> int
    {
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            if (viewModel.itemModel()->data(viewModel.itemModel()->index(row, 0), LibraryHierarchyModel::LabelRole).
                          toString()
                == label)
            {
                return row;
            }
        }
        return -1;
    };

    const int researchIndex = findIndexByLabel(QStringLiteral("Research"));
    QVERIFY(researchIndex >= 0);

    viewModel.setSelectedIndex(researchIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.deleteSelectedFolder();

    QCOMPARE(viewModel.selectedIndex(), researchIndex);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(researchIndex, 0), LibraryHierarchyModel::LabelRole)
                 .toString(),
        QStringLiteral("Brand"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_treatsUserFolderNamedAllAsRegularFolder()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"All\",\n"
        "      \"label\": \"All\"\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Brand\",\n"
        "      \"label\": \"Brand\"\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("All")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Brand")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    auto findNthIndexByLabel = [&viewModel](const QString& label, int occurrence) -> int
    {
        int seen = 0;
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            const QString rowLabel = viewModel.itemModel()
                                              ->data(viewModel.itemModel()->index(row, 0),
                                                     LibraryHierarchyModel::LabelRole)
                                              .toString();
            if (rowLabel != label)
            {
                continue;
            }
            ++seen;
            if (seen == occurrence)
            {
                return row;
            }
        }
        return -1;
    };

    const int allLibraryIndex = findNthIndexByLabel(QStringLiteral("All Library"), 1);
    const int userAllIndex = findNthIndexByLabel(QStringLiteral("All"), 1);
    QVERIFY(allLibraryIndex >= 0);
    QVERIFY(userAllIndex >= 0);
    QVERIFY(allLibraryIndex != userAllIndex);
    QVERIFY(viewModel.canRenameItem(userAllIndex));

    viewModel.setSelectedIndex(userAllIndex);
    QVERIFY(viewModel.deleteFolderEnabled());
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(allLibraryIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

void LibraryHierarchyViewModelTest::loadFromWshub_readsDynamicWslibraryDirectory()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath, QStringLiteral("Workspace.wslibrary")));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

void LibraryHierarchyViewModelTest::
applyRuntimeSnapshot_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders()
{
    LibraryHierarchyViewModel viewModel;

    LibraryNoteRecord researchNote;
    researchNote.noteId = QStringLiteral("note-a");
    researchNote.bodyPlainText = QStringLiteral("Alpha body summary.");
    researchNote.bodyFirstLine = QStringLiteral("Alpha body summary.");
    researchNote.folders = {QStringLiteral("Research"), QStringLiteral("/Competitor")};

    LibraryNoteRecord operationsNote;
    operationsNote.noteId = QStringLiteral("note-b");
    operationsNote.bodyPlainText = QStringLiteral("Beta body summary.");
    operationsNote.bodyFirstLine = QStringLiteral("Beta body summary.");
    operationsNote.folders = {QStringLiteral("Operations"), QStringLiteral("/Competitor")};

    QVector<WhatSonFolderDepthEntry> folderEntries;
    folderEntries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("Research"),
        QStringLiteral("Research"),
        0
    });
    folderEntries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("Research/Competitor"),
        QStringLiteral("Competitor"),
        1
    });
    folderEntries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("Operations"),
        QStringLiteral("Operations"),
        0
    });
    folderEntries.push_back(WhatSonFolderDepthEntry{
        QStringLiteral("Operations/Competitor"),
        QStringLiteral("Competitor"),
        1
    });

    viewModel.applyRuntimeSnapshot(
        QStringLiteral("/tmp/TestRuntime.wshub"),
        {researchNote, operationsNote},
        {},
        {},
        folderEntries,
        QStringLiteral("/tmp/Folders.wsfolders"),
        true,
        {});

    auto findNthIndexByLabel = [&viewModel](const QString& label, int occurrence) -> int
    {
        int seen = 0;
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            const QString rowLabel = viewModel.itemModel()
                                              ->data(viewModel.itemModel()->index(row, 0),
                                                     LibraryHierarchyModel::LabelRole)
                                              .toString();
            if (rowLabel != label)
            {
                continue;
            }
            ++seen;
            if (seen == occurrence)
            {
                return row;
            }
        }
        return -1;
    };

    const int firstCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 1);
    const int secondCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 2);
    QVERIFY(firstCompetitorIndex >= 0);
    QVERIFY(secondCompetitorIndex >= 0);

    viewModel.setSelectedIndex(firstCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
}

void LibraryHierarchyViewModelTest::assignNoteToFolder_updatesHeaderAndRefreshesDraftSelection()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QDir hubDir(hubPath);
    const QStringList contentsDirs = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    QVERIFY(!contentsDirs.isEmpty());
    const QString contentsPath = hubDir.filePath(contentsDirs.first());

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Research\",\n"
        "      \"label\": \"Research\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Research/Competitor\",\n"
        "          \"label\": \"Competitor\"\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    auto findIndexByLabel = [&viewModel](const QString& label) -> int
    {
        for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
        {
            if (viewModel.itemModel()->data(viewModel.itemModel()->index(row, 0), LibraryHierarchyModel::LabelRole).
                          toString()
                == label)
            {
                return row;
            }
        }
        return -1;
    };

    const int draftIndex = findIndexByLabel(QStringLiteral("Draft"));
    const int competitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    QVERIFY(draftIndex >= 0);
    QVERIFY(competitorIndex >= 0);

    viewModel.setSelectedIndex(draftIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    QVERIFY(viewModel.canAcceptNoteDrop(competitorIndex, QStringLiteral("note-a")));
    QVERIFY(!viewModel.canAcceptNoteDrop(draftIndex, QStringLiteral("note-a")));

    QVERIFY(viewModel.assignNoteToFolder(competitorIndex, QStringLiteral("note-a")));
    QVERIFY(!viewModel.canAcceptNoteDrop(competitorIndex, QStringLiteral("note-a")));
    QVERIFY(!viewModel.assignNoteToFolder(competitorIndex, QStringLiteral("note-a")));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::IdRole).
                  toString(),
        QStringLiteral("note-c"));

    QFile alphaHeaderFile(QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary/Alpha.wsnote/Alpha.wsnhead")));
    QVERIFY(alphaHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(alphaHeaderFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QCOMPARE(headerStore.folders(), QStringList({QStringLiteral("Research/Competitor")}));
    QVERIFY(headerStore.lastModifiedAt().startsWith(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))));

    viewModel.setSelectedIndex(competitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::IdRole).
                  toString(),
        QStringLiteral("note-a"));
}

void LibraryHierarchyViewModelTest::moveFolder_reparentsSubtreeAndAllowsRootExtraction()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("RootA")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("ChildA")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("RootB")},
            {"depth", 0}
        }
    });

    QVERIFY(viewModel.canMoveFolder(0));
    QVERIFY(!viewModel.canAcceptFolderDrop(0, 1, true));
    QVERIFY(viewModel.canAcceptFolderDrop(2, 0, true));
    QVERIFY(viewModel.moveFolder(2, 0, true));

    QVariantList movedAsChild = viewModel.depthItems();
    QCOMPARE(movedAsChild.size(), 3);
    QCOMPARE(movedAsChild.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootA"));
    QCOMPARE(movedAsChild.at(0).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA"));
    QCOMPARE(movedAsChild.at(1).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("ChildA"));
    QCOMPARE(movedAsChild.at(1).toMap().value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(movedAsChild.at(1).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA/ChildA"));
    QCOMPARE(movedAsChild.at(2).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootB"));
    QCOMPARE(movedAsChild.at(2).toMap().value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(movedAsChild.at(2).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA/RootB"));

    QVERIFY(viewModel.canMoveFolderToRoot(1));
    QVERIFY(viewModel.moveFolderToRoot(1));

    const QVariantList movedToRoot = viewModel.depthItems();
    QCOMPARE(movedToRoot.size(), 3);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(movedToRoot.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("ChildA"));
    QCOMPARE(movedToRoot.at(0).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedToRoot.at(0).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("ChildA"));
    QCOMPARE(movedToRoot.at(1).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootA"));
    QCOMPARE(movedToRoot.at(1).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedToRoot.at(1).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA"));
    QCOMPARE(movedToRoot.at(2).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootB"));
    QCOMPARE(movedToRoot.at(2).toMap().value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(movedToRoot.at(2).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA/RootB"));
}

QTEST_APPLESS_MAIN(LibraryHierarchyViewModelTest)

#include "test_library_hierarchy_view_model.moc"
