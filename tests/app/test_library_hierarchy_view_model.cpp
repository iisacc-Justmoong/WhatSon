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
#include <QRegularExpression>
#include <QUrl>
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
    void createFolder_preservesExpandedParentWhenExpansionCameFromUi();
    void createFolder_expandsCollapsedParentToRevealNewChild();
    void deleteSelectedFolder_removesDescendantSubtree();
    void loadFromWshub_buildsAllDraftTodayBuckets();
    void loadFromWshub_protectsAllDraftTodaySystemFolders();
    void loadFromWshub_populatesNoteListModelAndSwitchesBySelectedBucket();
    void loadFromWshub_noteListModel_exposesCurrentBodyTextFromWsnbody();
    void loadFromWshub_noteListModel_exposesImagePreviewFromWsnbodyResource();
    void applyRuntimeSnapshot_blankBody_keepsPreviewEmptyAndSearchIgnoresInternalId();
    void saveCurrentBodyText_rewritesWsnbodyAndPreservesLogicalLines();
    void saveCurrentBodyText_unchangedPlainText_preservesExistingBodyMarkup();
    void loadFromWshub_filtersNoteListBySearchText_usingBodyPlainTextBeyondVisiblePreview();
    void loadFromWshub_usesBodyFirstLineForPrimaryText();
    void loadFromWshub_usesFoldersFileForSidebarItems();
    void loadFromWshub_filtersNoteListBySelectedFolder_whenFoldersHierarchyLoaded();
    void loadFromWshub_filtersNoteListByExactFolderPath_whenLeafLabelsRepeat();
    void loadFromWshub_doesNotFanOutAmbiguousLeafFolder_whenLeafLabelsRepeat();
    void loadFromWshub_filtersOnlyExactNestedFolder_whenLeafLabelsRepeatAlongOnePath();
    void loadFromWshub_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders();
    void deleteSelectedFolder_reappliesSelectionToVisibleNeighborForNoteList();
    void deleteNoteById_removesScaffoldUpdatesIndexAndSelectsNeighbor();
    void clearNoteFoldersById_rewritesHeaderAndRefreshesVisibleFolders();
    void loadFromWshub_prunesOrphanIndexEntriesAndRewritesIndex();
    void loadFromWshub_treatsUserFolderNamedAllAsRegularFolder();
    void loadFromWshub_treatsTodayAsReservedSmartFolderInsteadOfConcreteFolder();
    void loadFromWshub_draftBucket_requiresEmptyRawHeaderFoldersBlock();
    void loadFromWshub_readsDynamicWslibraryDirectory();
    void setDepthItems_emptyInput_preservesIndexedBuckets();
    void applyRuntimeSnapshot_resolvesNestedFolderSelection_whenHeadersStoreAncestorAndLeafFolders();
    void assignNoteToFolder_updatesHeaderAndRefreshesDraftSelection();
    void assignNoteToFolder_preservesExistingFolderValuesAndAppendsDroppedTarget();
    void createEmptyNote_whenFolderSelected_createsScaffoldUpdatesIndexAndSelectsNote();
    void loadFromWshub_moveFolderBefore_rewritesFoldersFileAndHeaderAssignments();
    void loadFromWshub_applyHierarchyNodes_persistsLvrsEditableMove();
    void loadFromWshub_applyHierarchyNodes_preservesSystemBucketPrefix();
    void loadFromWshub_applyHierarchyNodes_preservesDraggedFolderBetweenSystemBuckets_withoutExplicitSourceIndex();
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

    QJsonObject readJsonObjectFile(const QString& filePath)
    {
        const QJsonDocument document = QJsonDocument::fromJson(readUtf8File(filePath).toUtf8());
        if (!document.isObject())
        {
            return {};
        }
        return document.object();
    }

    QStringList noteIdsFromIndexArray(const QJsonArray& notesArray)
    {
        QStringList noteIds;
        noteIds.reserve(notesArray.size());
        for (const QJsonValue& value : notesArray)
        {
            if (value.isString())
            {
                noteIds.push_back(value.toString());
                continue;
            }
            if (value.isObject())
            {
                noteIds.push_back(value.toObject().value(QStringLiteral("id")).toString());
            }
        }
        return noteIds;
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

void LibraryHierarchyViewModelTest::createFolder_preservesExpandedParentWhenExpansionCameFromUi()
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

    QVERIFY(viewModel.setItemExpanded(0, true));
    viewModel.setSelectedIndex(0);

    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    QCOMPARE(viewModel.selectedIndex(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ExpandedRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Sibling"));
}

void LibraryHierarchyViewModelTest::createFolder_expandsCollapsedParentToRevealNewChild()
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

    QVERIFY(
        !viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ExpandedRole).toBool());

    viewModel.createFolder();

    QCOMPARE(viewModel.selectedIndex(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::ExpandedRole).toBool(),
        true);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Untitled"));
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

    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    QCOMPARE(hierarchyModel.size(), 3);
    for (int row = 0; row < 3; ++row)
    {
        const QVariantMap node = hierarchyModel.at(row).toMap();
        QCOMPARE(node.value(QStringLiteral("draggable")).toBool(), false);
        QCOMPARE(node.value(QStringLiteral("dragAllowed")).toBool(), false);
        QCOMPARE(node.value(QStringLiteral("movable")).toBool(), false);
        QCOMPARE(node.value(QStringLiteral("dragLocked")).toBool(), true);
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
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Draft")}));
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
    for (int row = 0; row < viewModel.noteListModel()->rowCount(); ++row)
    {
        QCOMPARE(
            viewModel.noteListModel()->data(
                         viewModel.noteListModel()->index(row, 0),
                         LibraryNoteListModel::FoldersRole)
                     .toStringList(),
            QStringList({QStringLiteral("Draft")}));
    }

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

    QCOMPARE(viewModel.noteListModel()->currentIndex(), -1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QString());
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QString());

    viewModel.noteListModel()->setCurrentIndex(0);
    QCOMPARE(viewModel.noteListModel()->currentIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-a"));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QStringLiteral("Alpha body summary."));
    viewModel.noteListModel()->setCurrentIndex(1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-b"));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QStringLiteral("Beta body summary."));
}

void LibraryHierarchyViewModelTest::loadFromWshub_noteListModel_exposesImagePreviewFromWsnbodyResource()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QString resourceDirectoryName = QStringLiteral("LibraryHub_Library.wslibrary.wsresources");
    const QString resourceDirectoryPath = QDir(hubPath).filePath(resourceDirectoryName);
    QVERIFY(QDir().mkpath(resourceDirectoryPath));
    const QString resourceFilePath = QDir(resourceDirectoryPath).filePath(QStringLiteral("beta-preview.png"));
    QVERIFY(writeUtf8File(resourceFilePath, QStringLiteral("png")));

    const QString bodyPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Beta.wsnote/Beta.wsnbody"));
    QVERIFY(writeUtf8File(
        bodyPath,
        makeWsnBodyText(
            QStringLiteral(
                "    <resource format=\".png\" resourcePath=\"LibraryHub_Library.wslibrary.wsresources/beta-preview.png\" />\n"
                "    <paragraph>Beta body summary.</paragraph>\n"))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::ImageRole)
                 .toBool(),
        true);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::ImageSourceRole)
                 .toString(),
        QUrl::fromLocalFile(resourceFilePath).toString());
}

void LibraryHierarchyViewModelTest::applyRuntimeSnapshot_blankBody_keepsPreviewEmptyAndSearchIgnoresInternalId()
{
    LibraryHierarchyViewModel viewModel;

    LibraryNoteRecord blankNote;
    blankNote.noteId = QStringLiteral("internal-note-id");
    blankNote.bodyPlainText = QString();
    blankNote.bodyFirstLine = QString();
    blankNote.createdAt = QStringLiteral("2026-03-01-00-00-00");
    blankNote.lastModifiedAt = QStringLiteral("2026-03-01-00-00-00");

    viewModel.applyRuntimeSnapshot(
        QStringLiteral("/tmp/TestHub.wshub"),
        {blankNote},
        {},
        {},
        {},
        QString(),
        true);

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::PrimaryTextRole)
                 .toString(),
        QString());
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Draft")}));

    viewModel.noteListModel()->setSearchText(QStringLiteral("internal-note-id"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);
    viewModel.noteListModel()->setSearchText(QStringLiteral("Draft"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
}

void LibraryHierarchyViewModelTest::saveCurrentBodyText_rewritesWsnbodyAndPreservesLogicalLines()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->currentIndex(), -1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QString());
    viewModel.noteListModel()->setCurrentIndex(0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-a"));

    const QString editedBody = QStringLiteral("\nEdited first line\nEdited second line\n");
    QVERIFY(viewModel.saveBodyTextForNote(QStringLiteral("note-a"), editedBody));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), editedBody);
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::PrimaryTextRole)
                 .toString(),
        QStringLiteral("Edited first line\nEdited second line"));

    const QString bodyPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnbody"));
    const QString historyPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnhistory"));
    const QString versionPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnversion"));
    const QString savedBodyXml = readUtf8File(bodyPath);
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph></paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Edited first line</paragraph>")));
    QVERIFY(savedBodyXml.contains(QStringLiteral("<paragraph>Edited second line</paragraph>")));
    QVERIFY(QFileInfo(versionPath).isFile());
    QCOMPARE(readJsonObjectFile(versionPath).value(QStringLiteral("snapshots")).toArray().size(), 0);

    const QStringList historyLines = readUtf8File(historyPath).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(historyLines.size(), 1);
    const QJsonObject historyEntry = QJsonDocument::fromJson(historyLines.constFirst().toUtf8()).object();
    QCOMPARE(historyEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("note-a"));
    QCOMPARE(historyEntry.value(QStringLiteral("removedText")).toString(), QStringLiteral("Alpha body summary."));
    QCOMPARE(historyEntry.value(QStringLiteral("insertedText")).toString(), editedBody);
}

void LibraryHierarchyViewModelTest::saveCurrentBodyText_unchangedPlainText_preservesExistingBodyMarkup()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    const QString bodyPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnbody"));
    const QString headerPath = QDir(hubPath).filePath(
        QStringLiteral("LibraryHub_Library.wslibrary.wscontents/Library.wslibrary/Alpha.wsnote/Alpha.wsnhead"));
    const QString originalBodyXml = makeWsnBodyText(
        QStringLiteral(
            "    <paragraph>Alpha body summary.</paragraph>\n"
            "    <Anchor></Anchor>\n"));
    QVERIFY(writeUtf8File(bodyPath, originalBodyXml));
    const QString originalHeaderXml = readUtf8File(headerPath);

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.noteListModel()->setCurrentIndex(0);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-a"));
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QStringLiteral("Alpha body summary."));

    QVERIFY(viewModel.saveBodyTextForNote(QStringLiteral("note-a"), viewModel.noteListModel()->currentBodyText()));

    QCOMPARE(readUtf8File(bodyPath), originalBodyXml);
    QCOMPARE(readUtf8File(headerPath), originalHeaderXml);
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
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::ItemKeyRole).toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::ItemKeyRole).toString(),
        QStringLiteral("Research/Competitor"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(5, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(5, 0), LibraryHierarchyModel::ItemKeyRole).toString(),
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

    const QVariantList hierarchyModel = viewModel.hierarchyModel();
    QCOMPARE(hierarchyModel.size(), 6);

    const QVariantMap allBucketNode = hierarchyModel.at(0).toMap();
    QCOMPARE(allBucketNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:all"));
    QCOMPARE(allBucketNode.value(QStringLiteral("draggable")).toBool(), false);
    QCOMPARE(allBucketNode.value(QStringLiteral("dragAllowed")).toBool(), false);
    QCOMPARE(allBucketNode.value(QStringLiteral("movable")).toBool(), false);
    QCOMPARE(allBucketNode.value(QStringLiteral("dragLocked")).toBool(), true);

    const QVariantMap draftBucketNode = hierarchyModel.at(1).toMap();
    QCOMPARE(draftBucketNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:draft"));
    QCOMPARE(draftBucketNode.value(QStringLiteral("draggable")).toBool(), false);
    QCOMPARE(draftBucketNode.value(QStringLiteral("dragAllowed")).toBool(), false);
    QCOMPARE(draftBucketNode.value(QStringLiteral("movable")).toBool(), false);
    QCOMPARE(draftBucketNode.value(QStringLiteral("dragLocked")).toBool(), true);

    const QVariantMap todayBucketNode = hierarchyModel.at(2).toMap();
    QCOMPARE(todayBucketNode.value(QStringLiteral("key")).toString(), QStringLiteral("bucket:today"));
    QCOMPARE(todayBucketNode.value(QStringLiteral("draggable")).toBool(), false);
    QCOMPARE(todayBucketNode.value(QStringLiteral("dragAllowed")).toBool(), false);
    QCOMPARE(todayBucketNode.value(QStringLiteral("movable")).toBool(), false);
    QCOMPARE(todayBucketNode.value(QStringLiteral("dragLocked")).toBool(), true);

    const QVariantMap researchNode = hierarchyModel.at(3).toMap();
    QCOMPARE(researchNode.value(QStringLiteral("itemId")).toInt(), 3);
    QCOMPARE(researchNode.value(QStringLiteral("key")).toString(), QStringLiteral("Research"));
    QCOMPARE(researchNode.value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(researchNode.value(QStringLiteral("draggable")).toBool(), true);
    QCOMPARE(researchNode.value(QStringLiteral("dragAllowed")).toBool(), true);
    QCOMPARE(researchNode.value(QStringLiteral("movable")).toBool(), true);
    QCOMPARE(researchNode.value(QStringLiteral("dragLocked")).toBool(), false);
    QCOMPARE(researchNode.value(QStringLiteral("showChevron")).toBool(), true);
    QVERIFY(!researchNode.contains(QStringLiteral("children")));

    const QVariantMap competitorNode = hierarchyModel.at(4).toMap();
    QCOMPARE(competitorNode.value(QStringLiteral("itemId")).toInt(), 4);
    QCOMPARE(competitorNode.value(QStringLiteral("key")).toString(),
             QStringLiteral("Research/Competitor"));
    QCOMPARE(competitorNode.value(QStringLiteral("depth")).toInt(), 1);
    QCOMPARE(competitorNode.value(QStringLiteral("showChevron")).toBool(), false);
    QVERIFY(!competitorNode.contains(QStringLiteral("children")));

    const QVariantMap brandNode = hierarchyModel.at(5).toMap();
    QCOMPARE(brandNode.value(QStringLiteral("key")).toString(), QStringLiteral("Brand"));
    QVERIFY(!brandNode.contains(QStringLiteral("children")));

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
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

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
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(operationsIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

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

void LibraryHierarchyViewModelTest::loadFromWshub_doesNotFanOutAmbiguousLeafFolder_whenLeafLabelsRepeat()
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
            {QStringLiteral("Competitor")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Operations/Competitor")})));

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

    const int firstCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 1);
    const int secondCompetitorIndex = findNthIndexByLabel(QStringLiteral("Competitor"), 2);
    QVERIFY(firstCompetitorIndex >= 0);
    QVERIFY(secondCompetitorIndex >= 0);

    viewModel.setSelectedIndex(firstCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Operations/Competitor")}));
}

void LibraryHierarchyViewModelTest::loadFromWshub_filtersOnlyExactNestedFolder_whenLeafLabelsRepeatAlongOnePath()
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
        "      \"id\": \"Untitled\",\n"
        "      \"label\": \"Untitled\",\n"
        "      \"children\": [\n"
        "        {\n"
        "          \"id\": \"Untitled/Untitled\",\n"
        "          \"label\": \"Untitled\",\n"
        "          \"children\": [\n"
        "            {\n"
        "              \"id\": \"Untitled/Untitled/Untitled\",\n"
        "              \"label\": \"Untitled\"\n"
        "            }\n"
        "          ]\n"
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
            {QStringLiteral("Untitled/Untitled/Untitled")})));

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

    const int firstUntitledIndex = findNthIndexByLabel(QStringLiteral("Untitled"), 1);
    const int secondUntitledIndex = findNthIndexByLabel(QStringLiteral("Untitled"), 2);
    const int thirdUntitledIndex = findNthIndexByLabel(QStringLiteral("Untitled"), 3);
    QVERIFY(firstUntitledIndex >= 0);
    QVERIFY(secondUntitledIndex >= 0);
    QVERIFY(thirdUntitledIndex >= 0);

    viewModel.setSelectedIndex(firstUntitledIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(secondUntitledIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(thirdUntitledIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Untitled/Untitled/Untitled")}));
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
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(operationsIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    viewModel.setSelectedIndex(firstCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-a"));
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Research/Competitor")}));

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Operations/Competitor")}));
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

void LibraryHierarchyViewModelTest::loadFromWshub_treatsTodayAsReservedSmartFolderInsteadOfConcreteFolder()
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
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));

    const QString todayText = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral(
        "-12-00-00");
    const QString oldText = QStringLiteral("2024-01-01-00-00-00");

    const QString foldersJson = QStringLiteral(
        "{\n"
        "  \"version\": 1,\n"
        "  \"schema\": \"whatson.folders.tree\",\n"
        "  \"folders\": [\n"
        "    {\n"
        "      \"id\": \"Today\",\n"
        "      \"label\": \"Today\"\n"
        "    },\n"
        "    {\n"
        "      \"id\": \"Brand\",\n"
        "      \"label\": \"Brand\"\n"
        "    }\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-a"),
            oldText,
            oldText,
            {QStringLiteral("today")})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            oldText,
            todayText,
            {})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            oldText,
            oldText,
            {QStringLiteral("Brand")})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    int todayLabelCount = 0;
    int systemTodayIndex = -1;
    int draftIndex = -1;
    int brandIndex = -1;
    for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
    {
        const QString label = viewModel.itemModel()
                                  ->data(viewModel.itemModel()->index(row, 0), LibraryHierarchyModel::LabelRole)
                                  .toString();
        if (label == QStringLiteral("Today"))
        {
            ++todayLabelCount;
            if (systemTodayIndex < 0)
            {
                systemTodayIndex = row;
            }
        }
        else if (label == QStringLiteral("Draft"))
        {
            draftIndex = row;
        }
        else if (label == QStringLiteral("Brand"))
        {
            brandIndex = row;
        }
    }

    QCOMPARE(todayLabelCount, 1);
    QVERIFY(systemTodayIndex >= 0);
    QVERIFY(draftIndex >= 0);
    QVERIFY(brandIndex >= 0);

    viewModel.setSelectedIndex(systemTodayIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));

    viewModel.setSelectedIndex(draftIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));

    viewModel.setSelectedIndex(brandIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-c"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_draftBucket_requiresEmptyRawHeaderFoldersBlock()
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
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));

    const QString oldText = QStringLiteral("2024-01-01-00-00-00");

    const QString malformedDraftHeader =
        QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE WHATSONNOTE>\n"
            "<contents id=\"note-a\">\n"
            "  <head>\n"
            "    <created>%1</created>\n"
            "    <lastModified>%1</lastModified>\n"
            "    <folders>Draft</folders>\n"
            "  </head>\n"
            "</contents>\n").arg(oldText);

    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")),
        malformedDraftHeader));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            oldText,
            oldText,
            {})));
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Gamma.wsnote/Gamma.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-c"),
            oldText,
            oldText,
            {QStringLiteral("Brand")})));

    QJsonArray notesArray;
    notesArray.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("note-a")},
        {QStringLiteral("lastModified"), oldText}
    });
    notesArray.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("note-b")},
        {QStringLiteral("lastModified"), oldText},
        {QStringLiteral("folders"), QJsonArray{QStringLiteral("Workspace")}}
    });
    notesArray.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("note-c")},
        {QStringLiteral("lastModified"), oldText},
        {QStringLiteral("folders"), QJsonArray{QStringLiteral("Brand")}}
    });

    QJsonObject indexRoot;
    indexRoot.insert(QStringLiteral("version"), 1);
    indexRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.library.index"));
    indexRoot.insert(QStringLiteral("notes"), notesArray);
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
        QString::fromUtf8(QJsonDocument(indexRoot).toJson(QJsonDocument::Indented))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    int draftIndex = -1;
    for (int row = 0; row < viewModel.itemModel()->rowCount(); ++row)
    {
        const QString label = viewModel.itemModel()
                                  ->data(viewModel.itemModel()->index(row, 0), LibraryHierarchyModel::LabelRole)
                                  .toString();
        if (label == QStringLiteral("Draft"))
        {
            draftIndex = row;
            break;
        }
    }

    QVERIFY(draftIndex >= 0);
    viewModel.setSelectedIndex(draftIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
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
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Research/Competitor")}));

    viewModel.setSelectedIndex(secondCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole)
                 .toString(),
        QStringLiteral("note-b"));
    QCOMPARE(
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::FoldersRole)
                 .toStringList(),
        QStringList({QStringLiteral("Operations/Competitor")}));
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
    QVERIFY(viewModel.assignNoteToFolder(competitorIndex, QStringLiteral("note-a")));

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

void LibraryHierarchyViewModelTest::assignNoteToFolder_preservesExistingFolderValuesAndAppendsDroppedTarget()
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

    const int competitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    QVERIFY(competitorIndex >= 0);
    QVERIFY(viewModel.canAcceptNoteDrop(competitorIndex, QStringLiteral("note-b")));

    QVERIFY(viewModel.assignNoteToFolder(competitorIndex, QStringLiteral("note-b")));
    QVERIFY(!viewModel.canAcceptNoteDrop(competitorIndex, QStringLiteral("note-b")));
    QVERIFY(viewModel.assignNoteToFolder(competitorIndex, QStringLiteral("note-b")));

    QFile betaHeaderFile(QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary/Beta.wsnote/Beta.wsnhead")));
    QVERIFY(betaHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(betaHeaderFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QCOMPARE(
        headerStore.folders(),
        QStringList({QStringLiteral("Workspace"), QStringLiteral("Research/Competitor")}));

    viewModel.setSelectedIndex(competitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::IdRole).
                  toString(),
        QStringLiteral("note-b"));
}

void LibraryHierarchyViewModelTest::createEmptyNote_whenFolderSelected_createsScaffoldUpdatesIndexAndSelectsNote()
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
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString statFilePath = QDir(hubPath).filePath(QStringLiteral("LibraryHubStat.wsstat"));

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
    QVERIFY(writeUtf8File(
        QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote/Beta.wsnhead")),
        makeWsnHeadText(
            QStringLiteral("note-b"),
            QStringLiteral("2024-01-01-00-00-00"),
            QStringLiteral("2024-01-01-00-00-00"),
            {QStringLiteral("Brand")})));

    QJsonObject statRoot;
    statRoot.insert(QStringLiteral("version"), 1);
    statRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.stat"));
    statRoot.insert(QStringLiteral("hub"), QStringLiteral("LibraryHub"));
    statRoot.insert(QStringLiteral("noteCount"), 3);
    statRoot.insert(QStringLiteral("resourceCount"), 0);
    statRoot.insert(QStringLiteral("characterCount"), 0);
    statRoot.insert(QStringLiteral("createdAtUtc"), QStringLiteral("2026-03-01T00:00:00Z"));
    statRoot.insert(QStringLiteral("lastModifiedAtUtc"), QStringLiteral("2026-03-01T00:00:00Z"));
    statRoot.insert(QStringLiteral("participants"), QJsonArray{QStringLiteral("ProfileName")});
    statRoot.insert(QStringLiteral("profileLastModifiedAtUtc"), QJsonObject{});
    QVERIFY(writeUtf8File(
        statFilePath,
        QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    WhatSonHubStore hubStore;
    hubStore.setHubPath(hubPath);
    hubStore.setHubName(QStringLiteral("LibraryHub"));
    hubStore.setContentsPath(contentsPath);
    hubStore.setLibraryPath(libraryPath);
    hubStore.setStatPath(statFilePath);
    WhatSonHubStat hubStat;
    hubStat.setNoteCount(3);
    hubStat.setResourceCount(0);
    hubStat.setCharacterCount(0);
    hubStat.setCreatedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setLastModifiedAtUtc(QStringLiteral("2026-03-01T00:00:00Z"));
    hubStat.setParticipants(QStringList{QStringLiteral("ProfileName")});
    hubStore.setStat(hubStat);
    viewModel.setHubStore(hubStore);

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

    const int brandIndex = findIndexByLabel(QStringLiteral("Brand"));
    QVERIFY(brandIndex >= 0);
    viewModel.setSelectedIndex(brandIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);

    QVERIFY(viewModel.createEmptyNote());

    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    const QString createdNoteId = viewModel.noteListModel()->currentNoteId();
    QVERIFY(!createdNoteId.isEmpty());
    QVERIFY(QRegularExpression(QStringLiteral("^[A-Za-z0-9]{16}-[A-Za-z0-9]{16}$")).match(createdNoteId).
        hasMatch());
    QCOMPARE(viewModel.noteListModel()->currentBodyText(), QString());
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(viewModel.noteListModel()->currentIndex(), 0),
                      LibraryNoteListModel::PrimaryTextRole).
                  toString(),
        QString());

    const QString noteDirectoryPath = QDir(libraryPath).filePath(createdNoteId + QStringLiteral(".wsnote"));
    const QString noteHeaderPath = QDir(noteDirectoryPath).filePath(createdNoteId + QStringLiteral(".wsnhead"));
    const QString noteBodyPath = QDir(noteDirectoryPath).filePath(createdNoteId + QStringLiteral(".wsnbody"));
    const QString noteHistoryPath = QDir(noteDirectoryPath).filePath(createdNoteId + QStringLiteral(".wsnhistory"));
    const QString noteVersionPath = QDir(noteDirectoryPath).filePath(createdNoteId + QStringLiteral(".wsnversion"));
    QVERIFY(QFileInfo(noteDirectoryPath).isDir());
    QVERIFY(QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral(".meta"))).isDir());
    QVERIFY(QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral("attachments"))).isDir());
    QVERIFY(QFileInfo(noteHeaderPath).isFile());
    QVERIFY(QFileInfo(noteBodyPath).isFile());
    QVERIFY(QFileInfo(noteHistoryPath).isFile());
    QVERIFY(QFileInfo(noteVersionPath).isFile());
    QVERIFY(QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral("attachments.wsnpaint"))).isFile());
    QVERIFY(QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral("links.wsnlink"))).isFile());
    QVERIFY(!QFileInfo(QDir(noteDirectoryPath).filePath(QStringLiteral("backlinks.wsnlink"))).exists());

    QFile headerFile(noteHeaderPath);
    QVERIFY(headerFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(headerFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QCOMPARE(headerStore.noteId(), createdNoteId);
    QCOMPARE(headerStore.author(), QStringLiteral("ProfileName"));
    QCOMPARE(headerStore.modifiedBy(), QStringLiteral("ProfileName"));
    QCOMPARE(headerStore.folders(), QStringList{QStringLiteral("Brand")});
    QVERIFY(!headerStore.createdAt().isEmpty());
    QVERIFY(!headerStore.lastModifiedAt().isEmpty());

    const QString bodyText = readUtf8File(noteBodyPath);
    QVERIFY(bodyText.contains(QStringLiteral("<body>")));
    QVERIFY(!bodyText.contains(QStringLiteral("<paragraph>")));
    QVERIFY(readUtf8File(noteHistoryPath).trimmed().isEmpty());
    QCOMPARE(readJsonObjectFile(noteVersionPath).value(QStringLiteral("snapshots")).toArray().size(), 0);

    const QJsonObject indexRoot = readJsonObjectFile(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")));
    QVERIFY(indexRoot.contains(QStringLiteral("notes")));
    const QJsonArray notesArray = indexRoot.value(QStringLiteral("notes")).toArray();
    QCOMPARE(notesArray.size(), 4);
    QCOMPARE(notesArray.at(notesArray.size() - 1).toString(), createdNoteId);

    const QJsonObject updatedStatRoot = readJsonObjectFile(statFilePath);
    QCOMPARE(updatedStatRoot.value(QStringLiteral("noteCount")).toInt(), 4);
    QCOMPARE(updatedStatRoot.value(QStringLiteral("resourceCount")).toInt(), 0);
    QCOMPARE(updatedStatRoot.value(QStringLiteral("characterCount")).toInt(), 0);
    QVERIFY(!updatedStatRoot.value(QStringLiteral("lastModifiedAtUtc")).toString().isEmpty());
}

void LibraryHierarchyViewModelTest::deleteNoteById_removesScaffoldUpdatesIndexAndSelectsNeighbor()
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
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString statFilePath = QDir(hubPath).filePath(QStringLiteral("LibraryHubStat.wsstat"));

    QJsonObject statRoot;
    statRoot.insert(QStringLiteral("version"), 1);
    statRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.stat"));
    statRoot.insert(QStringLiteral("hub"), QStringLiteral("LibraryHub"));
    statRoot.insert(QStringLiteral("noteCount"), 3);
    statRoot.insert(QStringLiteral("resourceCount"), 0);
    statRoot.insert(QStringLiteral("characterCount"), 0);
    statRoot.insert(QStringLiteral("createdAtUtc"), QStringLiteral("2026-03-01T00:00:00Z"));
    statRoot.insert(QStringLiteral("lastModifiedAtUtc"), QStringLiteral("2026-03-01T00:00:00Z"));
    statRoot.insert(QStringLiteral("participants"), QJsonArray{QStringLiteral("ProfileName")});
    statRoot.insert(QStringLiteral("profileLastModifiedAtUtc"), QJsonObject{});
    QVERIFY(writeUtf8File(
        statFilePath,
        QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.noteListModel()->setCurrentIndex(1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-b"));

    QSignalSpy noteDeletedSpy(&viewModel, &LibraryHierarchyViewModel::noteDeleted);
    QVERIFY(viewModel.deleteNoteById(QStringLiteral("note-b")));

    QCOMPARE(noteDeletedSpy.count(), 1);
    QCOMPARE(noteDeletedSpy.takeFirst().at(0).toString(), QStringLiteral("note-b"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);
    QCOMPARE(viewModel.noteListModel()->currentIndex(), 1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-c"));
    QVERIFY(!QFileInfo(QDir(libraryPath).filePath(QStringLiteral("Beta.wsnote"))).exists());

    const QJsonObject indexRoot = readJsonObjectFile(QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")));
    const QStringList noteIds = noteIdsFromIndexArray(indexRoot.value(QStringLiteral("notes")).toArray());
    QCOMPARE(noteIds, QStringList({QStringLiteral("note-a"), QStringLiteral("note-c")}));

    const QJsonObject updatedStatRoot = readJsonObjectFile(statFilePath);
    QCOMPARE(updatedStatRoot.value(QStringLiteral("noteCount")).toInt(), 2);
    QVERIFY(!updatedStatRoot.value(QStringLiteral("lastModifiedAtUtc")).toString().isEmpty());
}

void LibraryHierarchyViewModelTest::clearNoteFoldersById_rewritesHeaderAndRefreshesVisibleFolders()
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

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.noteListModel()->setCurrentIndex(1);
    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-b"));
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(1, 0),
                      LibraryNoteListModel::FoldersRole).
                  toStringList(),
        QStringList({QStringLiteral("Workspace")}));

    QSignalSpy filesystemSpy(&viewModel, &LibraryHierarchyViewModel::hubFilesystemMutated);
    QVERIFY(viewModel.clearNoteFoldersById(QStringLiteral("note-b")));
    QCOMPARE(filesystemSpy.count(), 1);

    QFile betaHeaderFile(QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary/Beta.wsnote/Beta.wsnhead")));
    QVERIFY(betaHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(betaHeaderFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QVERIFY(headerStore.folders().isEmpty());
    QVERIFY(headerStore.lastModifiedAt().startsWith(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))));

    QCOMPARE(viewModel.noteListModel()->currentNoteId(), QStringLiteral("note-b"));
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(1, 0),
                      LibraryNoteListModel::FoldersRole).
                  toStringList(),
        QStringList());
    QVERIFY(viewModel.clearNoteFoldersById(QStringLiteral("note-b")));
}

void LibraryHierarchyViewModelTest::loadFromWshub_prunesOrphanIndexEntriesAndRewritesIndex()
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
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString indexFilePath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));

    QJsonObject indexRoot = readJsonObjectFile(indexFilePath);
    QJsonArray notesArray = indexRoot.value(QStringLiteral("notes")).toArray();
    notesArray.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("note-orphan")},
        {QStringLiteral("lastModified"), QStringLiteral("2024-02-02-00-00-00")}
    });
    indexRoot.insert(QStringLiteral("notes"), notesArray);
    QVERIFY(writeUtf8File(indexFilePath, QString::fromUtf8(QJsonDocument(indexRoot).toJson(QJsonDocument::Indented))));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
    const QStringList visibleNoteIds = {
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::IdRole).
                  toString(),
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::IdRole).
                  toString(),
        viewModel.noteListModel()->data(viewModel.noteListModel()->index(2, 0), LibraryNoteListModel::IdRole).toString()
    };
    QVERIFY(!visibleNoteIds.contains(QStringLiteral("note-orphan")));

    const QJsonObject healedIndexRoot = readJsonObjectFile(indexFilePath);
    QCOMPARE(
        noteIdsFromIndexArray(healedIndexRoot.value(QStringLiteral("notes")).toArray()),
        QStringList({QStringLiteral("note-a"), QStringLiteral("note-b"), QStringLiteral("note-c")}));
}

void LibraryHierarchyViewModelTest::loadFromWshub_moveFolderBefore_rewritesFoldersFileAndHeaderAssignments()
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
            {QStringLiteral("Research"), QStringLiteral("/Competitor")})));

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
    const int competitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    const int brandIndex = findIndexByLabel(QStringLiteral("Brand"));
    QVERIFY(researchIndex >= 0);
    QVERIFY(competitorIndex >= 0);
    QVERIFY(brandIndex >= 0);

    QVERIFY(viewModel.canAcceptFolderDropBefore(competitorIndex, brandIndex));
    QVERIFY(viewModel.moveFolderBefore(competitorIndex, brandIndex));

    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(researchIndex, 0), LibraryHierarchyModel::LabelRole)
                 .toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(researchIndex + 1, 0),
                                    LibraryHierarchyModel::LabelRole)
                 .toString(),
        QStringLiteral("Competitor"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(researchIndex + 1, 0),
                                    LibraryHierarchyModel::DepthRole)
                 .toInt(),
        0);

    const QJsonDocument foldersDocument = QJsonDocument::fromJson(readUtf8File(foldersFilePath).toUtf8());
    QVERIFY(foldersDocument.isObject());
    const QJsonArray folderArray = foldersDocument.object().value(QStringLiteral("folders")).toArray();
    QCOMPARE(folderArray.size(), 3);
    QCOMPARE(folderArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    QVERIFY(folderArray.at(0).toObject().value(QStringLiteral("children")).toArray().isEmpty());
    QCOMPARE(folderArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Competitor"));
    QCOMPARE(folderArray.at(2).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));

    QFile alphaHeaderFile(QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")));
    QVERIFY(alphaHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(alphaHeaderFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QVERIFY2(
        headerStore.folders() == QStringList({QStringLiteral("Competitor")}),
        qPrintable(headerStore.folders().join(QStringLiteral("|"))));
    QVERIFY(headerStore.lastModifiedAt().startsWith(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))));

    viewModel.setSelectedIndex(researchIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 0);

    const int movedCompetitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    QVERIFY(movedCompetitorIndex >= 0);
    viewModel.setSelectedIndex(movedCompetitorIndex);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
                      viewModel.noteListModel()->index(0, 0),
                      LibraryNoteListModel::IdRole).
                  toString(),
        QStringLiteral("note-a"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_applyHierarchyNodes_persistsLvrsEditableMove()
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
            {QStringLiteral("Research"), QStringLiteral("/Competitor")})));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QVariantList hierarchyNodes = viewModel.depthItems();
    QVERIFY(hierarchyNodes.size() >= 6);
    for (int index = 0; index < hierarchyNodes.size(); ++index)
    {
        QVariantMap entry = hierarchyNodes.at(index).toMap();
        entry.insert(QStringLiteral("sourceIndex"), index);
        hierarchyNodes[index] = entry;
    }

    const auto takeNodeByKey = [&hierarchyNodes](const QString& itemKey) -> QVariantMap
    {
        for (int index = 0; index < hierarchyNodes.size(); ++index)
        {
            const QVariantMap entry = hierarchyNodes.at(index).toMap();
            if (entry.value(QStringLiteral("key")).toString() == itemKey)
            {
                hierarchyNodes.removeAt(index);
                return entry;
            }
        }
        return {};
    };

    QVariantList reorderedNodes;
    reorderedNodes.push_back(takeNodeByKey(QStringLiteral("bucket:all")));
    reorderedNodes.push_back(takeNodeByKey(QStringLiteral("bucket:draft")));
    reorderedNodes.push_back(takeNodeByKey(QStringLiteral("bucket:today")));
    reorderedNodes.push_back(takeNodeByKey(QStringLiteral("Research")));

    QVariantMap competitorNode = takeNodeByKey(QStringLiteral("Research/Competitor"));
    QVERIFY(!competitorNode.isEmpty());
    competitorNode.insert(QStringLiteral("depth"), 0);
    reorderedNodes.push_back(competitorNode);

    reorderedNodes.push_back(takeNodeByKey(QStringLiteral("Brand")));
    QVERIFY(hierarchyNodes.isEmpty());

    QVERIFY(viewModel.applyHierarchyNodes(reorderedNodes, QStringLiteral("Research/Competitor")));

    const auto findIndexByLabel = [&viewModel](const QString& label) -> int
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

    const int movedCompetitorIndex = findIndexByLabel(QStringLiteral("Competitor"));
    QVERIFY(movedCompetitorIndex >= 0);
    QCOMPARE(viewModel.selectedIndex(), movedCompetitorIndex);
    QCOMPARE(
        viewModel.itemModel()->data(
            viewModel.itemModel()->index(movedCompetitorIndex, 0),
            LibraryHierarchyModel::DepthRole).toInt(),
        0);

    const QJsonDocument foldersDocument = QJsonDocument::fromJson(readUtf8File(foldersFilePath).toUtf8());
    QVERIFY(foldersDocument.isObject());
    const QJsonArray folderArray = foldersDocument.object().value(QStringLiteral("folders")).toArray();
    QCOMPARE(folderArray.size(), 3);
    QCOMPARE(folderArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
    QVERIFY(folderArray.at(0).toObject().value(QStringLiteral("children")).toArray().isEmpty());
    QCOMPARE(folderArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Competitor"));
    QCOMPARE(folderArray.at(2).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));

    QFile alphaHeaderFile(QDir(libraryPath).filePath(QStringLiteral("Alpha.wsnote/Alpha.wsnhead")));
    QVERIFY(alphaHeaderFile.open(QIODevice::ReadOnly | QIODevice::Text));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(QString::fromUtf8(alphaHeaderFile.readAll()), &headerStore, &parseError),
             qPrintable(parseError));
    QCOMPARE(headerStore.folders(), QStringList({QStringLiteral("Competitor")}));

    QCOMPARE(viewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        viewModel.noteListModel()->data(
            viewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::IdRole).toString(),
        QStringLiteral("note-a"));
}

void LibraryHierarchyViewModelTest::loadFromWshub_applyHierarchyNodes_preservesSystemBucketPrefix()
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

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QVariantList hierarchyNodes = viewModel.depthItems();
    QVERIFY(hierarchyNodes.size() >= 6);
    for (int index = 0; index < hierarchyNodes.size(); ++index)
    {
        QVariantMap entry = hierarchyNodes.at(index).toMap();
        entry.insert(QStringLiteral("sourceIndex"), index);
        hierarchyNodes[index] = entry;
    }

    const auto takeNodeByKey = [&hierarchyNodes](const QString& itemKey) -> QVariantMap
    {
        for (int index = 0; index < hierarchyNodes.size(); ++index)
        {
            const QVariantMap entry = hierarchyNodes.at(index).toMap();
            if (entry.value(QStringLiteral("key")).toString() == itemKey)
            {
                hierarchyNodes.removeAt(index);
                return entry;
            }
        }
        return {};
    };

    QVariantMap allNode = takeNodeByKey(QStringLiteral("bucket:all"));
    QVariantMap draftNode = takeNodeByKey(QStringLiteral("bucket:draft"));
    QVariantMap todayNode = takeNodeByKey(QStringLiteral("bucket:today"));
    QVariantMap researchNode = takeNodeByKey(QStringLiteral("Research"));
    QVariantMap competitorNode = takeNodeByKey(QStringLiteral("Research/Competitor"));
    QVariantMap brandNode = takeNodeByKey(QStringLiteral("Brand"));

    QVERIFY(!allNode.isEmpty());
    QVERIFY(!draftNode.isEmpty());
    QVERIFY(!todayNode.isEmpty());
    QVERIFY(!researchNode.isEmpty());
    QVERIFY(!competitorNode.isEmpty());
    QVERIFY(!brandNode.isEmpty());

    todayNode.insert(QStringLiteral("depth"), 1);
    competitorNode.insert(QStringLiteral("depth"), 1);

    QVariantList reorderedNodes;
    reorderedNodes.push_back(allNode);
    reorderedNodes.push_back(draftNode);
    reorderedNodes.push_back(brandNode);
    reorderedNodes.push_back(todayNode);
    reorderedNodes.push_back(researchNode);
    reorderedNodes.push_back(competitorNode);

    QVERIFY(viewModel.applyHierarchyNodes(reorderedNodes, QStringLiteral("bucket:today")));

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
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(viewModel.selectedIndex(), 2);
}

void LibraryHierarchyViewModelTest::loadFromWshub_applyHierarchyNodes_preservesDraggedFolderBetweenSystemBuckets_withoutExplicitSourceIndex()
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

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QVariantList hierarchyNodes = viewModel.depthItems();
    QVERIFY(hierarchyNodes.size() >= 6);

    const auto takeNodeByKey = [&hierarchyNodes](const QString& itemKey) -> QVariantMap
    {
        for (int index = 0; index < hierarchyNodes.size(); ++index)
        {
            const QVariantMap entry = hierarchyNodes.at(index).toMap();
            if (entry.value(QStringLiteral("key")).toString() == itemKey)
            {
                hierarchyNodes.removeAt(index);
                return entry;
            }
        }
        return {};
    };

    QVariantMap allNode = takeNodeByKey(QStringLiteral("bucket:all"));
    QVariantMap draftNode = takeNodeByKey(QStringLiteral("bucket:draft"));
    QVariantMap todayNode = takeNodeByKey(QStringLiteral("bucket:today"));
    QVariantMap researchNode = takeNodeByKey(QStringLiteral("Research"));
    QVariantMap competitorNode = takeNodeByKey(QStringLiteral("Research/Competitor"));
    QVariantMap brandNode = takeNodeByKey(QStringLiteral("Brand"));

    QVERIFY(!allNode.isEmpty());
    QVERIFY(!draftNode.isEmpty());
    QVERIFY(!todayNode.isEmpty());
    QVERIFY(!researchNode.isEmpty());
    QVERIFY(!competitorNode.isEmpty());
    QVERIFY(!brandNode.isEmpty());
    QVERIFY(!brandNode.contains(QStringLiteral("sourceIndex")));
    QVERIFY(brandNode.value(QStringLiteral("itemId")).toInt() >= 0);

    QVariantList reorderedNodes;
    reorderedNodes.push_back(allNode);
    reorderedNodes.push_back(draftNode);
    reorderedNodes.push_back(brandNode);
    reorderedNodes.push_back(todayNode);
    reorderedNodes.push_back(researchNode);
    reorderedNodes.push_back(competitorNode);

    QVERIFY(viewModel.applyHierarchyNodes(reorderedNodes, QStringLiteral("Brand")));

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
        viewModel.itemModel()->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(5, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Competitor"));
    QCOMPARE(viewModel.selectedIndex(), 3);

    const QJsonObject foldersRoot = readJsonObjectFile(foldersFilePath);
    QVERIFY(!foldersRoot.isEmpty());
    const QJsonArray folderArray = foldersRoot.value(QStringLiteral("folders")).toArray();
    QCOMPARE(folderArray.size(), 2);
    QCOMPARE(folderArray.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Brand"));
    QCOMPARE(folderArray.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("Research"));
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

    QVERIFY(viewModel.canAcceptFolderDropBefore(2, 1));
    QVERIFY(viewModel.moveFolderBefore(2, 1));

    const QVariantList movedBefore = viewModel.depthItems();
    QCOMPARE(movedBefore.size(), 3);
    QCOMPARE(viewModel.selectedIndex(), 1);
    QCOMPARE(movedBefore.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("ChildA"));
    QCOMPARE(movedBefore.at(0).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedBefore.at(1).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootB"));
    QCOMPARE(movedBefore.at(1).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedBefore.at(1).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootB"));
    QCOMPARE(movedBefore.at(2).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("RootA"));
    QCOMPARE(movedBefore.at(2).toMap().value(QStringLiteral("depth")).toInt(), 0);
    QCOMPARE(movedBefore.at(2).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("RootA"));
}

QTEST_APPLESS_MAIN(LibraryHierarchyViewModelTest)

#include "test_library_hierarchy_view_model.moc"
