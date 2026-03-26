#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"

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
    QVariantMap makeHierarchyEntry(const QString& key, const QString& label)
    {
        return QVariantMap{
            {QStringLiteral("key"), key},
            {QStringLiteral("label"), label}
        };
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
    QCOMPARE(projectSelector->property("hierarchyModel").toList(), sharedHierarchyModel);

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

    QCOMPARE(projectSelector->property("hierarchyModel").toList(), reorderedHierarchyModel);
    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 1);
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
                       "      <folder>Knowledge</folder>\n"
                       "    </folders>\n"
                       "    <project>Beta</project>\n"
                       "    <bookmarks state=\"true\" colors=\"amber\" />\n"
                       "    <tags>\n"
                       "      <tag>seed</tag>\n"
                       "    </tags>\n"
                       "    <progress enums=\"{Ready,Pending,InProgress,Done}\">2</progress>\n"
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
        makeHierarchyEntry(QStringLiteral("progress:ready"), QStringLiteral("Ready")),
        makeHierarchyEntry(QStringLiteral("progress:pending"), QStringLiteral("Pending")),
        makeHierarchyEntry(QStringLiteral("progress:inprogress"), QStringLiteral("InProgress")),
        makeHierarchyEntry(QStringLiteral("progress:done"), QStringLiteral("Done"))
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

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 1);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Beta"));
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QStringLiteral("amber"));
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), 2);

    QVERIFY(viewModel.writeProjectSelection(2));
    QVERIFY(viewModel.writeBookmarkSelection(0));
    QVERIFY(viewModel.writeProgressSelection(3));

    QCOMPARE(projectSelector->property("selectedIndex").toInt(), 2);
    QCOMPARE(bookmarkSelector->property("selectedIndex").toInt(), 0);
    QCOMPARE(progressSelector->property("selectedIndex").toInt(), 3);
    QCOMPARE(propertiesViewModel->property("currentProject").toString(), QStringLiteral("Gamma"));
    QCOMPARE(propertiesViewModel->property("currentBookmark").toString(), QStringLiteral("red"));
    QCOMPARE(propertiesViewModel->property("currentProgress").toInt(), 3);

    QFile headerFile(headerFilePath);
    QVERIFY2(headerFile.open(QIODevice::ReadOnly | QIODevice::Text), "persisted header file must be readable");
    const QString updatedHeaderText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    WhatSonNoteHeaderStore header;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(updatedHeaderText, &header, &parseError), qPrintable(parseError));
    QCOMPARE(header.project(), QStringLiteral("Gamma"));
    QCOMPARE(header.bookmarkColors(), QStringList{QStringLiteral("red")});
    QCOMPARE(header.progress(), 3);
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
