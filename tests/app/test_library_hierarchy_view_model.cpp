#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"

#include <QDate>
#include <QDir>
#include <QFile>
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
    void deleteSelectedFolder_removesDescendantSubtree();
    void loadFromWshub_buildsAllDraftTodayBuckets();
    void loadFromWshub_populatesNoteListModelAndSwitchesBySelectedBucket();
    void loadFromWshub_usesFoldersFileForSidebarItems();
    void loadFromWshub_readsDynamicWslibraryDirectory();
    void setDepthItems_emptyInput_preservesIndexedBuckets();
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

    QString makeWsnHeadText(
        const QString& noteId,
        const QString& title,
        const QString& createdAt,
        const QString& lastModifiedAt,
        const QStringList& folders)
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"%1\">\n").arg(noteId);
        text += QStringLiteral("  <head>\n");
        text += QStringLiteral("    <title>%1</title>\n").arg(title);
        text += QStringLiteral("    <created>%1</created>\n").arg(createdAt);
        text += QStringLiteral("    <lastModified>%1</lastModified>\n").arg(lastModifiedAt);
        text += QStringLiteral("    <folders>\n");
        for (const QString& folder : folders)
        {
            text += QStringLiteral("      <folder>%1</folder>\n").arg(folder);
        }
        text += QStringLiteral("    </folders>\n");
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
        const QString& libraryDirectoryName = QStringLiteral("Library.wslibrary"))
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
            {QStringLiteral("title"), QStringLiteral("Alpha Note")},
            {QStringLiteral("lastModified"), oldText}
        });
        notesArray.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("note-b")},
            {QStringLiteral("title"), QStringLiteral("Beta Note")},
            {QStringLiteral("lastModified"), todayText}
        });
        notesArray.append(QJsonObject{
            {QStringLiteral("id"), QStringLiteral("note-c")},
            {QStringLiteral("title"), QStringLiteral("Gamma Note")},
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
                QStringLiteral("Alpha Note"),
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
                QStringLiteral("Beta Note"),
                oldText,
                todayText,
                {QStringLiteral("Workspace")})))
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
                QStringLiteral("Gamma Note"),
                oldText,
                oldText,
                {})))
        {
            return false;
        }
        if (!writeUtf8File(
            QDir(noteCPath).filePath(QStringLiteral("Gamma.wsnbody")),
            makeWsnBodyText(QStringLiteral("    <paragraph>Gamma body summary.</paragraph>\n"))))
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
        QStringLiteral("Folder1"));
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
        QStringLiteral("Folder1"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
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

    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All (3)"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(4, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Draft (2)"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(7, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Today (2)"));

    const QString allFirstLabel = viewModel.itemModel()
                                           ->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::LabelRole)
                                           .toString();
    const QString allSecondLabel = viewModel.itemModel()
                                            ->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole)
                                            .toString();
    const QString allThirdLabel = viewModel.itemModel()
                                           ->data(viewModel.itemModel()->index(3, 0), LibraryHierarchyModel::LabelRole)
                                           .toString();
    QVERIFY(allFirstLabel.startsWith(QStringLiteral("Alpha Note")));
    QVERIFY(allSecondLabel.startsWith(QStringLiteral("Beta Note")));
    QVERIFY(allThirdLabel.startsWith(QStringLiteral("Gamma Note")));
}

void LibraryHierarchyViewModelTest::setDepthItems_emptyInput_preservesIndexedBuckets()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    viewModel.setDepthItems({});
    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("All (3)"));
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
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::TitleTextRole)
                 .toString(),
        QStringLiteral("Alpha Note"));
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(0, 0), LibraryNoteListModel::SummaryTextRole)
                 .toString(),
        QStringLiteral("Alpha body summary."));
    QCOMPARE(
        viewModel.noteListModel()
                 ->data(viewModel.noteListModel()->index(1, 0), LibraryNoteListModel::SummaryTextRole)
                 .toString(),
        QStringLiteral("Beta body summary."));

    viewModel.setSelectedIndex(4);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    viewModel.setSelectedIndex(7);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 2);

    viewModel.setSelectedIndex(2);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);

    int highlightedCount = 0;
    QString highlightedTitle;
    for (int index = 0; index < viewModel.noteListModel()->rowCount(); ++index)
    {
        const bool highlighted = viewModel.noteListModel()
                                          ->data(
                                              viewModel.noteListModel()->index(index, 0),
                                              LibraryNoteListModel::HighlightedRole)
                                          .toBool();
        if (!highlighted)
        {
            continue;
        }
        ++highlightedCount;
        highlightedTitle = viewModel.noteListModel()
                                    ->data(
                                        viewModel.noteListModel()->index(index, 0),
                                        LibraryNoteListModel::TitleTextRole)
                                    .toString();
    }

    QCOMPARE(highlightedCount, 1);
    QCOMPARE(highlightedTitle, QStringLiteral("Beta Note"));
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
        "    {\"id\": \"Research\", \"label\": \"Research\", \"depth\": 0},\n"
        "    {\"id\": \"Research/Competitor\", \"label\": \"Competitor\", \"depth\": 1},\n"
        "    {\"id\": \"Brand\", \"label\": \"Brand\", \"depth\": 0}\n"
        "  ]\n"
        "}\n");
    QVERIFY(writeUtf8File(foldersFilePath, foldersJson));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Research"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Brand"));
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

void LibraryHierarchyViewModelTest::loadFromWshub_readsDynamicWslibraryDirectory()
{
    QString hubPath;
    QVERIFY(prepareIndexedLibraryHub(&hubPath, QStringLiteral("Workspace.wslibrary")));

    LibraryHierarchyViewModel viewModel;
    QString errorMessage;
    QVERIFY2(viewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    QCOMPARE(viewModel.itemModel()->rowCount(), 10);
    QCOMPARE(viewModel.noteListModel()->rowCount(), 3);
}

QTEST_APPLESS_MAIN(LibraryHierarchyViewModelTest)

#include "test_library_hierarchy_view_model.moc"
