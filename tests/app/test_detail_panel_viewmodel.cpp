#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"
#include "file/hierarchy/WhatSonFolderIdentity.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QVariantMap>
#include <QtTest>

class FakeDetailHierarchySourceViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel WRITE setHierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    QVariantList hierarchyModel() const
    {
        return m_hierarchyModel;
    }

    int selectedIndex() const noexcept
    {
        return m_selectedIndex;
    }

    void setHierarchyModel(const QVariantList& hierarchyModel)
    {
        if (m_hierarchyModel == hierarchyModel)
        {
            return;
        }

        m_hierarchyModel = hierarchyModel;
        emit hierarchyModelChanged();
    }

    void setSelectedIndex(int index)
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

signals:
    void hierarchyModelChanged();
    void selectedIndexChanged();

private:
    QVariantList m_hierarchyModel;
    int m_selectedIndex = -1;
};

class FakeCurrentNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)

public:
    QString currentNoteId() const
    {
        return m_currentNoteId;
    }

    void setCurrentNoteId(const QString& noteId)
    {
        const QString normalized = noteId.trimmed();
        if (m_currentNoteId == normalized)
        {
            return;
        }

        m_currentNoteId = normalized;
        emit currentNoteIdChanged();
    }

signals:
    void currentNoteIdChanged();

private:
    QString m_currentNoteId;
};

class FakeNoteDirectorySourceViewModel final : public QObject
{
    Q_OBJECT

public:
    void setNoteDirectoryPath(const QString& noteId, const QString& noteDirectoryPath)
    {
        m_noteDirectoryPaths.insert(noteId.trimmed(), noteDirectoryPath.trimmed());
    }

    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const
    {
        return m_noteDirectoryPaths.value(noteId.trimmed());
    }

private:
    QHash<QString, QString> m_noteDirectoryPaths;
};

namespace
{
    QVariantMap makeHierarchyEntry(
        const QString& key,
        const QString& label,
        const QVariantMap& extraFields = {})
    {
        QVariantMap entry{
            {QStringLiteral("key"), key},
            {QStringLiteral("label"), label}
        };
        for (auto it = extraFields.constBegin(); it != extraFields.constEnd(); ++it)
        {
            entry.insert(it.key(), it.value());
        }
        return entry;
    }

    QVariantList detailSelectorHierarchyModel(
        const QString& clearKey,
        const QString& clearLabel,
        const QVariantList& sourceEntries,
        const QVariantMap& extraFields = {})
    {
        QVariantList hierarchyModel;
        hierarchyModel.reserve(sourceEntries.size() + 1);
        QVariantMap clearEntryExtraFields = extraFields;
        clearEntryExtraFields.insert(QStringLiteral("clearSelection"), true);
        hierarchyModel.push_back(makeHierarchyEntry(clearKey, clearLabel, clearEntryExtraFields));
        for (const QVariant& entry : sourceEntries)
        {
            hierarchyModel.push_back(entry);
        }
        return hierarchyModel;
    }

    bool writeUtf8File(const QString& filePath, const QString& text)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }

        file.write(text.toUtf8());
        file.close();
        return true;
    }
}

class DetailPanelViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaults_mustExposePropertiesState();
    void detailContentViewModels_mustMapEachStateToDedicatedViewModel();
    void detailSelectorViewModels_mustMirrorHierarchyItemsWithoutSharingSelection();
    void detailSelectors_mustMirrorCurrentNoteHeaderFileState();
    void detailProgressSelection_mustWriteCurrentModelValueForProgressHierarchyQueries();
    void folderAssignments_mustSyncHeaderFoldersAndHierarchyStore();
    void requestStateChange_mustUpdateActiveStateAndToolbarSelection();
    void requestStateChange_invalidValue_mustBeIgnored();
};

void DetailPanelViewModelTest::defaults_mustExposePropertiesState()
{
    DetailPanelViewModel viewModel;

    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Properties));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("properties"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    const QStringList expectedIconNames = {
        QStringLiteral("config"),
        QStringLiteral("chartBar"),
        QStringLiteral("generaladd"),
        QStringLiteral("toolwindowdependencies"),
        QStringLiteral("toolWindowClock"),
        QStringLiteral("featureAnswer")
    };

    for (int index = 0; index < toolbarItems.size(); ++index)
    {
        const QVariantMap item = toolbarItems.at(index).toMap();
        QVERIFY(item.contains(QStringLiteral("iconName")));
        QVERIFY(item.contains(QStringLiteral("stateValue")));
        QVERIFY(item.contains(QStringLiteral("selected")));
        QCOMPARE(item.value(QStringLiteral("iconName")).toString(), expectedIconNames.at(index));
        QCOMPARE(item.value(QStringLiteral("selected")).toBool(), index == 0);
    }
}

void DetailPanelViewModelTest::detailContentViewModels_mustMapEachStateToDedicatedViewModel()
{
    DetailPanelViewModel viewModel;

    QObject* propertiesVm = viewModel.propertiesViewModel();
    QObject* fileStatVm = viewModel.fileStatViewModel();
    QObject* insertVm = viewModel.insertViewModel();
    QObject* fileHistoryVm = viewModel.fileHistoryViewModel();
    QObject* layerVm = viewModel.layerViewModel();
    QObject* helpVm = viewModel.helpViewModel();

    QVERIFY(propertiesVm != nullptr);
    QVERIFY(fileStatVm != nullptr);
    QVERIFY(insertVm != nullptr);
    QVERIFY(fileHistoryVm != nullptr);
    QVERIFY(layerVm != nullptr);
    QVERIFY(helpVm != nullptr);

    QCOMPARE(propertiesVm->property("stateName").toString(), QStringLiteral("properties"));
    QCOMPARE(fileStatVm->property("stateName").toString(), QStringLiteral("fileStat"));
    QCOMPARE(insertVm->property("stateName").toString(), QStringLiteral("insert"));
    QCOMPARE(fileHistoryVm->property("stateName").toString(), QStringLiteral("fileHistory"));
    QCOMPARE(layerVm->property("stateName").toString(), QStringLiteral("layer"));
    QCOMPARE(helpVm->property("stateName").toString(), QStringLiteral("help"));

    QCOMPARE(viewModel.activeContentViewModel(), propertiesVm);
    QCOMPARE(propertiesVm->property("active").toBool(), true);
    QCOMPARE(layerVm->property("active").toBool(), false);

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));

    QCOMPARE(viewModel.activeContentViewModel(), layerVm);
    QCOMPARE(propertiesVm->property("active").toBool(), false);
    QCOMPARE(layerVm->property("active").toBool(), true);
    QCOMPARE(
        viewModel.contentViewModelForState(static_cast<int>(DetailPanelViewModel::DetailContentState::Help)),
        helpVm);
    QCOMPARE(viewModel.contentViewModelForState(-1), nullptr);
}

void DetailPanelViewModelTest::detailSelectorViewModels_mustMirrorHierarchyItemsWithoutSharingSelection()
{
    DetailPanelViewModel viewModel;
    FakeDetailHierarchySourceViewModel projectsSource;
    FakeDetailHierarchySourceViewModel bookmarksSource;
    FakeDetailHierarchySourceViewModel progressSource;

    const QVariantList sharedHierarchyModel = {
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("project:alpha")},
            {QStringLiteral("label"), QStringLiteral("Alpha")},
            {QStringLiteral("iconName"), QStringLiteral("customFolder")}
        },
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("project:beta")},
            {QStringLiteral("label"), QStringLiteral("Beta")},
            {QStringLiteral("iconName"), QStringLiteral("customFolder")}
        }
    };

    projectsSource.setHierarchyModel(sharedHierarchyModel);
    projectsSource.setSelectedIndex(1);
    bookmarksSource.setHierarchyModel(sharedHierarchyModel);
    bookmarksSource.setSelectedIndex(0);
    progressSource.setHierarchyModel(sharedHierarchyModel);
    progressSource.setSelectedIndex(-1);

    viewModel.setProjectSelectionSourceViewModel(&projectsSource);
    viewModel.setBookmarkSelectionSourceViewModel(&bookmarksSource);
    viewModel.setProgressSelectionSourceViewModel(&progressSource);

    QObject* projectSelector = viewModel.projectSelectionViewModel();
    QObject* bookmarkSelector = viewModel.bookmarkSelectionViewModel();
    QObject* progressSelector = viewModel.progressSelectionViewModel();

    QVERIFY(projectSelector != nullptr);
    QVERIFY(bookmarkSelector != nullptr);
    QVERIFY(progressSelector != nullptr);

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), -1);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), -1);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), -1);
    QCOMPARE(
        projectSelector->property("hierarchyModel").toList(),
        detailSelectorHierarchyModel(
            QStringLiteral("detail:none:project"),
            QStringLiteral("No project"),
            sharedHierarchyModel));

    projectsSource.setSelectedIndex(0);
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), -1);

    QVERIFY(QMetaObject::invokeMethod(projectSelector, "setSelectedIndex", Q_ARG(int, 0)));
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(projectsSource.selectedIndex(), 0);

    const QVariantList reorderedHierarchyModel = {
        QVariantMap{
            {QStringLiteral("key"), QStringLiteral("project:gamma")},
            {QStringLiteral("label"), QStringLiteral("Gamma")},
            {QStringLiteral("iconName"), QStringLiteral("customFolder")}
        },
        sharedHierarchyModel.at(0).toMap(),
        sharedHierarchyModel.at(1).toMap()
    };
    projectsSource.setHierarchyModel(reorderedHierarchyModel);

    QCOMPARE(
        projectSelector->property("hierarchyModel").toList(),
        detailSelectorHierarchyModel(
            QStringLiteral("detail:none:project"),
            QStringLiteral("No project"),
            reorderedHierarchyModel));
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 0);
}

void DetailPanelViewModelTest::detailSelectors_mustMirrorCurrentNoteHeaderFileState()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString noteId = QStringLiteral("MirrorNote");
    const QString noteDirectoryPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("MirrorNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString headerFilePath = QDir(noteDirectoryPath).filePath(QStringLiteral("MirrorNote.wsnhead"));
    const QString initialHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"MirrorNote\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "      <folder>Archive/Knowledge</folder>\n"
                       "    </folders>\n"
                       "    <project>Beta</project>\n"
                       "    <bookmarks state=\"true\" colors=\"amber\" />\n"
                       "    <tags>\n"
                       "      <tag>seed</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Queued,Review,Ship,Published}\">7</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, initialHeaderText), "initial header file must be written");

    DetailPanelViewModel viewModel;
    FakeDetailHierarchySourceViewModel projectsSource;
    FakeDetailHierarchySourceViewModel bookmarksSource;
    FakeDetailHierarchySourceViewModel progressSource;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;

    projectsSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("project:alpha"), QStringLiteral("Alpha")),
        makeHierarchyEntry(QStringLiteral("project:beta"), QStringLiteral("Beta")),
        makeHierarchyEntry(QStringLiteral("project:gamma"), QStringLiteral("Gamma"))
    });
    bookmarksSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("bookmark:red"), QStringLiteral("Red")),
        makeHierarchyEntry(QStringLiteral("bookmark:amber"), QStringLiteral("Amber")),
        makeHierarchyEntry(QStringLiteral("bookmark:blue"), QStringLiteral("Blue"))
    });
    progressSource.setHierarchyModel({
        makeHierarchyEntry(
            QStringLiteral("progress:4"),
            QStringLiteral("Queued"),
            QVariantMap{{QStringLiteral("itemId"), 4}}),
        makeHierarchyEntry(
            QStringLiteral("progress:7"),
            QStringLiteral("Review"),
            QVariantMap{{QStringLiteral("itemId"), 7}}),
        makeHierarchyEntry(
            QStringLiteral("progress:9"),
            QStringLiteral("Ship"),
            QVariantMap{{QStringLiteral("itemId"), 9}}),
        makeHierarchyEntry(
            QStringLiteral("progress:11"),
            QStringLiteral("Published"),
            QVariantMap{{QStringLiteral("itemId"), 11}})
    });

    viewModel.setProjectSelectionSourceViewModel(&projectsSource);
    viewModel.setBookmarkSelectionSourceViewModel(&bookmarksSource);
    viewModel.setProgressSelectionSourceViewModel(&progressSource);

    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    viewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    noteListModel.setCurrentNoteId(noteId);
    viewModel.setCurrentNoteListModel(&noteListModel);

    QObject* propertiesViewModel = viewModel.propertiesViewModel();
    QObject* projectSelector = viewModel.projectSelectionViewModel();
    QObject* bookmarkSelector = viewModel.bookmarkSelectionViewModel();
    QObject* progressSelector = viewModel.progressSelectionViewModel();

    QVERIFY(propertiesViewModel != nullptr);
    QVERIFY(projectSelector != nullptr);
    QVERIFY(bookmarkSelector != nullptr);
    QVERIFY(progressSelector != nullptr);

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{QStringLiteral("Knowledge")});
    QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{QStringLiteral("seed")});
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Beta"));
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QStringLiteral("amber"));
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), 7);

    const QVariantList projectHierarchyModel = projectSelector->property("hierarchyModel").toList();
    const QVariantList bookmarkHierarchyModel = bookmarkSelector->property("hierarchyModel").toList();
    const QVariantList progressHierarchyModel = progressSelector->property("hierarchyModel").toList();
    QCOMPARE(projectHierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("No project"));
    QCOMPARE(bookmarkHierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("No bookmark"));
    QCOMPARE(progressHierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("No progress"));

    QVERIFY(viewModel.writeProjectSelection(3));
    QVERIFY(viewModel.writeBookmarkSelection(1));
    QVERIFY(viewModel.writeProgressSelection(3));
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 3);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 3);
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Gamma"));
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QStringLiteral("red"));
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), 9);

    QVERIFY(viewModel.writeProjectSelection(0));
    QVERIFY(viewModel.writeBookmarkSelection(0));
    QVERIFY(viewModel.writeProgressSelection(0));
    QVERIFY(propertiesViewModel->setProperty("activeFolderIndex", 0));
    QVERIFY(viewModel.removeActiveFolder());
    QVERIFY(propertiesViewModel->setProperty("activeTagIndex", 0));
    QVERIFY(viewModel.removeActiveTag());

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{});
    QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{});
    QCOMPARE(propertiesViewModel->property("activeFolderIndex").toInt(), -1);
    QCOMPARE(propertiesViewModel->property("activeTagIndex").toInt(), -1);
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QString());
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QString());
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), -1);

    QFile headerFile(headerFilePath);
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "persisted header file must be readable");
    const QString updatedHeaderText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(updatedHeaderText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.folders(), QStringList{});
    QCOMPARE(header.project(), QString());
    QVERIFY(!header.isBookmarked());
    QCOMPARE(header.bookmarkColors(), QStringList{});
    QCOMPARE(header.tags(), QStringList{});
    QCOMPARE(header.progress(), -1);
}

void DetailPanelViewModelTest::detailProgressSelection_mustWriteCurrentModelValueForProgressHierarchyQueries()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailProgressBridge.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailProgressBridge.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("BridgeNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString noteId = QStringLiteral("note-bridge");
    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-bridge\"}\n"
                "  ]\n"
                "}\n")),
        "index.wsnindex must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("Progress.wsprogress")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.progress.state\",\n"
                "  \"progress\": 0,\n"
                "  \"states\": [\"Ready\", \"Pending\", \"InProgress\", \"Done\"]\n"
                "}\n")),
        "Progress.wsprogress must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("BridgeNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-bridge\">\n"
                "  <head>\n"
                "    <created>2026-03-01-00-00-00</created>\n"
                "    <author>Tester</author>\n"
                "    <lastModified>2026-03-01-00-00-00</lastModified>\n"
                "    <modifiedBy>Tester</modifiedBy>\n"
                "    <folders></folders>\n"
                "    <project></project>\n"
                "    <bookmarks state=\"false\" />\n"
                "    <tags>\n"
                "    </tags>\n"
                "    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n"
                "    <isPreset>false</isPreset>\n"
                "  </head>\n"
                "</contents>\n")),
        "BridgeNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("BridgeNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-bridge\">\n"
                "  <body>\n"
                "    <paragraph>Done bridge summary</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "BridgeNote.wsnbody must be written");

    ProgressHierarchyViewModel progressHierarchyViewModel;
    QVERIFY(progressHierarchyViewModel.loadFromWshub(hubPath));

    DetailPanelViewModel detailPanelViewModel;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;

    noteListModel.setCurrentNoteId(noteId);
    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    detailPanelViewModel.setCurrentNoteListModel(&noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    detailPanelViewModel.setProgressSelectionSourceViewModel(&progressHierarchyViewModel);

    QObject* progressSelector = detailPanelViewModel.progressSelectionViewModel();
    QVERIFY(progressSelector != nullptr);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 1);

    QVERIFY(detailPanelViewModel.writeProgressSelection(7));
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 7);
    QCOMPARE(detailPanelViewModel.propertiesViewModel()->property("currentProgress").toInt(), 6);

    QVERIFY(progressHierarchyViewModel.loadFromWshub(hubPath));
    progressHierarchyViewModel.setSelectedIndex(6);
    QCOMPARE(progressHierarchyViewModel.noteListModel()->rowCount(), 1);
    QCOMPARE(
        progressHierarchyViewModel.noteListModel()->data(
            progressHierarchyViewModel.noteListModel()->index(0, 0),
            LibraryNoteListModel::NoteIdRole).toString(),
        noteId);
}

void DetailPanelViewModelTest::folderAssignments_mustSyncHeaderFoldersAndHierarchyStore()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString contentsDirectoryPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("TestHub.wscontents"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));

    const QString noteId = QStringLiteral("FolderNote");
    const QString noteDirectoryPath = QDir(contentsDirectoryPath).filePath(QStringLiteral("FolderNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString headerFilePath = QDir(noteDirectoryPath).filePath(QStringLiteral("FolderNote.wsnhead"));
    const QString initialHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"FolderNote\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "    </folders>\n"
                       "    <project></project>\n"
                       "    <bookmarks state=\"false\" colors=\"\" />\n"
                       "    <tags>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, initialHeaderText), "initial header file must be written");

    const QString foldersFilePath = QDir(contentsDirectoryPath).filePath(QStringLiteral("Folders.wsfolders"));
    const QString knowledgeUuid = WhatSon::FolderIdentity::createFolderUuid();
    WhatSonFoldersHierarchyStore initialFoldersStore;
    initialFoldersStore.setFolderEntries({
        WhatSonFolderDepthEntry{
            QStringLiteral("Knowledge"),
            QStringLiteral("Knowledge"),
            0,
            knowledgeUuid
        }
    });
    QString foldersWriteError;
    QVERIFY2(initialFoldersStore.writeToFile(foldersFilePath, &foldersWriteError), qPrintable(foldersWriteError));

    DetailPanelViewModel viewModel;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;
    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    viewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    noteListModel.setCurrentNoteId(noteId);
    viewModel.setCurrentNoteListModel(&noteListModel);

    QObject* propertiesViewModel = viewModel.propertiesViewModel();
    QVERIFY(propertiesViewModel != nullptr);

    QVERIFY(viewModel.assignFolderByName(QStringLiteral("Knowledge")));
    QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{QStringLiteral("Knowledge")});
    QCOMPARE(propertiesViewModel->property("activeFolderIndex").toInt(), 0);

    QFile headerFile(headerFilePath);
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "header file must be readable");
    QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    QVERIFY2(headerParser.parse(headerText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.folders(), QStringList{QStringLiteral("Knowledge")});
    QCOMPARE(header.folderUuids(), QStringList{knowledgeUuid});

    QFile foldersFile(foldersFilePath);
    QVERIFY2(foldersFile.open(QIODevice::ReadOnly | QIODevice::Text), "folders hierarchy file must be readable");
    const QString foldersTextAfterExistingAssign = QString::fromUtf8(foldersFile.readAll());
    foldersFile.close();

    WhatSonFoldersHierarchyStore foldersStore;
    WhatSonFoldersHierarchyParser foldersParser;
    QVERIFY2(foldersParser.parse(foldersTextAfterExistingAssign, &foldersStore, &parseError, nullptr), qPrintable(parseError));
    QCOMPARE(foldersStore.folderEntries().size(), 1);
    QCOMPARE(foldersStore.folderEntries().constFirst().id, QStringLiteral("Knowledge"));
    QCOMPARE(foldersStore.folderEntries().constFirst().uuid, knowledgeUuid);

    QVERIFY(viewModel.assignFolderByName(QStringLiteral("Research/Ideas")));
    QCOMPARE(
        propertiesViewModel->property("folderItems").toStringList(),
        QStringList({QStringLiteral("Knowledge"), QStringLiteral("Ideas")}));
    QCOMPARE(propertiesViewModel->property("activeFolderIndex").toInt(), 1);

    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "header file must remain readable");
    headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();
    QVERIFY2(headerParser.parse(headerText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.folders(), QStringList({QStringLiteral("Knowledge"), QStringLiteral("Research/Ideas")}));
    QCOMPARE(header.folderUuids().size(), 2);
    QVERIFY(!header.folderUuids().at(1).trimmed().isEmpty());

    QVERIFY2(foldersFile.open(QIODevice::ReadOnly | QIODevice::Text), "folders hierarchy file must remain readable");
    const QString foldersTextAfterCreate = QString::fromUtf8(foldersFile.readAll());
    foldersFile.close();
    QVERIFY2(foldersParser.parse(foldersTextAfterCreate, &foldersStore, &parseError, nullptr), qPrintable(parseError));
    QCOMPARE(foldersStore.folderEntries().size(), 3);
    QCOMPARE(foldersStore.folderEntries().at(0).id, QStringLiteral("Knowledge"));
    QCOMPARE(foldersStore.folderEntries().at(0).uuid, knowledgeUuid);
    QCOMPARE(foldersStore.folderEntries().at(1).id, QStringLiteral("Research"));
    QCOMPARE(foldersStore.folderEntries().at(2).id, QStringLiteral("Research/Ideas"));
    QCOMPARE(foldersStore.folderEntries().at(2).uuid, header.folderUuids().at(1));

    QVERIFY(viewModel.assignFolderByName(QStringLiteral("Research/Ideas")));
    QVERIFY2(foldersFile.open(QIODevice::ReadOnly | QIODevice::Text), "folders hierarchy file must still be readable");
    const QString foldersTextAfterDuplicateAssign = QString::fromUtf8(foldersFile.readAll());
    foldersFile.close();
    QVERIFY2(foldersParser.parse(foldersTextAfterDuplicateAssign, &foldersStore, &parseError, nullptr), qPrintable(parseError));
    QCOMPARE(foldersStore.folderEntries().size(), 3);
}

void DetailPanelViewModelTest::requestStateChange_mustUpdateActiveStateAndToolbarSelection()
{
    DetailPanelViewModel viewModel;

    QSignalSpy activeStateSpy(&viewModel, &DetailPanelViewModel::activeStateChanged);
    QSignalSpy toolbarItemsSpy(&viewModel, &DetailPanelViewModel::toolbarItemsChanged);

    viewModel.requestStateChange(static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));

    QCOMPARE(activeStateSpy.count(), 1);
    QCOMPARE(toolbarItemsSpy.count(), 1);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Layer));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("layer"));

    const QVariantList toolbarItems = viewModel.toolbarItems();
    QCOMPARE(toolbarItems.size(), 6);
    for (int index = 0; index < toolbarItems.size(); ++index)
    {
        const QVariantMap item = toolbarItems.at(index).toMap();
        QCOMPARE(item.value(QStringLiteral("selected")).toBool(), index == 3);
    }
    QCOMPARE(toolbarItems.at(3).toMap().value(QStringLiteral("iconName")).toString(),
             QStringLiteral("toolwindowdependencies"));
    QCOMPARE(toolbarItems.at(4).toMap().value(QStringLiteral("iconName")).toString(),
             QStringLiteral("toolWindowClock"));
}

void DetailPanelViewModelTest::requestStateChange_invalidValue_mustBeIgnored()
{
    DetailPanelViewModel viewModel;

    QSignalSpy activeStateSpy(&viewModel, &DetailPanelViewModel::activeStateChanged);
    QSignalSpy toolbarItemsSpy(&viewModel, &DetailPanelViewModel::toolbarItemsChanged);

    viewModel.requestStateChange(999);

    QCOMPARE(activeStateSpy.count(), 0);
    QCOMPARE(toolbarItemsSpy.count(), 0);
    QCOMPARE(viewModel.activeState(), static_cast<int>(DetailPanelViewModel::DetailContentState::Properties));
    QCOMPARE(viewModel.activeStateName(), QStringLiteral("properties"));
}

QTEST_APPLESS_MAIN(DetailPanelViewModelTest)

#include "test_detail_panel_viewmodel.moc"
