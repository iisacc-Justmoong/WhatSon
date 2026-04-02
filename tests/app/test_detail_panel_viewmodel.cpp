#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"
#include "file/hierarchy/WhatSonFolderIdentity.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"

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

    QStringList reloadedNoteIds() const
    {
        return m_reloadedNoteIds;
    }

    int reloadCallCount() const noexcept
    {
        return m_reloadedNoteIds.size();
    }

    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return false;
        }

        m_reloadedNoteIds.push_back(normalizedNoteId);
        return true;
    }

signals:
    void hierarchyModelChanged();
    void selectedIndexChanged();

private:
    QVariantList m_hierarchyModel;
    int m_selectedIndex = -1;
    QStringList m_reloadedNoteIds;
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

    void notifyItemsChanged()
    {
        emit itemsChanged();
    }

signals:
    void itemsChanged();
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

class UnsupportedDetailNoteContextSource final : public QObject
{
    Q_OBJECT
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
    void detailSelectors_mustReloadCurrentHeaderWhenNoteMetadataChanges();
    void detailSelectors_mustReloadCurrentHeaderWhenCurrentNoteIdChangesWithStableDirectoryPath();
    void detailSelectors_mustPreserveCurrentNoteContextAcrossUnsupportedHierarchySources();
    void detailWrites_mustSynchronizeActiveLibraryNoteListMetadata();
    void detailWrites_mustSynchronizeActiveBookmarksNoteListMetadata();
    void detailWrites_mustSynchronizeProjectsNoteListMetadataFromLibraryContext();
    void detailWrites_mustDropSelectedProjectsNoteImmediatelyWhenProjectCleared();
    void detailWrites_mustSynchronizeActiveProgressNoteListMetadata();
    void detailProgressSelection_mustWriteCurrentModelValueForProgressHierarchyQueries();
    void detailProgressSelection_withoutSource_mustFallbackToHeaderEnumsAndCanonicalValueMapping();
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
            QStringLiteral("progress:9"),
            QStringLiteral("Ship"),
            QVariantMap{{QStringLiteral("itemId"), 9}}),
        makeHierarchyEntry(
            QStringLiteral("progress:4"),
            QStringLiteral("Queued"),
            QVariantMap{{QStringLiteral("itemId"), 4}}),
        makeHierarchyEntry(
            QStringLiteral("progress:11"),
            QStringLiteral("Published"),
            QVariantMap{{QStringLiteral("itemId"), 11}}),
        makeHierarchyEntry(
            QStringLiteral("progress:7"),
            QStringLiteral("Review"),
            QVariantMap{{QStringLiteral("itemId"), 7}})
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
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 4);
    QCOMPARE(
        propertiesViewModel->property("folderItems").toStringList(),
        QStringList{QStringLiteral("Knowledge")});
    QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{QStringLiteral("seed")});
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Beta"));
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QStringLiteral("amber"));
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), 7);

    const QVariantList projectHierarchyModel = projectSelector->property("hierarchyModel").toList();
    const QVariantList bookmarkHierarchyModel = bookmarkSelector->property("hierarchyModel").toList();
    const QVariantList progressHierarchyModel = progressSelector->property("hierarchyModel").toList();
    QCOMPARE(projectHierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("No project"));
    QCOMPARE(bookmarkHierarchyModel.at(0).toMap().value(QStringLiteral("label")).toString(), QStringLiteral("No bookmark"));
    QCOMPARE(
        progressHierarchyModel,
        detailSelectorHierarchyModel(
            QStringLiteral("detail:none:progress"),
            QStringLiteral("No progress"),
            QVariantList{
                makeHierarchyEntry(
                    QStringLiteral("progress:9"),
                    QStringLiteral("Ship"),
                    QVariantMap{{QStringLiteral("itemId"), 9}}),
                makeHierarchyEntry(
                    QStringLiteral("progress:4"),
                    QStringLiteral("Queued"),
                    QVariantMap{{QStringLiteral("itemId"), 4}}),
                makeHierarchyEntry(
                    QStringLiteral("progress:11"),
                    QStringLiteral("Published"),
                    QVariantMap{{QStringLiteral("itemId"), 11}}),
                makeHierarchyEntry(
                    QStringLiteral("progress:7"),
                    QStringLiteral("Review"),
                    QVariantMap{{QStringLiteral("itemId"), 7}})
            },
            QVariantMap{{QStringLiteral("progressValue"), -1}}));

    QVERIFY(viewModel.writeProjectSelection(3));
    QVERIFY(viewModel.writeBookmarkSelection(1));
    QVERIFY(viewModel.writeProgressSelection(1));
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 3);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 1);
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
    QCOMPARE(
        header.progressEnums(),
        QStringList({
            QStringLiteral("Queued"),
            QStringLiteral("Review"),
            QStringLiteral("Ship"),
            QStringLiteral("Published")
        }));
    QCOMPARE(header.progress(), -1);
}

void DetailPanelViewModelTest::detailSelectors_mustReloadCurrentHeaderWhenNoteMetadataChanges()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString noteId = QStringLiteral("ReloadNote");
    const QString noteDirectoryPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("ReloadNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString headerFilePath = QDir(noteDirectoryPath).filePath(QStringLiteral("ReloadNote.wsnhead"));
    const QString initialHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"ReloadNote\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "      <folder>Archive/Knowledge</folder>\n"
                       "    </folders>\n"
                       "    <project>Beta</project>\n"
                       "    <bookmarks state=\"false\" colors=\"\" />\n"
                       "    <tags>\n"
                       "      <tag>seed</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Queued,Review,Ship,Published}\">7</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, initialHeaderText), "initial header file must be written");

    DetailPanelViewModel viewModel;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;
    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    viewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    noteListModel.setCurrentNoteId(noteId);
    viewModel.setCurrentNoteListModel(&noteListModel);

    QObject* propertiesViewModel = viewModel.propertiesViewModel();
    QVERIFY(propertiesViewModel != nullptr);
    QCOMPARE(
        propertiesViewModel->property("folderItems").toStringList(),
        QStringList{QStringLiteral("Knowledge")});

    const QString updatedHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"ReloadNote\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "      <folder>Workspace/Knowledge</folder>\n"
                       "      <folder>Research/Ideas</folder>\n"
                       "    </folders>\n"
                       "    <project>Beta</project>\n"
                       "    <bookmarks state=\"false\" colors=\"\" />\n"
                       "    <tags>\n"
                       "      <tag>seed</tag>\n"
                       "      <tag>updated</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Queued,Review,Ship,Published}\">7</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, updatedHeaderText), "updated header file must be written");

    noteListModel.notifyItemsChanged();

    QCOMPARE(
        propertiesViewModel->property("folderItems").toStringList(),
        QStringList({QStringLiteral("Knowledge"), QStringLiteral("Ideas")}));
    QCOMPARE(
        propertiesViewModel->property("tagItems").toStringList(),
        QStringList({QStringLiteral("seed"), QStringLiteral("updated")}));
}

void DetailPanelViewModelTest::detailSelectors_mustReloadCurrentHeaderWhenCurrentNoteIdChangesWithStableDirectoryPath()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString firstNoteId = QStringLiteral("SharedFirst");
    const QString secondNoteId = QStringLiteral("SharedSecond");
    const QString noteDirectoryPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("Shared.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString headerFilePath = QDir(noteDirectoryPath).filePath(QStringLiteral("Shared.wsnhead"));
    const QString firstHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"SharedFirst\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "      <folder>Workspace/Alpha</folder>\n"
                       "    </folders>\n"
                       "    <project>One</project>\n"
                       "    <bookmarks state=\"false\" colors=\"\" />\n"
                       "    <tags>\n"
                       "      <tag>first</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Queued,Review,Ship,Published}\">1</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, firstHeaderText), "first shared header file must be written");

    DetailPanelViewModel viewModel;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;

    noteDirectorySourceViewModel.setNoteDirectoryPath(firstNoteId, noteDirectoryPath);
    noteDirectorySourceViewModel.setNoteDirectoryPath(secondNoteId, noteDirectoryPath);
    viewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    noteListModel.setCurrentNoteId(firstNoteId);
    viewModel.setCurrentNoteListModel(&noteListModel);

    QObject* propertiesViewModel = viewModel.propertiesViewModel();
    QVERIFY(propertiesViewModel != nullptr);
    QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{QStringLiteral("Alpha")});
    QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{QStringLiteral("first")});
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("One"));

    const QString secondHeaderText =
        QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE WHATSONNOTE>\n"
                       "<contents id=\"SharedSecond\">\n"
                       "  <head>\n"
                       "    <meta charset=\"UTF-8\" />\n"
                       "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                       "    <folders>\n"
                       "      <folder>Workspace/Beta</folder>\n"
                       "    </folders>\n"
                       "    <project>Two</project>\n"
                       "    <bookmarks state=\"false\" colors=\"\" />\n"
                       "    <tags>\n"
                       "      <tag>second</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Queued,Review,Ship,Published}\">2</progress>\n"
                       "    <isPreset>false</isPreset>\n"
                       "  </head>\n"
                       "</contents>\n");
    QVERIFY2(writeUtf8File(headerFilePath, secondHeaderText), "second shared header file must be written");

    noteListModel.setCurrentNoteId(secondNoteId);

    QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{QStringLiteral("Beta")});
    QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{QStringLiteral("second")});
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Two"));
}

void DetailPanelViewModelTest::detailSelectors_mustPreserveCurrentNoteContextAcrossUnsupportedHierarchySources()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString noteId = QStringLiteral("StickyNote");
    const QString noteDirectoryPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("StickyNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString headerFilePath = QDir(noteDirectoryPath).filePath(QStringLiteral("StickyNote.wsnhead"));
    QVERIFY2(
        writeUtf8File(
            headerFilePath,
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<!DOCTYPE WHATSONNOTE>\n"
                           "<contents id=\"StickyNote\">\n"
                           "  <head>\n"
                           "    <meta charset=\"UTF-8\" />\n"
                           "    <meta name=\"wsn-type\" content=\"wsnhead\" />\n"
                           "    <project>Alpha</project>\n"
                           "    <bookmarks state=\"true\" colors=\"amber\" />\n"
                           "    <progress enums=\"{First draft,Modified draft,In progress,Pending,Reviewing,Waiting for approval,Done,Lagacy,Archived,Delete review}\">6</progress>\n"
                           "    <isPreset>false</isPreset>\n"
                           "  </head>\n"
                           "</contents>\n")),
        "sticky header file must be written");

    DetailPanelViewModel viewModel;
    FakeDetailHierarchySourceViewModel projectsSource;
    FakeDetailHierarchySourceViewModel bookmarksSource;
    FakeDetailHierarchySourceViewModel progressSource;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;
    UnsupportedDetailNoteContextSource unsupportedSource;

    projectsSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("project:alpha"), QStringLiteral("Alpha")),
        makeHierarchyEntry(QStringLiteral("project:beta"), QStringLiteral("Beta"))
    });
    bookmarksSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("bookmark:red"), QStringLiteral("Red")),
        makeHierarchyEntry(QStringLiteral("bookmark:amber"), QStringLiteral("Amber"))
    });
    progressSource.setHierarchyModel({
        makeHierarchyEntry(
            QStringLiteral("progress:0"),
            QStringLiteral("First draft"),
            QVariantMap{{QStringLiteral("itemId"), 0}}),
        makeHierarchyEntry(
            QStringLiteral("progress:6"),
            QStringLiteral("Done"),
            QVariantMap{{QStringLiteral("itemId"), 6}}),
        makeHierarchyEntry(
            QStringLiteral("progress:8"),
            QStringLiteral("Archived"),
            QVariantMap{{QStringLiteral("itemId"), 8}})
    });

    viewModel.setProjectSelectionSourceViewModel(&projectsSource);
    viewModel.setBookmarkSelectionSourceViewModel(&bookmarksSource);
    viewModel.setProgressSelectionSourceViewModel(&progressSource);

    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    viewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);
    noteListModel.setCurrentNoteId(noteId);
    viewModel.setCurrentNoteListModel(&noteListModel);

    QObject* projectSelector = viewModel.projectSelectionViewModel();
    QObject* bookmarkSelector = viewModel.bookmarkSelectionViewModel();
    QObject* progressSelector = viewModel.progressSelectionViewModel();

    QVERIFY(projectSelector != nullptr);
    QVERIFY(bookmarkSelector != nullptr);
    QVERIFY(progressSelector != nullptr);
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 2);

    viewModel.setCurrentNoteListModel(nullptr);
    viewModel.setCurrentNoteDirectorySourceViewModel(&unsupportedSource);

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 2);

    QVERIFY(viewModel.writeProjectSelection(2));
    QVERIFY(viewModel.writeBookmarkSelection(1));
    QVERIFY(viewModel.writeProgressSelection(3));

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 3);

    QFile headerFile(headerFilePath);
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "sticky header file must remain readable");
    const QString updatedHeaderText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(updatedHeaderText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.project(), QStringLiteral("Beta"));
    QVERIFY(header.isBookmarked());
    QCOMPARE(header.bookmarkColors(), QStringList{QStringLiteral("red")});
    QCOMPARE(header.progress(), 8);
}

void DetailPanelViewModelTest::detailWrites_mustSynchronizeActiveLibraryNoteListMetadata()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailMirrorLibrary.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailMirrorLibrary.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("MirrorNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-library\"}\n"
                "  ]\n"
                "}\n")),
        "index.wsnindex must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("MirrorNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-library\">\n"
                "  <head>\n"
                "    <created>2026-03-01-00-00-00</created>\n"
                "    <author>Tester</author>\n"
                "    <lastModified>2026-03-01-00-00-00</lastModified>\n"
                "    <modifiedBy>Tester</modifiedBy>\n"
                "    <folders>\n"
                "      <folder>Knowledge</folder>\n"
                "    </folders>\n"
                "    <project></project>\n"
                "    <bookmarks state=\"false\" colors=\"\" />\n"
                "    <tags>\n"
                "      <tag>seed</tag>\n"
                "    </tags>\n"
                "    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n"
                "    <isPreset>false</isPreset>\n"
                "  </head>\n"
                "</contents>\n")),
        "MirrorNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("MirrorNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-library\">\n"
                "  <body>\n"
                "    <paragraph>Mirror library note</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "MirrorNote.wsnbody must be written");

    WhatSonFoldersHierarchyStore foldersStore;
    foldersStore.setFolderEntries({
        WhatSonFolderDepthEntry{
            QStringLiteral("Knowledge"),
            QStringLiteral("Knowledge"),
            0,
            WhatSon::FolderIdentity::createFolderUuid()
        }
    });
    QString foldersWriteError;
    QVERIFY2(
        foldersStore.writeToFile(QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders")), &foldersWriteError),
        qPrintable(foldersWriteError));

    LibraryHierarchyViewModel libraryHierarchyViewModel;
    QVERIFY(libraryHierarchyViewModel.loadFromWshub(hubPath));
    LibraryNoteListModel* noteListModel = libraryHierarchyViewModel.noteListModel();
    QVERIFY(noteListModel != nullptr);
    QCOMPARE(noteListModel->rowCount(), 1);
    noteListModel->setCurrentIndex(0);
    QCOMPARE(noteListModel->currentNoteId(), QStringLiteral("note-library"));

    DetailPanelViewModel detailPanelViewModel;
    FakeDetailHierarchySourceViewModel bookmarksSource;
    bookmarksSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("bookmark:red"), QStringLiteral("Red")),
        makeHierarchyEntry(QStringLiteral("bookmark:amber"), QStringLiteral("Amber"))
    });
    detailPanelViewModel.setBookmarkSelectionSourceViewModel(&bookmarksSource);
    detailPanelViewModel.setCurrentNoteListModel(noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&libraryHierarchyViewModel);

    QObject* propertiesViewModel = detailPanelViewModel.propertiesViewModel();
    QVERIFY(propertiesViewModel != nullptr);
    QVERIFY(detailPanelViewModel.assignFolderByName(QStringLiteral("Research/Ideas")));
    QVERIFY(propertiesViewModel->setProperty("activeTagIndex", 0));
    QVERIFY(detailPanelViewModel.removeActiveTag());
    QVERIFY(detailPanelViewModel.writeBookmarkSelection(1));

    const QModelIndex noteIndex = noteListModel->index(0, 0);
    QCOMPARE(
        noteListModel->data(noteIndex, LibraryNoteListModel::FoldersRole).toStringList(),
        QStringList({QStringLiteral("Knowledge"), QStringLiteral("Research/Ideas")}));
    QCOMPARE(noteListModel->data(noteIndex, LibraryNoteListModel::TagsRole).toStringList(), QStringList{});
    QVERIFY(noteListModel->data(noteIndex, LibraryNoteListModel::BookmarkedRole).toBool());
    QCOMPARE(
        noteListModel->data(noteIndex, LibraryNoteListModel::BookmarkColorRole).toString(),
        WhatSon::Bookmarks::bookmarkColorToHex(QStringLiteral("red")));
}

void DetailPanelViewModelTest::detailWrites_mustSynchronizeActiveBookmarksNoteListMetadata()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailMirrorBookmarks.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailMirrorBookmarks.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("MarkedNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-bookmark\"}\n"
                "  ]\n"
                "}\n")),
        "index.wsnindex must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("MarkedNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-bookmark\">\n"
                "  <head>\n"
                "    <created>2026-03-01-00-00-00</created>\n"
                "    <author>Tester</author>\n"
                "    <lastModified>2026-03-01-00-00-00</lastModified>\n"
                "    <modifiedBy>Tester</modifiedBy>\n"
                "    <folders></folders>\n"
                "    <project></project>\n"
                "    <bookmarks state=\"true\" colors=\"red\" />\n"
                "    <tags>\n"
                "    </tags>\n"
                "    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n"
                "    <isPreset>false</isPreset>\n"
                "  </head>\n"
                "</contents>\n")),
        "MarkedNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("MarkedNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-bookmark\">\n"
                "  <body>\n"
                "    <paragraph>Bookmark note</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "MarkedNote.wsnbody must be written");

    BookmarksHierarchyViewModel bookmarksHierarchyViewModel;
    QVERIFY(bookmarksHierarchyViewModel.loadFromWshub(hubPath));
    BookmarksNoteListModel* noteListModel = bookmarksHierarchyViewModel.noteListModel();
    QVERIFY(noteListModel != nullptr);
    QCOMPARE(noteListModel->rowCount(), 1);
    noteListModel->setCurrentIndex(0);
    QCOMPARE(noteListModel->currentNoteId(), QStringLiteral("note-bookmark"));

    DetailPanelViewModel detailPanelViewModel;
    detailPanelViewModel.setBookmarkSelectionSourceViewModel(&bookmarksHierarchyViewModel);
    detailPanelViewModel.setCurrentNoteListModel(noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&bookmarksHierarchyViewModel);

    QVERIFY(detailPanelViewModel.writeBookmarkSelection(0));
    QCOMPARE(noteListModel->rowCount(), 0);
}

void DetailPanelViewModelTest::detailWrites_mustSynchronizeProjectsNoteListMetadataFromLibraryContext()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailMirrorProjects.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailMirrorProjects.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("ProjectNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-project\"}\n"
                "  ]\n"
                "}\n")),
        "index.wsnindex must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.projects.list\",\n"
                "  \"projects\": [\n"
                "    {\"id\": \"Alpha\", \"label\": \"Alpha\"},\n"
                "    {\"id\": \"Beta\", \"label\": \"Beta\"}\n"
                "  ]\n"
                "}\n")),
        "ProjectLists.wsproj must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-project\">\n"
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
        "ProjectNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-project\">\n"
                "  <body>\n"
                "    <paragraph>Project note</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "ProjectNote.wsnbody must be written");

    LibraryHierarchyViewModel libraryHierarchyViewModel;
    QVERIFY(libraryHierarchyViewModel.loadFromWshub(hubPath));
    LibraryNoteListModel* libraryNoteListModel = libraryHierarchyViewModel.noteListModel();
    QVERIFY(libraryNoteListModel != nullptr);
    QCOMPARE(libraryNoteListModel->rowCount(), 1);
    libraryNoteListModel->setCurrentIndex(0);
    QCOMPARE(libraryNoteListModel->currentNoteId(), QStringLiteral("note-project"));

    FakeDetailHierarchySourceViewModel projectSource;
    projectSource.setHierarchyModel({
        makeHierarchyEntry(QStringLiteral("project:alpha"), QStringLiteral("Alpha")),
        makeHierarchyEntry(QStringLiteral("project:beta"), QStringLiteral("Beta"))
    });

    DetailPanelViewModel detailPanelViewModel;
    detailPanelViewModel.setProjectSelectionSourceViewModel(&projectSource);
    detailPanelViewModel.setCurrentNoteListModel(libraryNoteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&libraryHierarchyViewModel);

    QVERIFY(detailPanelViewModel.writeProjectSelection(1));
    QCOMPARE(projectSource.reloadCallCount(), 1);
    QCOMPARE(projectSource.reloadedNoteIds(), QStringList{QStringLiteral("note-project")});
    QVERIFY(detailPanelViewModel.writeProjectSelection(1));
    QCOMPARE(projectSource.reloadCallCount(), 1);
    QCOMPARE(projectSource.reloadedNoteIds(), QStringList{QStringLiteral("note-project")});

    QFile headerFile(QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnhead")));
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "ProjectNote.wsnhead must remain readable");
    const QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.project(), QStringLiteral("Alpha"));
}

void DetailPanelViewModelTest::detailWrites_mustDropSelectedProjectsNoteImmediatelyWhenProjectCleared()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailProjectsImmediateRefresh.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailProjectsImmediateRefresh.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("ProjectNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-project\"}\n"
                "  ]\n"
                "}\n")),
        "index.wsnindex must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(contentsPath).filePath(QStringLiteral("ProjectLists.wsproj")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.projects.list\",\n"
                "  \"projects\": [\n"
                "    {\"id\": \"Untitled\", \"label\": \"Untitled\"}\n"
                "  ]\n"
                "}\n")),
        "ProjectLists.wsproj must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-project\">\n"
                "  <head>\n"
                "    <created>2026-03-01-00-00-00</created>\n"
                "    <author>Tester</author>\n"
                "    <lastModified>2026-03-01-00-00-00</lastModified>\n"
                "    <modifiedBy>Tester</modifiedBy>\n"
                "    <folders></folders>\n"
                "    <project>Untitled</project>\n"
                "    <bookmarks state=\"false\" />\n"
                "    <tags>\n"
                "    </tags>\n"
                "    <progress enums=\"{Ready,Pending,InProgress,Done}\">0</progress>\n"
                "    <isPreset>false</isPreset>\n"
                "  </head>\n"
                "</contents>\n")),
        "ProjectNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-project\">\n"
                "  <body>\n"
                "    <paragraph>Project note</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "ProjectNote.wsnbody must be written");

    ProjectsHierarchyViewModel projectsHierarchyViewModel;
    QString errorMessage;
    QVERIFY2(projectsHierarchyViewModel.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));

    const auto noteCountByLabel = [&projectsHierarchyViewModel](const QString& label) -> int
    {
        const QVariantList hierarchyModel = projectsHierarchyViewModel.hierarchyModel();
        for (const QVariant& entry : hierarchyModel)
        {
            const QVariantMap node = entry.toMap();
            if (node.value(QStringLiteral("label")).toString() != label)
            {
                continue;
            }

            bool ok = false;
            const int count = node.value(QStringLiteral("count")).toInt(&ok);
            if (!ok)
            {
                return -1;
            }
            return count;
        }
        return -1;
    };

    QVERIFY(noteCountByLabel(QStringLiteral("Untitled")) == 1);
    projectsHierarchyViewModel.setSelectedIndex(0);
    LibraryNoteListModel* noteListModel = projectsHierarchyViewModel.noteListModel();
    QVERIFY(noteListModel != nullptr);
    QCOMPARE(noteListModel->rowCount(), 1);
    noteListModel->setCurrentIndex(0);
    QCOMPARE(noteListModel->currentNoteId(), QStringLiteral("note-project"));

    DetailPanelViewModel detailPanelViewModel;
    detailPanelViewModel.setProjectSelectionSourceViewModel(&projectsHierarchyViewModel);
    detailPanelViewModel.setCurrentNoteListModel(noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&projectsHierarchyViewModel);

    QObject* projectSelector = detailPanelViewModel.projectSelectionViewModel();
    QVERIFY(projectSelector != nullptr);
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 1);

    QVERIFY(detailPanelViewModel.writeProjectSelection(0));
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(noteListModel->rowCount(), 0);
    QCOMPARE(noteCountByLabel(QStringLiteral("Untitled")), 0);

    QFile headerFile(QDir(noteDirectoryPath).filePath(QStringLiteral("ProjectNote.wsnhead")));
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "ProjectNote.wsnhead must remain readable");
    const QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.project(), QString());
}

void DetailPanelViewModelTest::detailWrites_mustSynchronizeActiveProgressNoteListMetadata()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString hubPath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("DetailMirrorProgress.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("DetailMirrorProgress.wscontents"));
    const QString libraryPath = QDir(contentsPath).filePath(QStringLiteral("Library.wslibrary"));
    const QString noteDirectoryPath = QDir(libraryPath).filePath(QStringLiteral("ProgressNote.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(libraryPath).filePath(QStringLiteral("index.wsnindex")),
            QStringLiteral(
                "{\n"
                "  \"version\": 1,\n"
                "  \"schema\": \"whatson.library.index\",\n"
                "  \"notes\": [\n"
                "    {\"id\": \"note-progress\"}\n"
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
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProgressNote.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-progress\">\n"
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
        "ProgressNote.wsnhead must be written");
    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("ProgressNote.wsnbody")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-progress\">\n"
                "  <body>\n"
                "    <paragraph>Progress note</paragraph>\n"
                "  </body>\n"
                "</contents>\n")),
        "ProgressNote.wsnbody must be written");

    ProgressHierarchyViewModel progressHierarchyViewModel;
    QVERIFY(progressHierarchyViewModel.loadFromWshub(hubPath));
    progressHierarchyViewModel.setSelectedIndex(0);
    LibraryNoteListModel* noteListModel = progressHierarchyViewModel.noteListModel();
    QVERIFY(noteListModel != nullptr);
    QCOMPARE(noteListModel->rowCount(), 1);
    noteListModel->setCurrentIndex(0);
    QCOMPARE(noteListModel->currentNoteId(), QStringLiteral("note-progress"));

    DetailPanelViewModel detailPanelViewModel;
    detailPanelViewModel.setProgressSelectionSourceViewModel(&progressHierarchyViewModel);
    detailPanelViewModel.setCurrentNoteListModel(noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&progressHierarchyViewModel);

    QVERIFY(detailPanelViewModel.writeProgressSelection(4));
    QCOMPARE(noteListModel->rowCount(), 0);
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

void DetailPanelViewModelTest::detailProgressSelection_withoutSource_mustFallbackToHeaderEnumsAndCanonicalValueMapping()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY2(temporaryDirectory.isValid(), "temporary directory must be created");

    const QString noteId = QStringLiteral("note-progress-fallback");
    const QString noteDirectoryPath = QDir(temporaryDirectory.path()).filePath(
        QStringLiteral("FallbackProgress.wsnote"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    QVERIFY2(
        writeUtf8File(
            QDir(noteDirectoryPath).filePath(QStringLiteral("FallbackProgress.wsnhead")),
            QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTE>\n"
                "<contents id=\"note-progress-fallback\">\n"
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
        "FallbackProgress.wsnhead must be written");

    DetailPanelViewModel detailPanelViewModel;
    FakeCurrentNoteListModel noteListModel;
    FakeNoteDirectorySourceViewModel noteDirectorySourceViewModel;

    noteListModel.setCurrentNoteId(noteId);
    noteDirectorySourceViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);
    detailPanelViewModel.setCurrentNoteListModel(&noteListModel);
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&noteDirectorySourceViewModel);

    QObject* progressSelector = detailPanelViewModel.progressSelectionViewModel();
    QVERIFY(progressSelector != nullptr);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 1);

    const QVariantList progressHierarchyModel = progressSelector->property("hierarchyModel").toList();
    QCOMPARE(progressHierarchyModel.size(), 5);
    QCOMPARE(progressHierarchyModel.at(1).toMap().value(QStringLiteral("progressValue")).toInt(), 0);
    QCOMPARE(progressHierarchyModel.at(2).toMap().value(QStringLiteral("progressValue")).toInt(), 3);
    QCOMPARE(progressHierarchyModel.at(3).toMap().value(QStringLiteral("progressValue")).toInt(), 2);
    QCOMPARE(progressHierarchyModel.at(4).toMap().value(QStringLiteral("progressValue")).toInt(), 6);

    QVERIFY(detailPanelViewModel.writeProgressSelection(4));
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 4);
    QCOMPARE(detailPanelViewModel.propertiesViewModel()->property("currentProgress").toInt(), 6);

    QFile headerFile(QDir(noteDirectoryPath).filePath(QStringLiteral("FallbackProgress.wsnhead")));
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "FallbackProgress.wsnhead must remain readable");
    const QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.progress(), 6);
    QCOMPARE(
        header.progressEnums(),
        QStringList({
            QStringLiteral("Ready"),
            QStringLiteral("Pending"),
            QStringLiteral("InProgress"),
            QStringLiteral("Done")
        }));
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
