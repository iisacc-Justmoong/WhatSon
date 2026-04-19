#include "display/paper/ContentsA4PaperBackground.hpp"
#include "display/paper/ContentsPaperSelection.hpp"
#include "display/paper/ContentsTextFormatRenderer.hpp"
#include "display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/import/WhatSonClipboardResourceImportFileNamePolicy.hpp"
#include "file/viewer/ResourceBitmapViewer.hpp"
#define private public
#include "file/note/ContentsNoteManagementCoordinator.hpp"
#undef private
#include "file/note/WhatSonLocalNoteFileStore.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteHeaderCreator.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"
#include "file/statistic/WhatSonNoteFileStatSupport.hpp"
#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "runtime/scheduler/WhatSonCronExpression.hpp"
#include "runtime/scheduler/WhatSonUnixTimeAnalyzer.hpp"
#include "sensor/MonthlyUnusedNote.hpp"
#include "sensor/UnusedResourcesSensor.hpp"
#include "sensor/WeeklyUnusedNote.hpp"
#include "store/hub/SelectedHubStore.hpp"
#include "store/sidebar/ISidebarSelectionStore.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsEditorSessionController.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
#include "viewmodel/content/ContentsStructuredDocumentBlocksModel.hpp"
#include "viewmodel/content/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "viewmodel/content/ContentsStructuredDocumentHost.hpp"
#include "viewmodel/content/ContentsStructuredDocumentMutationPolicy.hpp"
#include "viewmodel/content/ContentsResourceTagTextGenerator.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteListModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewModeViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewSectionViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewState.hpp"
#include "viewmodel/navigationbar/NavigationModeSectionViewModel.hpp"
#include "viewmodel/navigationbar/NavigationModeState.hpp"
#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"
#include "viewmodel/onboarding/IOnboardingHubController.hpp"
#include "viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
#include "viewmodel/detailPanel/session/WhatSonFoldersHierarchySessionService.hpp"
#include "viewmodel/detailPanel/ResourceDetailPanelViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesListModel.hpp"
#include "viewmodel/panel/NoteListModelContractBridge.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/IActiveHierarchyContextSource.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QJSEngine>
#include <QJSValue>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <cmath>
#include <memory>

class FakeHierarchyViewModel final : public IHierarchyViewModel
{
    Q_OBJECT

public:
    explicit FakeHierarchyViewModel(const QString& labelPrefix, QObject* parent = nullptr)
        : IHierarchyViewModel(parent)
        , m_labelPrefix(labelPrefix)
        , m_itemModel(new QObject(this))
        , m_noteListModel(new QObject(this))
    {
        initializeHierarchyInterfaceSignalBridge();
    }

    QObject* itemModel() noexcept override
    {
        return m_itemModel;
    }

    QObject* noteListModel() noexcept override
    {
        return m_noteListModel;
    }

    void setNoteListModelObject(QObject* noteListModel)
    {
        m_noteListModel = noteListModel;
    }

    int selectedIndex() const noexcept override
    {
        return m_selectedIndex;
    }

    void setSelectedIndex(int index) override
    {
        if (m_selectedIndex == index)
        {
            return;
        }

        m_selectedIndex = index;
        emit selectedIndexChanged();
    }

    int itemCount() const noexcept override
    {
        return static_cast<int>(m_nodes.size());
    }

    bool loadSucceeded() const noexcept override
    {
        return m_loadSucceeded;
    }

    QString lastLoadError() const override
    {
        return m_lastLoadError;
    }

    QVariantList hierarchyModel() const override
    {
        return m_nodes;
    }

    QString itemLabel(int index) const override
    {
        return QStringLiteral("%1-%2").arg(m_labelPrefix).arg(index);
    }

    void setNodes(const QVariantList& nodes)
    {
        m_nodes = nodes;
        emit hierarchyModelChanged();
        emit itemCountChanged();
    }

signals:
    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();

private:
    QString m_labelPrefix;
    QObject* m_itemModel = nullptr;
    QObject* m_noteListModel = nullptr;
    QVariantList m_nodes;
    int m_selectedIndex = -1;
    bool m_loadSucceeded = true;
    QString m_lastLoadError;
};

class FakeSidebarSelectionStore final : public ISidebarSelectionStore
{
    Q_OBJECT

public:
    explicit FakeSidebarSelectionStore(QObject* parent = nullptr)
        : ISidebarSelectionStore(parent)
    {
    }

    int selectedHierarchyIndex() const noexcept override
    {
        return m_selectedHierarchyIndex;
    }

    void setSelectedHierarchyIndex(int index) override
    {
        if (m_selectedHierarchyIndex == index)
        {
            return;
        }

        m_selectedHierarchyIndex = index;
        emit selectedHierarchyIndexChanged();
    }

private:
    int m_selectedHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
};

class FakeOnboardingHubController final : public IOnboardingHubController
{
    Q_OBJECT

public:
    explicit FakeOnboardingHubController(QObject* parent = nullptr)
        : IOnboardingHubController(parent)
    {
    }

    void beginWorkspaceTransition() override
    {
        ++beginCount;
    }

    void completeWorkspaceTransition() override
    {
        ++completeCount;
    }

    void failWorkspaceTransition(const QString& message) override
    {
        ++failCount;
        lastFailureMessage = message;
    }

    int beginCount = 0;
    int completeCount = 0;
    int failCount = 0;
    QString lastFailureMessage;
};

class FakeContentPersistenceViewModel final : public QObject
{
    Q_OBJECT

public:
    explicit FakeContentPersistenceViewModel(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    Q_INVOKABLE QString noteDirectoryPathForNoteId(const QString& noteId) const
    {
        return m_noteDirectoryPaths.value(noteId.trimmed());
    }

    Q_INVOKABLE bool applyPersistedBodyStateForNote(
        const QString& noteId,
        const QString& bodyPlainText,
        const QString& bodySourceText,
        const QString& lastModifiedAt)
    {
        ++applyPersistedBodyStateCallCount;
        lastAppliedNoteId = noteId.trimmed();
        lastAppliedBodyPlainText = bodyPlainText;
        lastAppliedBodySourceText = bodySourceText;
        lastAppliedLastModifiedAt = lastModifiedAt;
        return true;
    }

    Q_INVOKABLE bool reloadNoteMetadataForNoteId(const QString& noteId)
    {
        ++reloadNoteMetadataCallCount;
        lastReloadedNoteId = noteId.trimmed();
        return true;
    }

    void setNoteDirectoryPath(const QString& noteId, const QString& noteDirectoryPath)
    {
        m_noteDirectoryPaths.insert(noteId.trimmed(), QDir::cleanPath(noteDirectoryPath.trimmed()));
    }

    int applyPersistedBodyStateCallCount = 0;
    int reloadNoteMetadataCallCount = 0;
    QString lastAppliedNoteId;
    QString lastAppliedBodyPlainText;
    QString lastAppliedBodySourceText;
    QString lastAppliedLastModifiedAt;
    QString lastReloadedNoteId;

private:
    QHash<QString, QString> m_noteDirectoryPaths;
};

class FakeSelectionNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool noteBacked READ noteBacked WRITE setNoteBacked NOTIFY noteBackedChanged)

public:
    explicit FakeSelectionNoteListModel(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    int currentIndex() const noexcept
    {
        return m_currentIndex;
    }

    void setCurrentIndex(const int currentIndex)
    {
        const int normalizedIndex = std::max(-1, currentIndex);
        if (m_currentIndex == normalizedIndex)
        {
            return;
        }

        m_currentIndex = normalizedIndex;
        emit currentIndexChanged();
    }

    QString currentNoteId() const
    {
        return m_currentNoteId;
    }

    void setCurrentNoteId(QString noteId)
    {
        noteId = noteId.trimmed();
        if (m_currentNoteId == noteId)
        {
            return;
        }

        m_currentNoteId = std::move(noteId);
        emit currentNoteIdChanged();
    }

    QString searchText() const
    {
        return m_searchText;
    }

    void setSearchText(QString searchText)
    {
        searchText = searchText.trimmed();
        if (m_searchText == searchText)
        {
            return;
        }

        m_searchText = std::move(searchText);
        emit searchTextChanged();
    }

    int itemCount() const noexcept
    {
        return m_itemCount;
    }

    void setItemCount(int itemCount)
    {
        itemCount = std::max(0, itemCount);
        if (m_itemCount == itemCount)
        {
            return;
        }

        m_itemCount = itemCount;
        emit itemCountChanged(m_itemCount);
    }

    bool noteBacked() const noexcept
    {
        return m_noteBacked;
    }

    void setNoteBacked(bool noteBacked)
    {
        if (m_noteBacked == noteBacked)
        {
            return;
        }

        m_noteBacked = noteBacked;
        emit noteBackedChanged();
    }

signals:
    void currentIndexChanged();
    void currentNoteIdChanged();
    void searchTextChanged();
    void itemCountChanged(int itemCount);
    void noteBackedChanged();

private:
    int m_currentIndex = -1;
    QString m_currentNoteId;
    QString m_searchText;
    int m_itemCount = 0;
    bool m_noteBacked = true;
};

class ScopedQSettingsSandbox final
{
public:
    explicit ScopedQSettingsSandbox(
        const QString& settingsRootPath,
        QString organizationName = QStringLiteral("WhatSonTests"),
        QString applicationName = QStringLiteral("WhatSonCppRegression"))
        : m_previousOrganizationName(QCoreApplication::organizationName())
        , m_previousApplicationName(QCoreApplication::applicationName())
    {
        QCoreApplication::setOrganizationName(std::move(organizationName));
        QCoreApplication::setApplicationName(std::move(applicationName));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsRootPath);

        QSettings settings;
        settings.clear();
        settings.sync();
    }

    ~ScopedQSettingsSandbox()
    {
        QSettings settings;
        settings.clear();
        settings.sync();

        QCoreApplication::setOrganizationName(m_previousOrganizationName);
        QCoreApplication::setApplicationName(m_previousApplicationName);
    }

private:
    QString m_previousOrganizationName;
    QString m_previousApplicationName;
};

class WhatSonCppRegressionTests final : public QObject
{
    Q_OBJECT

private slots:
    void selectedHubStore_persistsNormalizedSelectionsWithinSandboxedSettings();
    void sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals();
    void hierarchyViewModelProvider_normalizesMappingsAndAvoidsDuplicateSignals();
    void sidebarHierarchyViewModel_preservesFallbackAcrossStoreAttachDetach();
    void sidebarHierarchyViewModel_reactsToProviderMappingChanges();
    void sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings();
    void noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately();
    void noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches();
    void navigationModeViewModel_cyclesActiveSections();
    void editorViewModeViewModel_cyclesActiveSections();
    void onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle();
    void embeddedOnboardingRoutePages_avoidStackViewAnchorConflicts();
    void mainQml_embeddedStartup_dropsWatchdogRecoveryScaffold();
    void mainQml_iosInlineOnboarding_pinsPresentationToWorkspaceRoute();
    void mainCpp_iosStartup_suppressesAutomaticOnboarding();
    void iosInlineOnboardingSequence_reusesSharedOnboardingSurface();
    void onboardingContent_mobileLayout_avoidsFullscreenAntialiasedWindowFrame();
    void onboardingContent_saveDialog_doesNotPreselectMissingHubFile();
    void mainQml_exposesZeroWindowPaddingForLvrsApplicationWindow();
    void clipboardImportFileNamePolicy_generatesRandom32CharacterAlphaNumericPngNames();
    void unixTimeAnalyzer_reportsStableEpochFields();
    void cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication();
    void hierarchyTreeItemSupport_clampsNegativeSelectionToFirstVisibleRow();
    void progressHierarchySupport_defaultsFirstVisibleItemToFirstDraft();
    void resourcePackageSupport_roundTripsAnnotationMetadataAndBitmap();
    void unusedResourcesSensor_reportsHubPackagesMissingFromAllNoteEmbeddings();
    void unusedResourcesSensor_refreshesAfterRawBodyEmbedsAResource();
    void resourcesImportViewModel_wiresAnnotationBitmapGenerationIntoPackageCreation();
    void resourceTagTextGenerator_and_noteFolderSemantics_normalizeDescriptorsAndXml();
    void foldersHierarchyParser_escapesLiteralSlashLabelsIntoSingleSegments();
    void foldersHierarchySessionService_preservesEscapedLiteralSlashFolderPaths();
    void sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels();
    void resourcesHierarchyViewModel_defaultsSelectionToImageAndFiltersList();
    void structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches();
    void structuredMutationPolicy_buildsDeletionAndInsertionPayloads();
    void structuredMutationPolicy_buildsParagraphBoundaryMergeAndSplitPayloads();
    void structuredDocumentBlocksModel_updatesRowsWithoutResettingStableSuffixBlocks();
    void structuredDocumentBlocksModel_removesOnlyChangedMiddleRows();
    void structuredDocumentHost_tracksSelectionClearRevisionAcrossInteractions();
    void logicalLineLayoutSupport_mapsEditorRectanglesIntoBlockCoordinates();
    void logicalLineLayoutSupport_fallsBackWhenLiveEditorGeometryIsUnavailable();
    void editorSurfaceModeSupport_switchesToResourceEditorForResourceListModels();
    void qmlResourceEditorView_staysTransparentAndViewerOnly();
    void resourceDetailPanelViewModel_tracksCurrentResourceSelection();
    void detailPanelRouting_separatesNoteAndResourceViewsAndViewModels();
    void contentsDisplayView_invalidatesGutterGeometryImmediatelyAcrossRapidNoteSwitches();
    void contentsDisplayView_keepsGutterNumbersCloseToTheEditorBody();
    void contentsDisplayView_reservesHalfHeightBottomInsetAndCorrectsTypingViewport();
    void inlineFormatEditor_preservesMacModifierVerticalNavigationHooks();
    void structuredFlow_flattensImplicitTextBlocksIntoInteractiveGroups();
    void structuredEditorFormattingController_commitsInlineStyleMutationsThroughFlowRawRanges();
    void structuredEditors_routeMacModifierVerticalNavigationAcrossBlockAndDocumentBoundaries();
    void paperSelection_tracksChosenPaperEnumState();
    void a4PaperBackground_exposesCanonicalMetricsAndAnchorsPrintRendererDefaults();
    void textFormatRenderer_wrapsCommittedUrlsIntoCanonicalWebLinks();
    void textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml();
    void displayPaperModels_hostPageAndPrintViewModeObjectsUnderModelsDirectory();
    void noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks();
    void logicalTextBridge_advancesCursorPastClosingWebLinkTag();
    void qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode();
    void qmlEditors_routeRenderedHyperlinksToExternalBrowser();
    void resourceBitmapViewer_projectsRenderableImagePreviewState();
    void editorSessionController_preservesLocalEditorAuthorityAgainstSameNoteModelSync();
    void noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred();
    void noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative();
    void noteFileStatSupport_incrementsOpenCountAndPersistsLastOpenedAt();
    void unusedNoteSensors_filterNoteIdsByLastOpenedWindow();

private:
    static QString createLocalNoteForRegression(
        const QString& parentDirectoryPath,
        const QString& noteId,
        const QString& bodySourceText,
        QString* errorMessage = nullptr);
    static QJSValue evaluateQmlJsLibrary(
        QJSEngine* engine,
        const QString& relativeSourcePath);
    static QJSValue jsArrayEntry(const QJSValue& arrayValue, int index);
    static QString readUtf8SourceFile(const QString& relativeSourcePath);
    static QCoreApplication* ensureCoreApplication();
};

QString WhatSonCppRegressionTests::createLocalNoteForRegression(
    const QString& parentDirectoryPath,
    const QString& noteId,
    const QString& bodySourceText,
    QString* errorMessage)
{
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    const QString normalizedParentDirectoryPath = QDir::cleanPath(parentDirectoryPath.trimmed());
    const QString normalizedNoteId = noteId.trimmed();
    const QString noteDirectoryPath =
        QDir(normalizedParentDirectoryPath).filePath(QStringLiteral("%1.wsnote").arg(normalizedNoteId));

    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(normalizedNoteId);
    headerStore.setCreatedAt(QStringLiteral("2026-04-18-00-00-00"));
    headerStore.setAuthor(QStringLiteral("WhatSonCppRegressionTests"));
    headerStore.setLastModifiedAt(QStringLiteral("2026-04-18-00-00-00"));
    headerStore.setModifiedBy(QStringLiteral("WhatSonCppRegressionTests"));

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteFileStore::CreateRequest request;
    request.noteId = normalizedNoteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.headerStore = headerStore;
    request.bodyPlainText = bodySourceText;

    if (!fileStore.createNote(std::move(request), nullptr, errorMessage))
    {
        return {};
    }
    return noteDirectoryPath;
}

QJSValue WhatSonCppRegressionTests::evaluateQmlJsLibrary(
    QJSEngine* engine,
    const QString& relativeSourcePath)
{
    if (engine == nullptr)
    {
        return {};
    }

    QFile scriptFile(relativeSourcePath);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }

    QString scriptSource = QString::fromUtf8(scriptFile.readAll());
    scriptSource.remove(
        QRegularExpression(QStringLiteral(R"(^\s*\.pragma\s+library\s*\n?)")));

    const QJSValue evaluationResult = engine->evaluate(scriptSource, relativeSourcePath);
    if (evaluationResult.isError())
    {
        return evaluationResult;
    }
    return engine->globalObject();
}

QJSValue WhatSonCppRegressionTests::jsArrayEntry(const QJSValue& arrayValue, const int index)
{
    return arrayValue.property(QString::number(index));
}

QString WhatSonCppRegressionTests::readUtf8SourceFile(const QString& relativeSourcePath)
{
    QFile sourceFile(relativeSourcePath);
    if (!sourceFile.exists())
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        sourceFile.setFileName(repositoryRoot.filePath(relativeSourcePath));
    }
    if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }
    return QString::fromUtf8(sourceFile.readAll());
}

QCoreApplication* WhatSonCppRegressionTests::ensureCoreApplication()
{
    if (QCoreApplication::instance() != nullptr)
    {
        return QCoreApplication::instance();
    }

    static int argc = 1;
    static char applicationName[] = "whatson_cpp_regression_tests";
    static char* argv[] = {applicationName, nullptr};
    static std::unique_ptr<QCoreApplication> application;
    application = std::make_unique<QCoreApplication>(argc, argv);
    return application.get();
}

void WhatSonCppRegressionTests::selectedHubStore_persistsNormalizedSelectionsWithinSandboxedSettings()
{
    QTemporaryDir settingsDir;
    QTemporaryDir hubRootDir;
    QVERIFY(settingsDir.isValid());
    QVERIFY(hubRootDir.isValid());

    const ScopedQSettingsSandbox settingsSandbox(settingsDir.path());
    SelectedHubStore store;

    const QString selectedHubPath = QDir(hubRootDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    QVERIFY(QDir().mkpath(selectedHubPath));

    const QByteArray bookmark("bookmark-token");
    store.setSelectedHubSelection(selectedHubPath, bookmark);

    QCOMPARE(store.selectedHubPath(), WhatSon::HubPath::normalizeAbsolutePath(selectedHubPath));
    QCOMPARE(store.selectedHubAccessBookmark(), bookmark);

    store.clearSelectedHubPath();
    QCOMPARE(store.selectedHubPath(), QString());
    QCOMPARE(store.selectedHubAccessBookmark(), QByteArray());

    store.setSelectedHubPath(QDir(hubRootDir.path()).filePath(QStringLiteral("NotAHub.txt")));
    QCOMPARE(store.selectedHubPath(), QString());

    const QString fallbackHubPath = QDir(hubRootDir.path()).filePath(QStringLiteral("BlueprintFallback.wshub"));
    QCOMPARE(
        store.startupHubPath(fallbackHubPath),
        WhatSon::HubPath::normalizeAbsolutePath(fallbackHubPath));
    QCOMPARE(store.startupHubPath(QStringLiteral("BlueprintFallback.txt")), QString());
}

void WhatSonCppRegressionTests::sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals()
{
    SidebarSelectionStore store;
    QSignalSpy selectionChangedSpy(&store, &ISidebarSelectionStore::selectedHierarchyIndexChanged);

    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);

    store.setSelectedHierarchyIndex(999);
    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(selectionChangedSpy.count(), 0);

    store.setSelectedHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        store.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(selectionChangedSpy.count(), 1);

    store.setSelectedHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(selectionChangedSpy.count(), 1);

    store.setSelectedHierarchyIndex(-7);
    QCOMPARE(store.selectedHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(selectionChangedSpy.count(), 2);
}

void WhatSonCppRegressionTests::hierarchyViewModelProvider_normalizesMappingsAndAvoidsDuplicateSignals()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel tagsViewModel(QStringLiteral("tags"));

    libraryViewModel.setNodes(QVariantList{QStringLiteral("library-node")});
    tagsViewModel.setNodes(QVariantList{QStringLiteral("tags-node-a"), QStringLiteral("tags-node-b")});

    QSignalSpy mappingsChangedSpy(&provider, &IHierarchyViewModelProvider::mappingsChanged);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
        {999, &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
    QCOMPARE(
        provider.hierarchyViewModel(WhatSon::Sidebar::kHierarchyDefaultIndex),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        provider.hierarchyViewModel(-42),
        static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        provider.hierarchyViewModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<IHierarchyViewModel*>(&tagsViewModel));
    QCOMPARE(
        provider.noteListModel(-42),
        static_cast<QObject*>(libraryViewModel.hierarchyNoteListModel()));
    QCOMPARE(
        provider.noteListModel(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags)),
        static_cast<QObject*>(tagsViewModel.hierarchyNoteListModel()));

    const QVector<HierarchyViewModelProvider::Mapping> exportedMappings = provider.mappings();
    QCOMPARE(exportedMappings.size(), 2);
    QCOMPARE(exportedMappings.at(0).hierarchyIndex, WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(exportedMappings.at(0).viewModel, static_cast<IHierarchyViewModel*>(&libraryViewModel));
    QCOMPARE(
        exportedMappings.at(1).hierarchyIndex,
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(exportedMappings.at(1).viewModel, static_cast<IHierarchyViewModel*>(&tagsViewModel));

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
        {999, &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
    });

    QCOMPARE(mappingsChangedSpy.count(), 1);
}

void WhatSonCppRegressionTests::sidebarHierarchyViewModel_preservesFallbackAcrossStoreAttachDetach()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel tagsViewModel(QStringLiteral("tags"));
    FakeSidebarSelectionStore selectionStore;
    SidebarHierarchyViewModel sidebarViewModel;

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsViewModel},
    });

    sidebarViewModel.setViewModelProvider(&provider);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.activeNoteListModel(), libraryViewModel.hierarchyNoteListModel());

    QSignalSpy activeBindingsSpy(&sidebarViewModel, &IActiveHierarchyContextSource::activeBindingsChanged);
    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));

    QCOMPARE(
        sidebarViewModel.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&tagsViewModel));
    QVERIFY(activeBindingsSpy.count() >= 1);

    sidebarViewModel.setSelectionStore(&selectionStore);

    QCOMPARE(
        selectionStore.selectedHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(
        sidebarViewModel.activeHierarchyIndex(),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags));
    QCOMPARE(sidebarViewModel.activeNoteListModel(), tagsViewModel.hierarchyNoteListModel());

    selectionStore.setSelectedHierarchyIndex(999);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.hierarchyViewModelForIndex(-1), static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(sidebarViewModel.noteListModelForIndex(-1), libraryViewModel.hierarchyNoteListModel());

    sidebarViewModel.setSelectionStore(nullptr);

    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), WhatSon::Sidebar::kHierarchyDefaultIndex);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), static_cast<QObject*>(&libraryViewModel));
}

void WhatSonCppRegressionTests::sidebarHierarchyViewModel_reactsToProviderMappingChanges()
{
    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel firstLibraryViewModel(QStringLiteral("library-a"));
    FakeHierarchyViewModel replacementLibraryViewModel(QStringLiteral("library-b"));
    SidebarHierarchyViewModel sidebarViewModel;

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &firstLibraryViewModel},
    });
    sidebarViewModel.setViewModelProvider(&provider);

    QSignalSpy hierarchyViewModelChangedSpy(
        &sidebarViewModel,
        &IActiveHierarchyContextSource::activeHierarchyViewModelChanged);
    QSignalSpy noteListModelChangedSpy(
        &sidebarViewModel,
        &IActiveHierarchyContextSource::activeNoteListModelChanged);
    QSignalSpy activeBindingsSpy(&sidebarViewModel, &IActiveHierarchyContextSource::activeBindingsChanged);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &replacementLibraryViewModel},
    });

    QCOMPARE(
        sidebarViewModel.activeHierarchyViewModel(),
        static_cast<QObject*>(&replacementLibraryViewModel));
    QCOMPARE(
        sidebarViewModel.activeNoteListModel(),
        replacementLibraryViewModel.hierarchyNoteListModel());
    QVERIFY(hierarchyViewModelChangedSpy.count() >= 1);
    QVERIFY(noteListModelChangedSpy.count() >= 1);
    QVERIFY(activeBindingsSpy.count() >= 1);
}

void WhatSonCppRegressionTests::sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings()
{
    ensureCoreApplication();

    HierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    FakeSelectionNoteListModel libraryNoteListModel;
    FakeSelectionNoteListModel resourcesNoteListModel;
    SidebarHierarchyViewModel sidebarViewModel;
    ContentsEditorSelectionBridge selectionBridge;

    libraryNoteListModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryNoteListModel.setItemCount(5);
    libraryNoteListModel.setNoteBacked(true);

    resourcesNoteListModel.setCurrentNoteId(QStringLiteral("resource-entry"));
    resourcesNoteListModel.setItemCount(13);
    resourcesNoteListModel.setNoteBacked(false);

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    provider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryViewModel},
        {static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesViewModel},
    });
    sidebarViewModel.setViewModelProvider(&provider);

    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources));

    QObject* activeResourcesViewModel = sidebarViewModel.activeHierarchyViewModel();
    QObject* activeResourcesNoteListModel = sidebarViewModel.activeNoteListModel();

    QCOMPARE(activeResourcesViewModel, static_cast<QObject*>(&resourcesViewModel));
    QCOMPARE(activeResourcesNoteListModel, static_cast<QObject*>(&resourcesNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentViewModel(activeResourcesViewModel);
    selectionBridge.setNoteListModel(activeResourcesNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentViewModel(), activeResourcesViewModel);
    QCOMPARE(selectionBridge.noteListModel(), activeResourcesNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 13);
    QVERIFY(!selectionBridge.noteSelectionContractAvailable());

    sidebarViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));

    QObject* activeLibraryViewModel = sidebarViewModel.activeHierarchyViewModel();
    QObject* activeLibraryNoteListModel = sidebarViewModel.activeNoteListModel();

    QCOMPARE(activeLibraryViewModel, static_cast<QObject*>(&libraryViewModel));
    QCOMPARE(activeLibraryNoteListModel, static_cast<QObject*>(&libraryNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);

    selectionBridge.setContentViewModel(activeLibraryViewModel);
    selectionBridge.setNoteListModel(activeLibraryNoteListModel);
    QCoreApplication::processEvents();

    QCOMPARE(selectionBridge.contentViewModel(), activeLibraryViewModel);
    QCOMPARE(selectionBridge.noteListModel(), activeLibraryNoteListModel);
    QVERIFY(selectionBridge.noteCountContractAvailable());
    QCOMPARE(selectionBridge.visibleNoteCount(), 5);
    QVERIFY(selectionBridge.noteSelectionContractAvailable());
    QCOMPARE(selectionBridge.selectedNoteId(), QStringLiteral("library-note"));
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(activeLibraryNoteListModel), QQmlEngine::CppOwnership);
}

void WhatSonCppRegressionTests::noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately()
{
    ensureCoreApplication();

    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    FakeSelectionNoteListModel libraryNoteListModel;
    FakeSelectionNoteListModel resourcesNoteListModel;
    NoteListModelContractBridge bridge;

    libraryNoteListModel.setCurrentIndex(2);
    libraryNoteListModel.setCurrentNoteId(QStringLiteral("library-note"));
    libraryNoteListModel.setSearchText(QStringLiteral("library-query"));

    resourcesNoteListModel.setCurrentIndex(7);
    resourcesNoteListModel.setCurrentNoteId(QStringLiteral("resource-note"));
    resourcesNoteListModel.setSearchText(QStringLiteral("resource-query"));

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    QSignalSpy noteListModelChangedSpy(&bridge, &NoteListModelContractBridge::noteListModelChanged);

    bridge.setHierarchyViewModel(&resourcesViewModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&resourcesNoteListModel));
    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());
    QCOMPARE(bridge.currentIndex(), 7);
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("resource-note"));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);
    QVERIFY(bridge.applySearchText(QStringLiteral("resources-filter")));
    QCOMPARE(resourcesNoteListModel.searchText(), QStringLiteral("resources-filter"));
    QVERIFY(bridge.pushCurrentIndex(3));
    QCOMPARE(resourcesNoteListModel.currentIndex(), 3);

    bridge.setHierarchyViewModel(&libraryViewModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&libraryNoteListModel));
    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());
    QCOMPARE(bridge.currentIndex(), 2);
    QCOMPARE(bridge.currentNoteId(), QStringLiteral("library-note"));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryViewModel), QQmlEngine::CppOwnership);
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);
    QVERIFY(bridge.applySearchText(QStringLiteral("library-filter")));
    QCOMPARE(libraryNoteListModel.searchText(), QStringLiteral("library-filter"));
    QVERIFY(bridge.pushCurrentIndex(1));
    QCOMPARE(libraryNoteListModel.currentIndex(), 1);

    QCOMPARE(noteListModelChangedSpy.count(), 2);
}

void WhatSonCppRegressionTests::noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches()
{
    ensureCoreApplication();

    FakeHierarchyViewModel libraryViewModel(QStringLiteral("library"));
    FakeHierarchyViewModel resourcesViewModel(QStringLiteral("resources"));
    LibraryNoteListModel libraryNoteListModel;
    ResourcesListModel resourcesNoteListModel;
    NoteListModelContractBridge bridge;

    LibraryNoteListItem libraryItem;
    libraryItem.id = QStringLiteral("library-note");
    libraryItem.primaryText = QStringLiteral("Library note");
    libraryItem.displayDate = QStringLiteral("2026-04-18");
    libraryItem.folders = {QStringLiteral("Marketing")};
    libraryItem.tags = {QStringLiteral("launch")};
    libraryNoteListModel.setItems({libraryItem});

    ResourcesListItem resourceItem;
    resourceItem.id = QStringLiteral("resource-entry");
    resourceItem.primaryText = QStringLiteral("Resource entry");
    resourceItem.displayDate = QStringLiteral("image");
    resourceItem.folders = {QStringLiteral("Image")};
    resourceItem.tags = {QStringLiteral(".png")};
    resourcesNoteListModel.setItems({resourceItem});

    libraryViewModel.setNoteListModelObject(&libraryNoteListModel);
    resourcesViewModel.setNoteListModelObject(&resourcesNoteListModel);

    QQmlEngine::setObjectOwnership(&libraryViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesViewModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&libraryNoteListModel, QQmlEngine::JavaScriptOwnership);
    QQmlEngine::setObjectOwnership(&resourcesNoteListModel, QQmlEngine::JavaScriptOwnership);

    bridge.setHierarchyViewModel(&resourcesViewModel);
    bridge.setNoteListModel(&resourcesNoteListModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&resourcesNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&resourcesNoteListModel), QQmlEngine::CppOwnership);

    QVariantList resourceRows = bridge.readAllRows();
    QCOMPARE(resourceRows.size(), 1);
    const QVariantMap resourceRow = resourceRows.constFirst().toMap();
    QCOMPARE(resourceRow.value(QStringLiteral("noteId")).toString(), QStringLiteral("resource-entry"));
    QCOMPARE(resourceRow.value(QStringLiteral("folders")).toStringList(), QStringList{QStringLiteral("Image")});
    QCOMPARE(resourceRow.value(QStringLiteral("tags")).toStringList(), QStringList{QStringLiteral(".png")});

    bridge.setHierarchyViewModel(&libraryViewModel);
    bridge.setNoteListModel(&libraryNoteListModel);

    QCOMPARE(bridge.noteListModel(), static_cast<QObject*>(&libraryNoteListModel));
    QCOMPARE(QQmlEngine::objectOwnership(&libraryNoteListModel), QQmlEngine::CppOwnership);

    QVariantList libraryRows = bridge.readAllRows();
    QCOMPARE(libraryRows.size(), 1);
    const QVariantMap libraryRow = libraryRows.constFirst().toMap();
    QCOMPARE(libraryRow.value(QStringLiteral("noteId")).toString(), QStringLiteral("library-note"));
    QCOMPARE(libraryRow.value(QStringLiteral("folders")).toStringList(), QStringList{QStringLiteral("Marketing")});
    QCOMPARE(libraryRow.value(QStringLiteral("tags")).toStringList(), QStringList{QStringLiteral("launch")});
}

void WhatSonCppRegressionTests::navigationModeViewModel_cyclesActiveSections()
{
    NavigationModeViewModel viewModel;
    auto* viewSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.viewModeViewModel());
    auto* editSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.editModeViewModel());
    auto* controlSection =
        qobject_cast<NavigationModeSectionViewModel*>(viewModel.controlModeViewModel());

    QVERIFY(viewSection != nullptr);
    QVERIFY(editSection != nullptr);
    QVERIFY(controlSection != nullptr);

    QSignalSpy activeModeChangedSpy(&viewModel, &NavigationModeViewModel::activeModeChanged);

    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("View"));
    QCOMPARE(
        viewModel.activeModeViewModel(),
        static_cast<QObject*>(viewSection));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QVERIFY(viewModel.modeViewModelForState(999) == nullptr);

    viewModel.requestNextMode();
    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Edit));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Edit"));
    QCOMPARE(
        viewModel.activeModeViewModel(),
        static_cast<QObject*>(editSection));
    QVERIFY(!viewSection->active());
    QVERIFY(editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 1);

    viewModel.requestModeChange(WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::Control));
    QCOMPARE(viewModel.activeModeName(), QStringLiteral("Control"));
    QVERIFY(!viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 2);

    viewModel.setActiveMode(999);
    QCOMPARE(activeModeChangedSpy.count(), 2);

    viewModel.requestNextMode();
    QCOMPARE(
        viewModel.activeMode(),
        WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::Mode::View));
    QVERIFY(viewSection->active());
    QVERIFY(!editSection->active());
    QVERIFY(!controlSection->active());
    QCOMPARE(activeModeChangedSpy.count(), 3);
}

void WhatSonCppRegressionTests::editorViewModeViewModel_cyclesActiveSections()
{
    EditorViewModeViewModel viewModel;
    auto* plainSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.plainViewModeViewModel());
    auto* pageSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.pageViewModeViewModel());
    auto* printSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.printViewModeViewModel());
    auto* webSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.webViewModeViewModel());
    auto* presentationSection =
        qobject_cast<EditorViewSectionViewModel*>(viewModel.presentationViewModeViewModel());

    QVERIFY(plainSection != nullptr);
    QVERIFY(pageSection != nullptr);
    QVERIFY(printSection != nullptr);
    QVERIFY(webSection != nullptr);
    QVERIFY(presentationSection != nullptr);

    QSignalSpy activeViewModeChangedSpy(&viewModel, &EditorViewModeViewModel::activeViewModeChanged);

    QCOMPARE(
        viewModel.activeViewMode(),
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Plain));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QCOMPARE(
        viewModel.activeViewModeViewModel(),
        static_cast<QObject*>(plainSection));
    QVERIFY(plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(!presentationSection->active());
    QVERIFY(viewModel.viewModeViewModelForState(999) == nullptr);

    viewModel.requestNextViewMode();
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Page"));
    QVERIFY(!plainSection->active());
    QVERIFY(pageSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 1);

    viewModel.requestViewModeChange(
        WhatSon::NavigationBar::editorViewValue(WhatSon::NavigationBar::EditorView::Presentation));
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Presentation"));
    QVERIFY(!plainSection->active());
    QVERIFY(!pageSection->active());
    QVERIFY(!printSection->active());
    QVERIFY(!webSection->active());
    QVERIFY(presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    viewModel.setActiveViewMode(-1);
    QCOMPARE(activeViewModeChangedSpy.count(), 2);

    viewModel.requestNextViewMode();
    QCOMPARE(viewModel.activeViewModeName(), QStringLiteral("Plain"));
    QVERIFY(plainSection->active());
    QVERIFY(!presentationSection->active());
    QCOMPARE(activeViewModeChangedSpy.count(), 3);
}

void WhatSonCppRegressionTests::onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle()
{
    OnboardingRouteBootstrapController controller;
    FakeOnboardingHubController hubController;
    controller.setHubController(&hubController);

    QSignalSpy routeSyncSpy(
        &controller,
        &OnboardingRouteBootstrapController::routeSyncRequested);

    controller.configure(true, false);

    QVERIFY(controller.embeddedOnboardingEnabled());
    QVERIFY(controller.embeddedOnboardingVisible());
    QVERIFY(!controller.routeCommitPending());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/onboarding"));

    controller.handleHubLoaded();
    QCOMPARE(hubController.beginCount, 1);
    QVERIFY(controller.routeCommitPending());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 1);
    QCOMPARE(routeSyncSpy.at(0).at(0).toString(), QStringLiteral("/"));
    QCOMPARE(routeSyncSpy.at(0).at(1).toBool(), true);
    QCOMPARE(routeSyncSpy.at(0).at(2).toString(), QStringLiteral("embeddedHubLoaded"));

    controller.handleOperationFailed(QStringLiteral("load failed"));
    QCOMPARE(hubController.failCount, 1);
    QCOMPARE(hubController.lastFailureMessage, QStringLiteral("load failed"));
    QVERIFY(!controller.routeCommitPending());
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 2);
    QCOMPARE(routeSyncSpy.at(1).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(1).at(1).toBool(), true);
    QCOMPARE(routeSyncSpy.at(1).at(2).toString(), QStringLiteral("embeddedOperationFailed"));

    controller.handleHubLoaded();
    controller.handlePageStackNavigated(QStringLiteral("/"));
    QCOMPARE(hubController.beginCount, 2);
    QCOMPARE(hubController.completeCount, 1);
    QVERIFY(!controller.routeCommitPending());

    controller.handleHubLoaded();
    controller.handlePageStackNavigationFailed(QStringLiteral("/workspace"));
    QCOMPARE(hubController.failCount, 2);
    QCOMPARE(
        hubController.lastFailureMessage,
        QStringLiteral("Failed to open the WhatSon workspace after onboarding."));
    QVERIFY(!controller.routeCommitPending());
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 5);
    QCOMPARE(routeSyncSpy.at(4).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(4).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(4).at(2).toString(), QStringLiteral("navigationFailed"));

    controller.dismissEmbeddedOnboarding();
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 6);
    QCOMPARE(routeSyncSpy.at(5).at(0).toString(), QStringLiteral("/"));
    QCOMPARE(routeSyncSpy.at(5).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(5).at(2).toString(), QStringLiteral("dismissEmbeddedOnboarding"));

    controller.reopenEmbeddedOnboarding();
    QVERIFY(controller.embeddedOnboardingVisible());
    QCOMPARE(routeSyncSpy.count(), 7);
    QCOMPARE(routeSyncSpy.at(6).at(0).toString(), QStringLiteral("/onboarding"));
    QCOMPARE(routeSyncSpy.at(6).at(1).toBool(), false);
    QCOMPARE(routeSyncSpy.at(6).at(2).toString(), QStringLiteral("reopenEmbeddedOnboarding"));

    controller.configure(false, false);
    QVERIFY(!controller.embeddedOnboardingEnabled());
    QVERIFY(!controller.embeddedOnboardingVisible());
    QCOMPARE(controller.startupRoutePath(), QStringLiteral("/"));
}

void WhatSonCppRegressionTests::embeddedOnboardingRoutePages_avoidStackViewAnchorConflicts()
{
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(mainQmlSource.contains(QStringLiteral("id: onboardingPageComponent")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("id: workspacePageComponent")));
    QVERIFY(mainQmlSource.contains(
        QStringLiteral("id: onboardingPageComponent\n\n        Item {\n            clip: true")));
    QVERIFY(mainQmlSource.contains(
        QStringLiteral("id: workspacePageComponent\n\n        Item {\n            clip: true")));
    QVERIFY(!mainQmlSource.contains(
        QStringLiteral("id: onboardingPageComponent\n\n        Item {\n            anchors.fill: parent")));
    QVERIFY(!mainQmlSource.contains(
        QStringLiteral("id: workspacePageComponent\n\n        Item {\n            anchors.fill: parent")));
}

void WhatSonCppRegressionTests::mainQml_embeddedStartup_dropsWatchdogRecoveryScaffold()
{
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(mainQmlSource.contains(QStringLiteral("applicationWindow.applyRequestedRoute(applicationWindow.startupRoutePath, \"completed\");")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("applicationWindow.applyRequestedRoute(normalizedTargetPath, normalizedReason);")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("expectedEmbeddedRoutePath")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("embeddedRouteRecoveryNeeded")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("embeddedRouteStartupStatusText")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("startupRouteRecoveryAttempts")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("startupRouteRecoveryReason")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("function recoverEmbeddedRouteHost(")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("function syncEmbeddedRouteWatchdog(")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("id: startupRouteWatchdogTimer")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("id: embeddedRouteStartupFallback")));
    QVERIFY(!mainQmlSource.contains(QStringLiteral("forceReload")));
}

void WhatSonCppRegressionTests::mainQml_iosInlineOnboarding_pinsPresentationToWorkspaceRoute()
{
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(mainQmlSource.contains(QStringLiteral("readonly property bool useIosInlineOnboardingSequence: applicationWindow.platform === \"ios\"")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("readonly property bool useRoutedEmbeddedOnboardingRoute: !applicationWindow.useIosInlineOnboardingSequence")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("readonly property string startupRoutePath: applicationWindow.useIosInlineOnboardingSequence")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("pageRoutes: applicationWindow.useIosInlineOnboardingSequence")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("? iosInlineOnboardingSequenceComponent")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("applicationWindow.onboardingRouteBootstrapController.handlePageStackNavigated(")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("applicationWindow.workspaceRoutePath);")));
}

void WhatSonCppRegressionTests::mainCpp_iosStartup_suppressesAutomaticOnboarding()
{
    const QString mainCppSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(!mainCppSource.isEmpty());
    QVERIFY(mainCppSource.contains(QStringLiteral("const bool enableEmbeddedOnboardingPresentation = true;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("const bool suppressAutomaticStartupOnboardingOnIos = !startupHubSelection.mounted;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("onboardingRouteBootstrapController.dismissEmbeddedOnboarding();")));
}

void WhatSonCppRegressionTests::iosInlineOnboardingSequence_reusesSharedOnboardingSurface()
{
    const QString sequenceSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/IosInlineOnboardingSequence.qml"));

    QVERIFY(!sequenceSource.isEmpty());
    QVERIFY(sequenceSource.contains(QStringLiteral("WindowView.OnboardingContent {")));
    QVERIFY(sequenceSource.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(sequenceSource.contains(QStringLiteral("autoCompleteOnHubLoaded: false")));
    QVERIFY(sequenceSource.contains(QStringLiteral("color: root.canvasColor")));
}

void WhatSonCppRegressionTests::onboardingContent_mobileLayout_avoidsFullscreenAntialiasedWindowFrame()
{
    const QString onboardingContentSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"));

    QVERIFY(!onboardingContentSource.isEmpty());
    QVERIFY(onboardingContentSource.contains(QStringLiteral("readonly property bool useRoundedWindowFrame: !root.useMobileLayout")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("antialiasing: root.useRoundedWindowFrame")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("clip: root.useRoundedWindowFrame")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("radius: root.useRoundedWindowFrame ? root.panelCornerRadius : 0")));
    QVERIFY(!onboardingContentSource.contains(QStringLiteral("radius: root.useMobileLayout ? root.mobileSurfaceRadius : root.panelCornerRadius")));
}

void WhatSonCppRegressionTests::onboardingContent_saveDialog_doesNotPreselectMissingHubFile()
{
    const QString onboardingContentSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/window/OnboardingContent.qml"));

    QVERIFY(!onboardingContentSource.isEmpty());
    QVERIFY(onboardingContentSource.contains(QStringLiteral("id: createHubDialogComponent")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("FileDialog {")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("root.createHubDialogInstance = createHubDialogComponent.createObject(root);")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("root.openCreateHubDialog();")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("id: selectHubFileDialogComponent")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("root.selectHubFileDialogInstance = selectHubFileDialogComponent.createObject(root);")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("currentFile: root.suggestedCreateHubFileUrl")));
    QVERIFY(onboardingContentSource.contains(QStringLiteral("currentFolder: root.currentFolderUrl")));
    QVERIFY(!onboardingContentSource.contains(QStringLiteral("id: createHubDialog\n")));
    QVERIFY(!onboardingContentSource.contains(QStringLiteral("id: selectHubFileDialog\n")));
    QVERIFY(!onboardingContentSource.contains(QStringLiteral("selectedFile: root.suggestedCreateHubFileUrl")));
}

void WhatSonCppRegressionTests::mainQml_exposesZeroWindowPaddingForLvrsApplicationWindow()
{
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));

    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(mainQmlSource.contains(QStringLiteral("property int topPadding: 0")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("property int rightPadding: 0")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("property int bottomPadding: 0")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("property int leftPadding: 0")));
}

void WhatSonCppRegressionTests::clipboardImportFileNamePolicy_generatesRandom32CharacterAlphaNumericPngNames()
{
    const QString firstFileName = WhatSon::Resources::generateClipboardImportAssetFileName();
    const QString secondFileName = WhatSon::Resources::generateClipboardImportAssetFileName();

    const QRegularExpression expectedPattern(QStringLiteral("^[A-Za-z0-9]{32}\\.png$"));
    QVERIFY(expectedPattern.match(firstFileName).hasMatch());
    QVERIFY(expectedPattern.match(secondFileName).hasMatch());
    QVERIFY(firstFileName != secondFileName);
}

void WhatSonCppRegressionTests::unixTimeAnalyzer_reportsStableEpochFields()
{
    const QVariantMap epochAnalysis = WhatSon::Runtime::Scheduler::analyzeUnixSeconds(0);

    QCOMPARE(epochAnalysis.value(QStringLiteral("unixSeconds")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("unixMilliseconds")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("unixMinuteKey")).toLongLong(), 0);
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcDate")).toString(), QStringLiteral("1970-01-01"));
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcTime")).toString(), QStringLiteral("00:00:00"));
    QCOMPARE(epochAnalysis.value(QStringLiteral("utcIso")).toString(), QStringLiteral("1970-01-01T00:00:00Z"));
    QVERIFY(!epochAnalysis.value(QStringLiteral("localIso")).toString().trimmed().isEmpty());

    const int cronDayOfWeek = epochAnalysis.value(QStringLiteral("localDayOfWeekCron")).toInt();
    QVERIFY(cronDayOfWeek >= 0);
    QVERIFY(cronDayOfWeek <= 6);
    QVERIFY(WhatSon::Runtime::Scheduler::unixNowSeconds() > 0);
}

void WhatSonCppRegressionTests::cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication()
{
    WhatSonCronExpression matcher;
    QString parseError;
    QVERIFY(matcher.parse(QStringLiteral("*/15 9-10 * * 1,3,5"), &parseError));
    QVERIFY(parseError.isEmpty());
    QCOMPARE(matcher.expression(), QStringLiteral("*/15 9-10 * * 1,3,5"));
    QVERIFY(matcher.isValid());
    QVERIFY(matcher.matches(QDateTime(QDate(2024, 4, 1), QTime(9, 30))));
    QVERIFY(!matcher.matches(QDateTime(QDate(2024, 4, 1), QTime(9, 31))));
    QVERIFY(!matcher.matches(QDateTime(QDate(2024, 4, 2), QTime(9, 30))));

    WhatSonCronExpression sundayAliasMatcher;
    QVERIFY(sundayAliasMatcher.parse(QStringLiteral("0 12 * * 7")));
    QVERIFY(sundayAliasMatcher.matches(QDateTime(QDate(2024, 3, 31), QTime(12, 0))));

    WhatSonCronExpression invalidMatcher;
    QString invalidParseError;
    QVERIFY(!invalidMatcher.parse(QStringLiteral("61 * * * *"), &invalidParseError));
    QVERIFY(!invalidParseError.trimmed().isEmpty());

    WhatSonAsyncScheduler scheduler;
    QSignalSpy schedulerWarningSpy(&scheduler, &WhatSonAsyncScheduler::schedulerWarning);
    QSignalSpy schedulesChangedSpy(&scheduler, &WhatSonAsyncScheduler::schedulesChanged);
    QSignalSpy scheduleTriggeredSpy(&scheduler, &WhatSonAsyncScheduler::scheduleTriggered);
    QSignalSpy runningChangedSpy(&scheduler, &WhatSonAsyncScheduler::runningChanged);
    QSignalSpy hookRequestCountChangedSpy(&scheduler, &WhatSonAsyncScheduler::hookRequestCountChanged);

    QVERIFY(!scheduler.hookCronEvent(QString(), QStringLiteral("* * * * *")));
    QCOMPARE(schedulerWarningSpy.count(), 1);

    QVERIFY(scheduler.hookCronEvent(
        QStringLiteral("cron"),
        QStringLiteral("* * * * *"),
        QVariantMap{{QStringLiteral("scope"), QStringLiteral("minute")}}));
    QCOMPARE(scheduler.scheduleCount(), 1);
    QVERIFY(schedulesChangedSpy.count() >= 1);

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(120), 1);
    QCOMPARE(scheduleTriggeredSpy.count(), 1);
    QCOMPARE(scheduleTriggeredSpy.at(0).at(0).toString(), QStringLiteral("cron"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(1).toString(), QStringLiteral("cron"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(2).toString(), QStringLiteral("* * * * *"));
    QCOMPARE(scheduleTriggeredSpy.at(0).at(3).toLongLong(), 120);
    QCOMPARE(
        scheduleTriggeredSpy.at(0).at(5).toMap().value(QStringLiteral("scope")).toString(),
        QStringLiteral("minute"));

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(121), 0);
    QCOMPARE(scheduleTriggeredSpy.count(), 1);

    QVERIFY(scheduler.unhookEvent(QStringLiteral("cron")));
    QCOMPARE(scheduler.scheduleCount(), 0);
    QVERIFY(!scheduler.unhookEvent(QStringLiteral("missing")));

    QVERIFY(scheduler.hookIntervalEvent(
        QStringLiteral("interval"),
        5,
        QVariantMap{{QStringLiteral("scope"), QStringLiteral("interval")}}));
    QCOMPARE(scheduler.scheduleCount(), 1);

    qint64 nextTriggerUnixSeconds = -1;
    const QVariantList schedules = scheduler.schedules();
    for (const QVariant& scheduleValue : schedules)
    {
        const QVariantMap schedule = scheduleValue.toMap();
        if (schedule.value(QStringLiteral("type")).toString() == QStringLiteral("interval"))
        {
            nextTriggerUnixSeconds = schedule.value(QStringLiteral("nextTriggerUnixSeconds")).toLongLong();
            break;
        }
    }
    QVERIFY(nextTriggerUnixSeconds > 0);

    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(nextTriggerUnixSeconds - 1), 0);
    QCOMPARE(scheduler.evaluateSchedulesAtUnixSeconds(nextTriggerUnixSeconds), 1);
    QCOMPARE(scheduleTriggeredSpy.count(), 2);
    QCOMPARE(scheduleTriggeredSpy.at(1).at(0).toString(), QStringLiteral("interval"));
    QCOMPARE(scheduleTriggeredSpy.at(1).at(1).toString(), QStringLiteral("interval"));
    QCOMPARE(scheduleTriggeredSpy.at(1).at(2).toString(), QString::number(5));
    QCOMPARE(
        scheduleTriggeredSpy.at(1).at(5).toMap().value(QStringLiteral("scope")).toString(),
        QStringLiteral("interval"));

    scheduler.requestViewModelHook();
    QCOMPARE(scheduler.hookRequestCount(), 1);
    QCOMPARE(hookRequestCountChangedSpy.count(), 1);

    const bool started = scheduler.start();
    if (started)
    {
        QVERIFY(!scheduler.start());
        QVERIFY(scheduler.running());
        scheduler.stop();
        scheduler.stop();
        QVERIFY(!scheduler.running());
        QCOMPARE(runningChangedSpy.count(), 2);
    }
    else
    {
        QVERIFY(!scheduler.running());
        QVERIFY(!schedulerWarningSpy.isEmpty());
        QVERIFY(
            schedulerWarningSpy.constLast().at(0).toString().contains(
                QStringLiteral("could not be activated")));
        QVERIFY(!scheduler.start());
        QVERIFY(!scheduler.running());
    }

    QVERIFY(scheduler.unhookEvent(QStringLiteral("interval")));
    scheduler.clearSchedules();
    QCOMPARE(scheduler.scheduleCount(), 0);
}

void WhatSonCppRegressionTests::hierarchyTreeItemSupport_clampsNegativeSelectionToFirstVisibleRow()
{
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, 0), -1);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, 4), 0);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(0, 4), 0);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(3, 4), 3);
    QCOMPARE(WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(99, 4), 3);
}

void WhatSonCppRegressionTests::progressHierarchySupport_defaultsFirstVisibleItemToFirstDraft()
{
    const QVector<ProgressHierarchyItem> items =
        WhatSon::Hierarchy::ProgressSupport::buildSupportedTypeItems({});

    QVERIFY(!items.isEmpty());
    QCOMPARE(items.constFirst().label, QStringLiteral("First draft"));
    QCOMPARE(
        WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(-1, items.size()),
        0);
}

void WhatSonCppRegressionTests::resourcePackageSupport_roundTripsAnnotationMetadataAndBitmap()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString assetFilePath = QDir(temporaryDirectory.path()).filePath(QStringLiteral("cover.png"));
    QImage sourceImage(QSize(13, 7), QImage::Format_ARGB32_Premultiplied);
    sourceImage.fill(qRgba(12, 34, 56, 255));
    QVERIFY(sourceImage.save(assetFilePath));

    const WhatSon::Resources::ResourcePackageMetadata metadata =
        WhatSon::Resources::buildMetadataForAssetFile(
            assetFilePath,
            QStringLiteral("cover"),
            QStringLiteral("Demo.wsresources/cover.wsresource"));
    QCOMPARE(metadata.assetPath, QStringLiteral("cover.png"));
    QCOMPARE(metadata.annotationPath, QStringLiteral("annotation.png"));

    const QString metadataXml = WhatSon::Resources::createResourcePackageMetadataXml(metadata);
    QVERIFY(metadataXml.contains(QStringLiteral("<annotation path=\"annotation.png\"/>")));

    WhatSon::Resources::ResourcePackageMetadata parsedMetadata;
    QString parseError;
    QVERIFY2(
        WhatSon::Resources::parseResourcePackageMetadataXml(metadataXml, &parsedMetadata, &parseError),
        qPrintable(parseError));
    QCOMPARE(parsedMetadata.annotationPath, QStringLiteral("annotation.png"));

    const QByteArray annotationBytes =
        WhatSon::Resources::createEmptyAnnotationBitmapPngBytes(assetFilePath);
    QVERIFY(!annotationBytes.isEmpty());

    QImage annotationImage;
    QVERIFY(annotationImage.loadFromData(annotationBytes, "PNG"));
    QCOMPARE(annotationImage.size(), sourceImage.size());
    QCOMPARE(qAlpha(annotationImage.pixel(0, 0)), 0);
    QCOMPARE(
        qAlpha(annotationImage.pixel(annotationImage.width() - 1, annotationImage.height() - 1)),
        0);

    const QString packageDirectoryPath =
        QDir(temporaryDirectory.path()).filePath(QStringLiteral("cover.wsresource"));
    QVERIFY(QDir().mkpath(packageDirectoryPath));
    QVERIFY(QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(QStringLiteral("cover.png"))));

    QString writeError;
    QVERIFY2(
        WhatSon::Resources::writeResourcePackageAnnotationBitmap(
            packageDirectoryPath,
            assetFilePath,
            &writeError),
        qPrintable(writeError));

    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(metadataFile.write(metadataXml.toUtf8()) >= 0);
    metadataFile.close();

    WhatSon::Resources::ResourcePackageMetadata loadedMetadata;
    QString loadError;
    QVERIFY2(
        WhatSon::Resources::loadResourcePackageMetadata(
            packageDirectoryPath,
            &loadedMetadata,
            &loadError),
        qPrintable(loadError));
    QCOMPARE(loadedMetadata.assetPath, QStringLiteral("cover.png"));
    QCOMPARE(loadedMetadata.annotationPath, QStringLiteral("annotation.png"));
    QVERIFY(QFileInfo(WhatSon::Resources::annotationFilePathForPackage(packageDirectoryPath)).isFile());
}

void WhatSonCppRegressionTests::unusedResourcesSensor_reportsHubPackagesMissingFromAllNoteEmbeddings()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString resourcesDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesDirectoryPath));

    auto createResourcePackage = [&](const QString& resourceId, const QRgb fillColor) -> QString
    {
        const QString assetFilePath =
            QDir(workspaceDir.path()).filePath(QStringLiteral("%1.png").arg(resourceId));
        QImage image(QSize(11, 7), QImage::Format_ARGB32_Premultiplied);
        image.fill(fillColor);
        if (!image.save(assetFilePath))
        {
            return {};
        }

        const QString packageDirectoryPath =
            QDir(resourcesDirectoryPath).filePath(QStringLiteral("%1.wsresource").arg(resourceId));
        if (!QDir().mkpath(packageDirectoryPath))
        {
            return {};
        }

        const QString resourcePath =
            WhatSon::Resources::resourcePathForPackageDirectory(packageDirectoryPath);
        const WhatSon::Resources::ResourcePackageMetadata metadata =
            WhatSon::Resources::buildMetadataForAssetFile(assetFilePath, resourceId, resourcePath);

        if (!QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(metadata.assetPath)))
        {
            return {};
        }

        QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
        if (!metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return {};
        }
        if (metadataFile.write(
                WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8())
            < 0)
        {
            return {};
        }
        metadataFile.close();

        QString annotationError;
        if (!WhatSon::Resources::writeResourcePackageAnnotationBitmap(
                packageDirectoryPath,
                assetFilePath,
                &annotationError))
        {
            return {};
        }
        return resourcePath;
    };

    const QString usedResourcePath = createResourcePackage(QStringLiteral("used-cover"), qRgba(20, 60, 90, 255));
    const QString unusedResourcePath = createResourcePackage(QStringLiteral("unused-cover"), qRgba(90, 60, 20, 255));
    const QString hiddenOnlyResourcePath = createResourcePackage(QStringLiteral("hidden-only"), qRgba(40, 90, 30, 255));
    QVERIFY(!usedResourcePath.isEmpty());
    QVERIFY(!unusedResourcePath.isEmpty());
    QVERIFY(!hiddenOnlyResourcePath.isEmpty());

    QString createError;
    const QString usedNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("used-note"),
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />\nvisible body")
            .arg(usedResourcePath),
        &createError);
    QVERIFY2(
        !usedNoteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create used note fixture: %1").arg(createError)));

    const QString hiddenNotesDirectoryPath = QDir(contentsDirectoryPath).filePath(QStringLiteral(".archive"));
    QVERIFY(QDir().mkpath(hiddenNotesDirectoryPath));

    createError.clear();
    const QString hiddenNoteDirectoryPath = createLocalNoteForRegression(
        hiddenNotesDirectoryPath,
        QStringLiteral("hidden-note"),
        QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />").arg(hiddenOnlyResourcePath),
        &createError);
    QVERIFY2(
        !hiddenNoteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create hidden note fixture: %1").arg(createError)));

    UnusedResourcesSensor sensor;
    QSignalSpy unusedResourcesChangedSpy(&sensor, &UnusedResourcesSensor::unusedResourcesChanged);
    QSignalSpy scanCompletedSpy(&sensor, &UnusedResourcesSensor::scanCompleted);

    sensor.setHubPath(hubPath);

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 2);
    const QStringList expectedUnusedResourcePaths{
        hiddenOnlyResourcePath,
        unusedResourcePath
    };
    QCOMPARE(
        sensor.unusedResourcePaths(),
        expectedUnusedResourcePaths);
    QCOMPARE(unusedResourcesChangedSpy.count(), 1);
    QCOMPARE(scanCompletedSpy.count(), 1);

    const QVariantList unusedEntries = sensor.unusedResources();
    QCOMPARE(unusedEntries.size(), 2);

    const QVariantMap firstEntry = unusedEntries.at(0).toMap();
    QCOMPARE(firstEntry.value(QStringLiteral("resourcePath")).toString(), hiddenOnlyResourcePath);
    QCOMPARE(firstEntry.value(QStringLiteral("annotationPath")).toString(), QStringLiteral("annotation.png"));
    QVERIFY(firstEntry.value(QStringLiteral("metadataValid")).toBool());
    QVERIFY(QFileInfo(firstEntry.value(QStringLiteral("assetAbsolutePath")).toString()).isFile());

    const QVariantList completedEntries = scanCompletedSpy.constFirst().constFirst().toList();
    QCOMPARE(completedEntries.size(), 2);
}

void WhatSonCppRegressionTests::unusedResourcesSensor_refreshesAfterRawBodyEmbedsAResource()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    const QString resourcesDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wsresources"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));
    QVERIFY(QDir().mkpath(resourcesDirectoryPath));

    const QString assetFilePath = QDir(workspaceDir.path()).filePath(QStringLiteral("loose-image.png"));
    QImage image(QSize(9, 9), QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(80, 20, 140, 255));
    QVERIFY(image.save(assetFilePath));

    const QString packageDirectoryPath =
        QDir(resourcesDirectoryPath).filePath(QStringLiteral("loose-image.wsresource"));
    QVERIFY(QDir().mkpath(packageDirectoryPath));

    const QString resourcePath =
        WhatSon::Resources::resourcePathForPackageDirectory(packageDirectoryPath);
    const WhatSon::Resources::ResourcePackageMetadata metadata =
        WhatSon::Resources::buildMetadataForAssetFile(
            assetFilePath,
            QStringLiteral("loose-image"),
            resourcePath);

    QVERIFY(QFile::copy(assetFilePath, QDir(packageDirectoryPath).filePath(metadata.assetPath)));
    QFile metadataFile(WhatSon::Resources::metadataFilePathForPackage(packageDirectoryPath));
    QVERIFY(metadataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(metadataFile.write(WhatSon::Resources::createResourcePackageMetadataXml(metadata).toUtf8()) >= 0);
    metadataFile.close();

    QString annotationError;
    QVERIFY2(
        WhatSon::Resources::writeResourcePackageAnnotationBitmap(
            packageDirectoryPath,
            assetFilePath,
            &annotationError),
        qPrintable(annotationError));

    QString createError;
    const QString noteId = QStringLiteral("dynamic-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        noteId,
        QStringLiteral("body-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    UnusedResourcesSensor sensor;
    QSignalSpy unusedResourcesChangedSpy(&sensor, &UnusedResourcesSensor::unusedResourcesChanged);
    QSignalSpy scanCompletedSpy(&sensor, &UnusedResourcesSensor::scanCompleted);

    sensor.setHubPath(hubPath);

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 1);
    QCOMPARE(sensor.unusedResourcePaths(), QStringList{resourcePath});

    const QString bodyFilePath = WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath);
    QFile bodyFile(bodyFilePath);
    QVERIFY(bodyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QVERIFY(
        bodyFile.write(
            WhatSon::NoteBodyPersistence::serializeBodyDocument(
                noteId,
                QStringLiteral("<resource type=\"image\" format=\".png\" path=\"%1\" />").arg(resourcePath))
                .toUtf8())
        >= 0);
    bodyFile.close();

    sensor.refresh();

    QCOMPARE(sensor.lastError(), QString());
    QCOMPARE(sensor.unusedResourceCount(), 0);
    QVERIFY(sensor.unusedResources().isEmpty());
    QCOMPARE(sensor.collectUnusedResourcePaths(), QStringList{});
    QCOMPARE(unusedResourcesChangedSpy.count(), 2);
    QCOMPARE(scanCompletedSpy.count(), 3);
}

void WhatSonCppRegressionTests::resourcesImportViewModel_wiresAnnotationBitmapGenerationIntoPackageCreation()
{
    const QString importViewModelSource = readUtf8SourceFile(
        QStringLiteral("src/app/file/import/ResourcesImportViewModel.cpp"));

    QVERIFY(!importViewModelSource.isEmpty());
    QVERIFY(importViewModelSource.count(QStringLiteral("writeResourcePackageAnnotationBitmap(")) >= 2);
    QVERIFY(importViewModelSource.contains(QStringLiteral("entry.insert(QStringLiteral(\"annotationPath\")")));
}

void WhatSonCppRegressionTests::resourceTagTextGenerator_and_noteFolderSemantics_normalizeDescriptorsAndXml()
{
    ContentsResourceTagTextGenerator generator;
    QSignalSpy generatedTagTextChangedSpy(
        &generator,
        &ContentsResourceTagTextGenerator::lastGeneratedTagTextChanged);
    QSignalSpy generatedDescriptorChangedSpy(
        &generator,
        &ContentsResourceTagTextGenerator::lastGeneratedDescriptorChanged);

    const QVariantMap importedResourceEntry{
        {QStringLiteral("resourcePath"), QStringLiteral("images/&lt;Cover&gt;.PNG")},
        {QStringLiteral("bucket"), QStringLiteral("Image")},
        {QStringLiteral("id"), QStringLiteral("res-1")},
    };

    const QVariantMap normalizedDescriptor = generator.normalizeImportedResourceEntry(importedResourceEntry);
    QVERIFY(normalizedDescriptor.value(QStringLiteral("valid")).toBool());
    QCOMPARE(
        normalizedDescriptor.value(QStringLiteral("resourcePath")).toString(),
        QStringLiteral("images/<Cover>.PNG"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("bucket")).toString(), QStringLiteral("Image"));
    QCOMPARE(normalizedDescriptor.value(QStringLiteral("format")).toString(), QStringLiteral(".PNG"));

    const QString generatedTagText = generator.buildCanonicalResourceTag(importedResourceEntry);
    QCOMPARE(
        generatedTagText,
        QStringLiteral("<resource type=\"image\" format=\".PNG\" path=\"images/&lt;Cover&gt;.PNG\" id=\"res-1\" />"));
    QCOMPARE(generator.lastGeneratedTagText(), generatedTagText);
    QCOMPARE(
        generator.lastGeneratedDescriptor().value(QStringLiteral("resourcePath")).toString(),
        QStringLiteral("images/<Cover>.PNG"));
    QCOMPARE(generatedTagTextChangedSpy.count(), 1);
    QCOMPARE(generatedDescriptorChangedSpy.count(), 1);

    QCOMPARE(generator.buildCanonicalResourceTag(importedResourceEntry), generatedTagText);
    QCOMPARE(generatedTagTextChangedSpy.count(), 1);
    QCOMPARE(generatedDescriptorChangedSpy.count(), 1);

    QCOMPARE(
        WhatSon::NoteFolders::normalizeFolderPath(QStringLiteral(" /Research//Today/ ")),
        QStringLiteral("Research/Today"));
    QCOMPARE(
        WhatSon::NoteFolders::normalizeFolderPath(QStringLiteral(" Marketing\\/Sales ")),
        QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::leafFolderName(QStringLiteral(" /Research//Today/ ")),
        QStringLiteral("Today"));
    QCOMPARE(
        WhatSon::NoteFolders::leafFolderName(QStringLiteral("Marketing\\/Sales")),
        QStringLiteral("Marketing/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::displayFolderPath(QStringLiteral("Marketing\\/Sales/Pipeline")),
        QStringLiteral("Marketing/Sales/Pipeline"));
    QCOMPARE(
        WhatSon::NoteFolders::folderPathSegments(QStringLiteral("Marketing\\/Sales/Pipeline")).size(),
        2);
    QVERIFY(!WhatSon::NoteFolders::isHierarchicalFolderPath(QStringLiteral("Marketing\\/Sales")));
    QVERIFY(WhatSon::NoteFolders::isHierarchicalFolderPath(QStringLiteral("Marketing\\/Sales/Pipeline")));
    QCOMPARE(
        WhatSon::NoteFolders::appendFolderPathSegment(QString(), QStringLiteral("Marketing/Sales")),
        QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(
        WhatSon::NoteFolders::appendFolderPathSegment(
            QStringLiteral("Marketing\\/Sales"),
            QStringLiteral("Pipeline")),
        QStringLiteral("Marketing\\/Sales/Pipeline"));
    QVERIFY(WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/today")));
    QVERIFY(!WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/Tomorrow")));
    QVERIFY(!WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research\\/today")));

    const WhatSon::NoteFolders::RawFoldersBlockState missingFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(QStringLiteral("<header />"));
    QVERIFY(!missingFoldersBlock.blockPresent);
    QVERIFY(!missingFoldersBlock.hasConcreteEntry);

    const WhatSon::NoteFolders::RawFoldersBlockState emptyFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(
            QStringLiteral("<folders>\n  <folder>   </folder>\n</folders>"));
    QVERIFY(emptyFoldersBlock.blockPresent);
    QVERIFY(!emptyFoldersBlock.hasConcreteEntry);

    const WhatSon::NoteFolders::RawFoldersBlockState concreteFoldersBlock =
        WhatSon::NoteFolders::inspectRawFoldersBlock(
            QStringLiteral("<folders><folder>Research</folder></folders>"));
    QVERIFY(concreteFoldersBlock.blockPresent);
    QVERIFY(concreteFoldersBlock.hasConcreteEntry);
}

void WhatSonCppRegressionTests::foldersHierarchyParser_escapesLiteralSlashLabelsIntoSingleSegments()
{
    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    QString errorMessage;
    bool uuidMigrationRequired = false;

    const QString rawText = QStringLiteral(R"JSON(
{
  "version": 1,
  "schema": "whatson.folders.tree",
  "folders": [
    {
      "id": "Marketing/Sales",
      "label": "Marketing/Sales",
      "depth": 0,
      "uuid": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
      "children": [
        {
          "id": "Marketing/Sales/Pipeline",
          "label": "Pipeline",
          "depth": 1,
          "uuid": "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        }
      ]
    }
  ]
}
)JSON");

    QVERIFY(parser.parse(rawText, &store, &errorMessage, &uuidMigrationRequired));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(!uuidMigrationRequired);

    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).label, QStringLiteral("Marketing/Sales"));
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(0).id, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(entries.at(1).label, QStringLiteral("Pipeline"));
    QCOMPARE(entries.at(1).depth, 1);
    QCOMPARE(entries.at(1).id, QStringLiteral("Marketing\\/Sales/Pipeline"));
}

void WhatSonCppRegressionTests::foldersHierarchySessionService_preservesEscapedLiteralSlashFolderPaths()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString contentsPath = workspaceDir.filePath(QStringLiteral("Workspace.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString noteDirectoryPath = QDir(contentsPath).filePath(QStringLiteral("Notes/Current"));
    QVERIFY(QDir().mkpath(noteDirectoryPath));

    const QString foldersFilePath = QDir(contentsPath).filePath(QStringLiteral("Folders.wsfolders"));
    QFile foldersFile(foldersFilePath);
    QVERIFY(foldersFile.open(QIODevice::WriteOnly | QIODevice::Text));
    foldersFile.write(R"JSON(
{
  "version": 1,
  "schema": "whatson.folders.tree",
  "folders": [
    {
      "id": "Marketing\\/Sales",
      "label": "Marketing/Sales",
      "depth": 0,
      "uuid": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    }
  ]
}
)JSON");
    foldersFile.close();

    WhatSonFoldersHierarchySessionService service;
    WhatSonFoldersHierarchySessionService::FolderResolution resolution;
    QString errorMessage;

    QVERIFY(service.ensureFolderEntry(
        noteDirectoryPath,
        QStringLiteral("Marketing\\/Sales"),
        &resolution,
        &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(resolution.folderPath, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(resolution.folderUuid, QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    QCOMPARE(resolution.foldersFilePath, foldersFilePath);
    QVERIFY(!resolution.folderCreated);
    QVERIFY(!resolution.hierarchyChanged);

    WhatSonFoldersHierarchyParser parser;
    WhatSonFoldersHierarchyStore store;
    bool uuidMigrationRequired = false;

    QVERIFY(foldersFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString persistedText = QString::fromUtf8(foldersFile.readAll());
    foldersFile.close();

    QVERIFY(parser.parse(persistedText, &store, &errorMessage, &uuidMigrationRequired));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QVERIFY(!uuidMigrationRequired);

    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.constFirst().id, QStringLiteral("Marketing\\/Sales"));
    QCOMPARE(entries.constFirst().label, QStringLiteral("Marketing/Sales"));
    QCOMPARE(entries.constFirst().depth, 0);
}

void WhatSonCppRegressionTests::sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels()
{
    const QString renameControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/sidebar/SidebarHierarchyRenameController.qml"));

    QVERIFY(!renameControllerSource.isEmpty());
    QVERIFY(renameControllerSource.contains(QStringLiteral("function decodedHierarchyPathSegments(rawPath)")));
    QVERIFY(renameControllerSource.contains(QStringLiteral("function leafHierarchyItemLabel(rawLabel, rawPath)")));
    QVERIFY(renameControllerSource.contains(QStringLiteral("if (nextCharacter === \"\\\\\" || nextCharacter === \"/\")")));
    QVERIFY(renameControllerSource.contains(QStringLiteral("renameController.leafHierarchyItemLabel(item.label, item.id)")));
    QVERIFY(renameControllerSource.contains(QStringLiteral("item && item.id !== undefined && item.id !== null ? item.id : \"\"")));
    QVERIFY(!renameControllerSource.contains(QStringLiteral("const segments = normalizedLabel.split(\"/\")")));
}

void WhatSonCppRegressionTests::resourcesHierarchyViewModel_defaultsSelectionToImageAndFiltersList()
{
    ResourcesHierarchyViewModel viewModel;
    viewModel.setResourcePaths({
        QStringLiteral("images/Cover.PNG"),
        QStringLiteral("documents/Report.pdf")
    });

    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Image"));
    QVERIFY(viewModel.noteListModel() != nullptr);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);

    const QModelIndex imageIndex = viewModel.noteListModel()->index(0, 0);
    QVERIFY(imageIndex.isValid());
    QCOMPARE(
        viewModel.noteListModel()->data(imageIndex, ResourcesListModel::TypeRole).toString(),
        QStringLiteral("image"));

    viewModel.setSelectedIndex(-1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(viewModel.noteListModel()->itemCount(), 1);
}

void WhatSonCppRegressionTests::structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches()
{
    ContentsStructuredDocumentCollectionPolicy policy;

    const QVariantMap indexedEntries{
        {QStringLiteral("10"), QStringLiteral("ten")},
        {QStringLiteral("2"), QStringLiteral("two")},
        {QStringLiteral("0"), QStringLiteral("zero")},
        {QStringLiteral("ignored"), QStringLiteral("skip")},
    };
    const QVariantList normalizedEntries = policy.normalizeEntries(indexedEntries);
    QCOMPARE(normalizedEntries.size(), 3);
    QCOMPARE(normalizedEntries.at(0).toString(), QStringLiteral("zero"));
    QCOMPARE(normalizedEntries.at(1).toString(), QStringLiteral("two"));
    QCOMPARE(normalizedEntries.at(2).toString(), QStringLiteral("ten"));

    QCOMPARE(
        policy.normalizeSourceText(QStringLiteral("a\r\nb\rc\u2028d\u2029e\u00a0f")),
        QStringLiteral("a\nb\nc\nd\ne f"));
    QCOMPARE(policy.spliceSourceRange(QStringLiteral("abc"), -1, 99, QStringLiteral("X")), QStringLiteral("X"));
    QCOMPARE(policy.floorNumberOrFallback(QVariant(12.9), -1), 12);
    QCOMPARE(policy.floorNumberOrFallback(QVariant(QStringLiteral("not-a-number")), 7), 7);

    const QVariantMap blockEntry{
        {QStringLiteral("resourceIndex"), 7},
        {QStringLiteral("sourceStart"), 10},
        {QStringLiteral("sourceEnd"), 20},
        {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
        {QStringLiteral("resourcePath"), QStringLiteral("/tmp/resource-7")},
    };
    const QVariantList renderedResources{
        QVariantMap{
            {QStringLiteral("index"), 7},
            {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
        },
        QVariantMap{
            {QStringLiteral("index"), 7},
            {QStringLiteral("resourceId"), QStringLiteral("resource-7")},
            {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/resource-7")},
        },
    };

    const QVariantMap resolvedEntry = policy.resourceEntryForBlock(blockEntry, renderedResources);
    QCOMPARE(resolvedEntry.value(QStringLiteral("index")).toInt(), 7);
    QCOMPARE(
        resolvedEntry.value(QStringLiteral("resolvedPath")).toString(),
        QStringLiteral("/tmp/resource-7"));
}

void WhatSonCppRegressionTests::structuredMutationPolicy_buildsDeletionAndInsertionPayloads()
{
    ContentsStructuredDocumentMutationPolicy policy;

    const QString sourceWithEmptyBlock = QStringLiteral("aa\n\nbb");
    const QVariantMap emptyBlockData{{QStringLiteral("sourceStart"), 3}};

    const QVariantMap backwardDeletion =
        policy.emptyTextBlockDeletionRange(emptyBlockData, QStringLiteral("backward"), sourceWithEmptyBlock);
    QCOMPARE(backwardDeletion.value(QStringLiteral("start")).toInt(), 2);
    QCOMPARE(backwardDeletion.value(QStringLiteral("end")).toInt(), 3);
    QCOMPARE(
        backwardDeletion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        2);

    const QVariantMap forwardDeletion =
        policy.emptyTextBlockDeletionRange(emptyBlockData, QStringLiteral("forward"), sourceWithEmptyBlock);
    QCOMPARE(forwardDeletion.value(QStringLiteral("start")).toInt(), 3);
    QCOMPARE(forwardDeletion.value(QStringLiteral("end")).toInt(), 4);
    QCOMPARE(
        forwardDeletion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        3);

    QCOMPARE(policy.nextEditableSourceOffsetAfterBlock(QStringLiteral("a\nb"), 1), 2);
    QCOMPARE(policy.nextEditableSourceOffsetAfterBlock(QStringLiteral("abc"), 2), 2);

    const QVariantMap structuredInsertion = policy.buildStructuredInsertionPayload(
        QStringLiteral("abc"),
        1,
        QStringLiteral("block"),
        2);
    QCOMPARE(
        structuredInsertion.value(QStringLiteral("insertedSourceText")).toString(),
        QStringLiteral("\nblock\n"));
    QCOMPARE(
        structuredInsertion.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("a\nblock\nbc"));
    QCOMPARE(structuredInsertion.value(QStringLiteral("sourceOffset")).toInt(), 4);

    const QString expectedResourceBlock =
        QStringLiteral("<resource one />\n<resource two />");
    const QString expectedInsertedResourceText =
        QStringLiteral("\n%1\n").arg(expectedResourceBlock);
    const QVariantMap resourceInsertion = policy.buildResourceInsertionPayload(
        QStringLiteral("abc"),
        1,
        QStringList{
            QStringLiteral("  <resource one />  "),
            QString(),
            QStringLiteral("<resource two />"),
        });
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("insertedSourceText")).toString(),
        expectedInsertedResourceText);
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("a%1bc").arg(expectedInsertedResourceText));

    const int expectedFocusOffset = 1 + 1 + static_cast<int>(expectedResourceBlock.size()) + 1;
    QCOMPARE(
        resourceInsertion.value(QStringLiteral("focusRequest")).toMap().value(QStringLiteral("sourceOffset")).toInt(),
        expectedFocusOffset);

    QVERIFY(policy.buildResourceInsertionPayload(QString(), 0, QVariantList{}).isEmpty());
}

void WhatSonCppRegressionTests::structuredMutationPolicy_buildsParagraphBoundaryMergeAndSplitPayloads()
{
    ContentsStructuredDocumentMutationPolicy policy;

    const QVariantMap implicitParagraphBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 6},
    };
    const QVariantMap titleBlock{
        {QStringLiteral("type"), QStringLiteral("title")},
        {QStringLiteral("semanticTagName"), QStringLiteral("title")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 5},
    };
    QVERIFY(policy.supportsParagraphBoundaryOperations(implicitParagraphBlock));
    QVERIFY(!policy.supportsParagraphBoundaryOperations(titleBlock));

    const QString implicitMergeSource = QStringLiteral("foo\nbar");
    const QVariantMap implicitPreviousBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 0},
        {QStringLiteral("sourceEnd"), 3},
    };
    const QVariantMap implicitCurrentBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("sourceStart"), 4},
        {QStringLiteral("sourceEnd"), 7},
    };
    const QVariantMap implicitMergePayload = policy.buildParagraphMergePayload(
        implicitPreviousBlock,
        implicitCurrentBlock,
        implicitMergeSource);
    QCOMPARE(
        implicitMergePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("foobar"));
    QCOMPARE(
        implicitMergePayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        3);

    const QString explicitMergeSource =
        QStringLiteral("<paragraph>foo</paragraph>\n<paragraph>bar</paragraph>");
    const QVariantMap explicitPreviousBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("tagName"), QStringLiteral("paragraph")},
        {QStringLiteral("blockSourceStart"), 0},
        {QStringLiteral("blockSourceEnd"), 26},
        {QStringLiteral("contentStart"), 11},
        {QStringLiteral("contentEnd"), 14},
        {QStringLiteral("sourceStart"), 11},
        {QStringLiteral("sourceEnd"), 14},
    };
    const QVariantMap explicitCurrentBlock{
        {QStringLiteral("type"), QStringLiteral("paragraph")},
        {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
        {QStringLiteral("tagName"), QStringLiteral("paragraph")},
        {QStringLiteral("blockSourceStart"), 27},
        {QStringLiteral("blockSourceEnd"), 53},
        {QStringLiteral("contentStart"), 38},
        {QStringLiteral("contentEnd"), 41},
        {QStringLiteral("sourceStart"), 38},
        {QStringLiteral("sourceEnd"), 41},
    };
    const QVariantMap explicitMergePayload = policy.buildParagraphMergePayload(
        explicitPreviousBlock,
        explicitCurrentBlock,
        explicitMergeSource);
    QCOMPARE(
        explicitMergePayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<paragraph>foobar</paragraph>"));
    QCOMPARE(
        explicitMergePayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        14);

    const QVariantMap implicitSplitPayload = policy.buildParagraphSplitPayload(
        implicitParagraphBlock,
        QStringLiteral("foobar"),
        3);
    QCOMPARE(
        implicitSplitPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("foo\nbar"));
    QCOMPARE(
        implicitSplitPayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        4);

    const QVariantMap explicitSplitPayload = policy.buildParagraphSplitPayload(
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("paragraph")},
            {QStringLiteral("semanticTagName"), QStringLiteral("paragraph")},
            {QStringLiteral("tagName"), QStringLiteral("paragraph")},
            {QStringLiteral("blockSourceStart"), 0},
            {QStringLiteral("blockSourceEnd"), 29},
            {QStringLiteral("contentStart"), 11},
            {QStringLiteral("contentEnd"), 17},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 17},
        },
        QStringLiteral("<paragraph>foobar</paragraph>"),
        14);
    QCOMPARE(
        explicitSplitPayload.value(QStringLiteral("nextSourceText")).toString(),
        QStringLiteral("<paragraph>foo</paragraph>\n<paragraph>bar</paragraph>"));
    QCOMPARE(
        explicitSplitPayload.value(QStringLiteral("focusRequest"))
            .toMap()
            .value(QStringLiteral("sourceOffset"))
            .toInt(),
        38);
}

void WhatSonCppRegressionTests::structuredDocumentBlocksModel_updatesRowsWithoutResettingStableSuffixBlocks()
{
    ContentsStructuredDocumentBlocksModel model;
    QSignalSpy rowsInsertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&model, &QAbstractItemModel::rowsRemoved);
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

    const QVariantList initialBlocks{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("resource")},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 24},
            {QStringLiteral("sourceText"), QStringLiteral("<resource />")},
        },
    };
    model.setBlocks(initialBlocks);
    QCOMPARE(model.count(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    rowsInsertedSpy.clear();
    rowsRemovedSpy.clear();
    dataChangedSpy.clear();

    const QVariantList editedBlocks{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 2},
            {QStringLiteral("sourceText"), QStringLiteral("al")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 3},
            {QStringLiteral("sourceEnd"), 7},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("resource")},
            {QStringLiteral("sourceStart"), 8},
            {QStringLiteral("sourceEnd"), 21},
            {QStringLiteral("sourceText"), QStringLiteral("<resource />")},
        },
    };
    model.setBlocks(editedBlocks);

    QCOMPARE(model.count(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto changedRange = dataChangedSpy.at(0);
    QCOMPARE(changedRange.at(0).value<QModelIndex>().row(), 0);
    QCOMPARE(changedRange.at(1).value<QModelIndex>().row(), 2);

    QCOMPARE(
        model.data(model.index(2, 0), ContentsStructuredDocumentBlocksModel::BlockDataRole).toMap().value(
            QStringLiteral("sourceStart")).toInt(),
        8);
}

void WhatSonCppRegressionTests::structuredDocumentBlocksModel_removesOnlyChangedMiddleRows()
{
    ContentsStructuredDocumentBlocksModel model;
    QSignalSpy rowsInsertedSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&model, &QAbstractItemModel::rowsRemoved);
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

    model.setBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 10},
            {QStringLiteral("sourceText"), QStringLiteral("beta")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 11},
            {QStringLiteral("sourceEnd"), 16},
            {QStringLiteral("sourceText"), QStringLiteral("gamma")},
        },
    });

    rowsInsertedSpy.clear();
    rowsRemovedSpy.clear();
    dataChangedSpy.clear();

    model.setBlocks(QVariantList{
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceEnd"), 5},
            {QStringLiteral("sourceText"), QStringLiteral("alpha")},
        },
        QVariantMap{
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("sourceStart"), 6},
            {QStringLiteral("sourceEnd"), 11},
            {QStringLiteral("sourceText"), QStringLiteral("gamma")},
        },
    });

    QCOMPARE(model.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto removedRange = rowsRemovedSpy.takeFirst();
    QCOMPARE(removedRange.at(1).toInt(), 1);
    QCOMPARE(removedRange.at(2).toInt(), 1);

    const auto changedRange = dataChangedSpy.takeFirst();
    QCOMPARE(changedRange.at(0).value<QModelIndex>().row(), 1);
    QCOMPARE(changedRange.at(1).value<QModelIndex>().row(), 1);
    QCOMPARE(
        model.data(model.index(1, 0), ContentsStructuredDocumentBlocksModel::BlockDataRole).toMap().value(
            QStringLiteral("sourceText")).toString(),
        QStringLiteral("gamma"));
}

void WhatSonCppRegressionTests::structuredDocumentHost_tracksSelectionClearRevisionAcrossInteractions()
{
    ContentsStructuredDocumentHost host;

    QSignalSpy activeBlockIndexSpy(&host, &ContentsStructuredDocumentHost::activeBlockIndexChanged);
    QSignalSpy activeBlockCursorRevisionSpy(
        &host,
        &ContentsStructuredDocumentHost::activeBlockCursorRevisionChanged);
    QSignalSpy selectionClearRevisionSpy(
        &host,
        &ContentsStructuredDocumentHost::selectionClearRevisionChanged);
    QSignalSpy selectionClearRetainedBlockIndexSpy(
        &host,
        &ContentsStructuredDocumentHost::selectionClearRetainedBlockIndexChanged);

    QCOMPARE(host.activeBlockIndex(), -1);
    QCOMPARE(host.activeBlockCursorRevision(), 0);
    QCOMPARE(host.selectionClearRevision(), 0);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), -1);

    host.requestSelectionClear(3);
    QCOMPARE(host.selectionClearRevision(), 1);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 3);
    QCOMPARE(selectionClearRevisionSpy.count(), 1);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 1);

    host.requestSelectionClear(3);
    QCOMPARE(host.selectionClearRevision(), 2);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 3);
    QCOMPARE(selectionClearRevisionSpy.count(), 2);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 1);

    host.noteActiveBlockInteraction(5);
    QCOMPARE(host.activeBlockIndex(), 5);
    QCOMPARE(host.activeBlockCursorRevision(), 1);
    QCOMPARE(host.selectionClearRevision(), 3);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 5);
    QCOMPARE(activeBlockIndexSpy.count(), 1);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 1);
    QCOMPARE(selectionClearRevisionSpy.count(), 3);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 2);

    host.noteActiveBlockInteraction(5);
    QCOMPARE(host.activeBlockIndex(), 5);
    QCOMPARE(host.activeBlockCursorRevision(), 2);
    QCOMPARE(host.selectionClearRevision(), 4);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), 5);
    QCOMPARE(activeBlockIndexSpy.count(), 1);
    QCOMPARE(activeBlockCursorRevisionSpy.count(), 2);
    QCOMPARE(selectionClearRevisionSpy.count(), 4);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 2);

    host.requestSelectionClear(-1);
    QCOMPARE(host.selectionClearRevision(), 5);
    QCOMPARE(host.selectionClearRetainedBlockIndex(), -1);
    QCOMPARE(selectionClearRevisionSpy.count(), 5);
    QCOMPARE(selectionClearRetainedBlockIndexSpy.count(), 3);
}

void WhatSonCppRegressionTests::logicalLineLayoutSupport_mapsEditorRectanglesIntoBlockCoordinates()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/qml/view/content/editor/ContentsLogicalLineLayoutSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue buildEntries = library.property(QStringLiteral("buildEntries"));
    QVERIFY(buildEntries.isCallable());

    QJSValue editorItem = engine.newObject();
    editorItem.setProperty(
        QStringLiteral("positionToRectangle"),
        engine.evaluate(
            QStringLiteral(
                "(function (position) {"
                "  if (position <= 0)"
                "    return { y: 0, height: 14 };"
                "  return { y: 18, height: 14 };"
                "})")));
    editorItem.setProperty(
        QStringLiteral("mapToItem"),
        engine.evaluate(
            QStringLiteral(
                "(function (_target, _x, y) {"
                "  return { x: 0, y: y + 11 };"
                "})")));

    const QJSValue entries = buildEntries.call(QJSValueList {
        QJSValue(QStringLiteral("alpha\nbeta")),
        QJSValue(52),
        editorItem,
        engine.newObject(),
        QJSValue(12),
    });

    QVERIFY2(!entries.isError(), qPrintable(entries.toString()));
    QCOMPARE(entries.property(QStringLiteral("length")).toInt(), 2);

    const QJSValue firstEntry = jsArrayEntry(entries, 0);
    const QJSValue secondEntry = jsArrayEntry(entries, 1);
    QVERIFY(firstEntry.isObject());
    QVERIFY(secondEntry.isObject());
    QCOMPARE(firstEntry.property(QStringLiteral("contentY")).toInt(), 11);
    QCOMPARE(firstEntry.property(QStringLiteral("contentHeight")).toInt(), 18);
    QCOMPARE(secondEntry.property(QStringLiteral("contentY")).toInt(), 29);
    QCOMPARE(secondEntry.property(QStringLiteral("contentHeight")).toInt(), 23);
}

void WhatSonCppRegressionTests::logicalLineLayoutSupport_fallsBackWhenLiveEditorGeometryIsUnavailable()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/qml/view/content/editor/ContentsLogicalLineLayoutSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue buildEntries = library.property(QStringLiteral("buildEntries"));
    QVERIFY(buildEntries.isCallable());

    const QJSValue entries = buildEntries.call(QJSValueList {
        QJSValue(QStringLiteral("")),
        QJSValue(0),
        QJSValue(QJSValue::UndefinedValue),
        QJSValue(QJSValue::UndefinedValue),
        QJSValue(12),
    });

    QVERIFY2(!entries.isError(), qPrintable(entries.toString()));
    QCOMPARE(entries.property(QStringLiteral("length")).toInt(), 1);

    const QJSValue onlyEntry = jsArrayEntry(entries, 0);
    QVERIFY(onlyEntry.isObject());
    QCOMPARE(onlyEntry.property(QStringLiteral("contentY")).toInt(), 0);
    QCOMPARE(onlyEntry.property(QStringLiteral("contentHeight")).toInt(), 12);
}

void WhatSonCppRegressionTests::editorSurfaceModeSupport_switchesToResourceEditorForResourceListModels()
{
    ensureCoreApplication();
    QJSEngine engine;
    const QJSValue library = evaluateQmlJsLibrary(
        &engine,
        QStringLiteral("src/app/qml/view/content/editor/ContentsEditorSurfaceModeSupport.js"));
    QVERIFY2(!library.isError(), qPrintable(library.toString()));

    const QJSValue resourceEditorVisible = library.property(QStringLiteral("resourceEditorVisible"));
    const QJSValue currentResourceEntry = library.property(QStringLiteral("currentResourceEntry"));
    const QJSValue hasCurrentResourceEntry = library.property(QStringLiteral("hasCurrentResourceEntry"));
    QVERIFY(resourceEditorVisible.isCallable());
    QVERIFY(currentResourceEntry.isCallable());
    QVERIFY(hasCurrentResourceEntry.isCallable());

    QJSValue resourceEntry = engine.newObject();
    resourceEntry.setProperty(QStringLiteral("displayName"), QStringLiteral("Cover.PNG"));
    resourceEntry.setProperty(QStringLiteral("renderMode"), QStringLiteral("image"));

    QJSValue resourceListModel = engine.newObject();
    resourceListModel.setProperty(QStringLiteral("noteBacked"), false);
    resourceListModel.setProperty(QStringLiteral("currentResourceEntry"), resourceEntry);

    QJSValue noteListModel = engine.newObject();
    noteListModel.setProperty(QStringLiteral("noteBacked"), true);
    noteListModel.setProperty(QStringLiteral("currentResourceEntry"), resourceEntry);

    QVERIFY(resourceEditorVisible.call(QJSValueList{resourceListModel}).toBool());
    QVERIFY(!resourceEditorVisible.call(QJSValueList{noteListModel}).toBool());
    QVERIFY(!resourceEditorVisible.call(QJSValueList{QJSValue(QJSValue::UndefinedValue)}).toBool());

    const QJSValue resolvedResourceEntry = currentResourceEntry.call(QJSValueList{resourceListModel});
    QVERIFY(resolvedResourceEntry.isObject());
    QCOMPARE(
        resolvedResourceEntry.property(QStringLiteral("displayName")).toString(),
        QStringLiteral("Cover.PNG"));
    QVERIFY(hasCurrentResourceEntry.call(QJSValueList{resourceListModel}).toBool());

    const QJSValue emptyResourceEntry = currentResourceEntry.call(QJSValueList{noteListModel});
    QVERIFY(emptyResourceEntry.isObject());
    QVERIFY(!hasCurrentResourceEntry.call(QJSValueList{noteListModel}).toBool());
    QVERIFY(emptyResourceEntry.property(QStringLiteral("displayName")).isUndefined());
}

void WhatSonCppRegressionTests::qmlResourceEditorView_staysTransparentAndViewerOnly()
{
    const QString resourceEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceEditorView.qml"));

    QVERIFY(!resourceEditorSource.isEmpty());
    QVERIFY(resourceEditorSource.contains(QStringLiteral("Item {")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("property color displayColor: \"transparent\"")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("ContentsResourceViewer {")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("anchors.fill: parent")));
    QVERIFY(resourceEditorSource.contains(QStringLiteral("visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("LV.Theme.panelBackground03")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("#CC0F141A")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("No Resource Selected")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("Preview Unavailable")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("The dedicated resource editor currently previews image resources first.")));
    QVERIFY(!resourceEditorSource.contains(QStringLiteral("The resource editor now owns direct resource preview for the Resources hierarchy.")));
}

void WhatSonCppRegressionTests::resourceDetailPanelViewModel_tracksCurrentResourceSelection()
{
    ResourceDetailPanelViewModel viewModel;
    ResourcesListModel resourceListModel;

    QSignalSpy resourceListModelChangedSpy(
        &viewModel,
        &ResourceDetailPanelViewModel::resourceListModelChanged);
    QSignalSpy currentResourceEntryChangedSpy(
        &viewModel,
        &ResourceDetailPanelViewModel::currentResourceEntryChanged);

    ResourcesListItem firstItem;
    firstItem.id = QStringLiteral("resource-1");
    firstItem.displayName = QStringLiteral("Cover");
    firstItem.resourcePath = QStringLiteral("resources/cover.png");
    firstItem.resolvedPath = QStringLiteral("/tmp/resources/cover.png");
    firstItem.type = QStringLiteral("image");
    firstItem.renderMode = QStringLiteral("image");

    ResourcesListItem secondItem;
    secondItem.id = QStringLiteral("resource-2");
    secondItem.displayName = QStringLiteral("Palette");
    secondItem.resourcePath = QStringLiteral("resources/palette.png");
    secondItem.resolvedPath = QStringLiteral("/tmp/resources/palette.png");
    secondItem.type = QStringLiteral("image");
    secondItem.renderMode = QStringLiteral("image");

    resourceListModel.setItems({firstItem, secondItem});
    resourceListModel.setCurrentIndex(1);

    viewModel.setCurrentResourceListModel(&resourceListModel);

    QCOMPARE(resourceListModelChangedSpy.count(), 1);
    QVERIFY(viewModel.resourceContextLinked());
    QCOMPARE(viewModel.resourceListModel(), static_cast<QObject*>(&resourceListModel));
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        secondItem.resourcePath);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        secondItem.displayName);

    resourceListModel.setCurrentIndex(0);

    QVERIFY(currentResourceEntryChangedSpy.count() >= 2);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("resourcePath")).toString(),
        firstItem.resourcePath);
    QCOMPARE(
        viewModel.currentResourceEntry().value(QStringLiteral("displayName")).toString(),
        firstItem.displayName);

    viewModel.setCurrentResourceListModel(nullptr);

    QCOMPARE(resourceListModelChangedSpy.count(), 2);
    QVERIFY(!viewModel.resourceContextLinked());
    QVERIFY(viewModel.currentResourceEntry().isEmpty());
}

void WhatSonCppRegressionTests::detailPanelRouting_separatesNoteAndResourceViewsAndViewModels()
{
    const QString detailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/DetailPanel.qml"));
    const QString noteDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/NoteDetailPanel.qml"));
    const QString resourceDetailPanelSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/panels/detail/ResourceDetailPanel.qml"));
    const QString binderSource = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/detailPanel/DetailPanelCurrentHierarchyBinder.cpp"));
    const QString mainQmlSource = readUtf8SourceFile(QStringLiteral("src/app/qml/Main.qml"));
    const QString mainCppSource = readUtf8SourceFile(QStringLiteral("src/app/main.cpp"));

    QVERIFY(!detailPanelSource.isEmpty());
    QVERIFY(!noteDetailPanelSource.isEmpty());
    QVERIFY(!resourceDetailPanelSource.isEmpty());
    QVERIFY(!binderSource.isEmpty());
    QVERIFY(!mainQmlSource.isEmpty());
    QVERIFY(!mainCppSource.isEmpty());

    QVERIFY(detailPanelSource.contains(QStringLiteral("LV.ViewModels.get(\"noteDetailPanelViewModel\")")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("LV.ViewModels.get(\"resourceDetailPanelViewModel\")")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("NoteDetailPanel {")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("ResourceDetailPanel {")));
    QVERIFY(detailPanelSource.contains(QStringLiteral("sidebarHierarchyVm.activeHierarchyViewModel === resourcesHierarchyVm")));

    QVERIFY(noteDetailPanelSource.contains(QStringLiteral("property var noteDetailPanelViewModel: null")));
    QVERIFY(noteDetailPanelSource.contains(QStringLiteral("readonly property var detailPanelVm: noteDetailPanel.noteDetailPanelViewModel")));
    QVERIFY(resourceDetailPanelSource.contains(QStringLiteral("property var resourceDetailPanelViewModel: null")));

    QVERIFY(mainQmlSource.contains(QStringLiteral("LV.ViewModels.set(\"noteDetailPanelViewModel\", noteDetailPanelViewModel);")));
    QVERIFY(mainQmlSource.contains(QStringLiteral("LV.ViewModels.set(\"resourceDetailPanelViewModel\", resourceDetailPanelViewModel);")));
    QVERIFY(mainCppSource.contains(QStringLiteral("NoteDetailPanelViewModel noteDetailPanelViewModel;")));
    QVERIFY(mainCppSource.contains(QStringLiteral("ResourceDetailPanelViewModel resourceDetailPanelViewModel;")));

    QVERIFY(binderSource.contains(QStringLiteral("setNoteDetailPanelViewModel")));
    QVERIFY(binderSource.contains(QStringLiteral("setResourceDetailPanelViewModel")));
    QVERIFY(binderSource.contains(QStringLiteral("HierarchyDomain::Resources")));
    QVERIFY(binderSource.contains(QStringLiteral("setCurrentResourceListModel")));
}

void WhatSonCppRegressionTests::contentsDisplayView_invalidatesGutterGeometryImmediatelyAcrossRapidNoteSwitches()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("property string minimapLineGroupsNoteId: \"\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function hasPendingNoteEntryGutterRefresh(noteId)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function finalizePendingNoteEntryGutterRefresh(noteId, reason, refreshStructuredLayout)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function resetNoteEntryLineGeometryState()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId === contentsView.activeLineGeometryNoteId()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("currentNoteId === contentsView.minimapLineGroupsNoteId")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId = currentNoteId;")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.minimapLineGroupsNoteId = \"\";")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.scheduleLayoutCacheRefresh();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleViewportGutterRefresh();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleGutterRefresh(6, \"note-entry\");")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"structured-layout-cache\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("\"editor-text-synchronized\"")));
}

void WhatSonCppRegressionTests::contentsDisplayView_keepsGutterNumbersCloseToTheEditorBody()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int gutterBodyGap")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property int lineNumberRightInset: contentsView.gutterBodyGap")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("readonly property int lineNumberRightInset: contentsView.editorHorizontalInset")));
}

void WhatSonCppRegressionTests::contentsDisplayView_reservesHalfHeightBottomInsetAndCorrectsTypingViewport()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(displayViewSource.contains(QStringLiteral("Math.round(contentsView.editorSurfaceHeight * 0.5)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function typingViewportBandTop(cursorHeight)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function typingViewportBandBottom(cursorHeight)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function typingViewportAnchorCenter(cursorHeight)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function correctTypingViewport(forceAnchor)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function scheduleTypingViewportCorrection(forceAnchor)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleTypingViewportCorrection(true);")));
    QVERIFY(displayViewSource.contains(QStringLiteral("contentsView.scheduleTypingViewportCorrection(false);")));
    QVERIFY(displayViewSource.contains(QStringLiteral("flickable.contentY = nextContentY;")));
}

void WhatSonCppRegressionTests::inlineFormatEditor_preservesMacModifierVerticalNavigationHooks()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler: null")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function previousParagraphCursorPosition(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function nextParagraphCursorPosition(")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function handleMacModifierVerticalNavigation(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.platform.os !== \"osx\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("const commandPressed = (modifiers & Qt.MetaModifier) !== 0;")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("const optionPressed = (modifiers & Qt.AltModifier) !== 0;")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.modifierVerticalNavigationHandler")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textInput.deselect !== undefined")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (control.handleMacModifierVerticalNavigation(event))")));
}

void WhatSonCppRegressionTests::structuredFlow_flattensImplicitTextBlocksIntoInteractiveGroups()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(QStringLiteral("property var documentBlocks: []")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function normalizedParsedBlocks() {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function implicitTextBlockInteractiveFlattenCandidate(blockEntryOverride) {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function buildFlattenedInteractiveTextGroup(groupBlocks, normalizedSourceText) {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function flattenedInteractiveBlocks() {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("documentHost.documentBlocks = documentFlow.flattenedInteractiveBlocks()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"flattenedInteractiveGroup\": true")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"type\": \"text-group\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("if (!!safeBlock.flattenedInteractiveGroup)")));
}

void WhatSonCppRegressionTests::structuredEditorFormattingController_commitsInlineStyleMutationsThroughFlowRawRanges()
{
    const QString formattingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredEditorFormattingController.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));

    QVERIFY(!formattingControllerSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());

    QVERIFY(formattingControllerSource.contains(QStringLiteral("ContentsTextFormatRenderer {")));
    QVERIFY(formattingControllerSource.contains(QStringLiteral("function inlineFormatTargetState() {")));
    QVERIFY(formattingControllerSource.contains(QStringLiteral("function applyInlineFormatToBlockSelection(blockIndex, tagName, explicitSelectionSnapshot) {")));
    QVERIFY(formattingControllerSource.contains(QStringLiteral("documentFlow.replaceSourceRange(")));
    QVERIFY(formattingControllerSource.contains(QStringLiteral("StructuredCursorSupport.sourceOffsetForInlineTaggedCursor(")));
    QVERIFY(formattingControllerSource.contains(QStringLiteral("\"targetBlockIndex\": safeBlockIndex")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("ContentsStructuredEditorFormattingController {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("return structuredEditorFormattingController.inlineFormatTargetState()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("return structuredEditorFormattingController.applyInlineFormatToBlockSelection(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("return structuredEditorFormattingController.applyInlineFormatToActiveSelection(tagName)")));

    QVERIFY(!documentBlockSource.contains(QStringLiteral("function applyInlineFormatToSelection(")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function applyInlineFormatToSelection(")));
}

void WhatSonCppRegressionTests::structuredEditors_routeMacModifierVerticalNavigationAcrossBlockAndDocumentBoundaries()
{
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString breakBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsBreakBlock.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!breakBlockSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());

    QVERIFY(textBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler: function (request, event) {")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textBlock.boundaryNavigationRequested(\"document\", request.moveUp ? \"before\" : \"after\")")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("calloutBlock.boundaryNavigationRequested(\"document\", request.moveUp ? \"before\" : \"after\")")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("agendaBlock.boundaryNavigationRequested(\"document\", request.moveUp ? \"before\" : \"after\")")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("const macModifierVerticalNavigation = Qt.platform.os === \"osx\"")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("documentBlock.boundaryNavigationRequested(\"document\", moveUp ? \"before\" : \"after\")")));
    QVERIFY(breakBlockSource.contains(QStringLiteral("breakBlock.boundaryNavigationRequested(\"document\", moveUp ? \"before\" : \"after\")")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("if (normalizedAxis === \"document\") {")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"sourceOffset\": normalizedSide === \"before\"")));
}

void WhatSonCppRegressionTests::textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml()
{
    ContentsTextFormatRenderer renderer;
    renderer.setSourceText(
        QStringLiteral(
            "<title>Heading</title>\n"
            "<highlight>Glow</highlight>\n"
            "# Print title\n"
            "> Quote\n"
            "`Code`\n"
            "[Doc](https://example.com)"));
    renderer.setPreviewEnabled(true);

    const QString screenEditorHtml = renderer.editorSurfaceHtml();
    const QString screenPreviewHtml = renderer.renderedHtml();
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(screenPreviewHtml.contains(QStringLiteral("#D6AE58")));

    renderer.setPaperPaletteEnabled(true);

    const QString paperEditorHtml = renderer.editorSurfaceHtml();
    const QString paperPreviewHtml = renderer.renderedHtml();
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperEditorHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#F3F5F8")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#8CB4FF")));
    QVERIFY(!paperPreviewHtml.contains(QStringLiteral("#D6AE58")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperEditorHtml.contains(QStringLiteral("background-color:#F4D37A")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#111111")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("color:#1F5FBF")));
    QVERIFY(paperPreviewHtml.contains(QStringLiteral("background-color:#E7EAEE")));
}

void WhatSonCppRegressionTests::textFormatRenderer_wrapsCommittedUrlsIntoCanonicalWebLinks()
{
    ContentsTextFormatRenderer renderer;

    const QString notYetCommittedSource = renderer.applyPlainTextReplacementToSource(
        QStringLiteral("www.iisacc.co"),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("www.iisacc.co").size(),
        QStringLiteral("m"));
    QCOMPARE(notYetCommittedSource, QStringLiteral("www.iisacc.com"));

    const QString committedSource = renderer.applyPlainTextReplacementToSource(
        notYetCommittedSource,
        notYetCommittedSource.size(),
        notYetCommittedSource.size(),
        QStringLiteral(" "));
    QCOMPARE(
        committedSource,
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink> "));

    const QString pastedSource = renderer.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("Visit https://www.iisacc.com/path?q=1"));
    QCOMPARE(
        pastedSource,
        QStringLiteral(
            "Visit <weblink href=\"https://www.iisacc.com/path?q=1\">https://www.iisacc.com/path?q=1</weblink>"));

    const QString markdownSource = renderer.applyPlainTextReplacementToSource(
        QString(),
        0,
        0,
        QStringLiteral("[Doc](https://example.com)"));
    QCOMPARE(markdownSource, QStringLiteral("[Doc](https://example.com)"));

    renderer.setSourceText(QStringLiteral("<weblink href=\"www.iisacc.com\">아이작닷컴</weblink>"));
    renderer.setPreviewEnabled(true);
    QVERIFY(renderer.renderedHtml().contains(
        QStringLiteral("<a href=\"https://www.iisacc.com\" style=\"color:#8CB4FF;text-decoration: underline;\">")));
    QVERIFY(renderer.renderedHtml().contains(QStringLiteral("아이작닷컴</a>")));
}

void WhatSonCppRegressionTests::paperSelection_tracksChosenPaperEnumState()
{
    ContentsPaperSelection selection;
    QSignalSpy selectedPaperKindChangedSpy(&selection, &ContentsPaperSelection::selectedPaperKindChanged);

    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::A4);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("A4"));
    QCOMPARE(
        ContentsPaperSelection::paperStandardForKind(ContentsPaperSelection::Letter),
        QStringLiteral("Letter"));
    QCOMPARE(selection.paperStandardForValue(ContentsPaperSelection::Legal), QStringLiteral("Legal"));

    selection.setSelectedPaperKind(ContentsPaperSelection::B5);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::B5);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("B5"));

    selection.setSelectedPaperKindByValue(ContentsPaperSelection::A5);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::A5);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("A5"));

    selection.setSelectedPaperKindByValue(999);
    QCOMPARE(selection.selectedPaperKind(), ContentsPaperSelection::Unknown);
    QCOMPARE(selection.selectedPaperStandard(), QStringLiteral("Unknown"));
    QCOMPARE(selectedPaperKindChangedSpy.count(), 3);
}

void WhatSonCppRegressionTests::a4PaperBackground_exposesCanonicalMetricsAndAnchorsPrintRendererDefaults()
{
    ContentsA4PaperBackground background;
    ContentsPagePrintLayoutRenderer layoutRenderer;

    QCOMPARE(background.paperKind(), ContentsPaperSelection::A4);
    QCOMPARE(background.paperStandard(), QStringLiteral("A4"));
    QCOMPARE(background.widthMillimeters(), 210.0);
    QCOMPARE(background.heightMillimeters(), 297.0);
    QCOMPARE(background.sizeMillimeters(), QSizeF(210.0, 297.0));
    QVERIFY(std::abs(background.aspectRatio() - (210.0 / 297.0)) < 0.0001);

    QCOMPARE(background.canvasColor(), QColor(QStringLiteral("#F1F3F6")));
    QCOMPARE(background.paperBorderColor(), QColor(QStringLiteral("#19000000")));
    QCOMPARE(background.paperColor(), QColor(QStringLiteral("#FFFCF5")));
    QCOMPARE(background.paperHighlightColor(), QColor(QStringLiteral("#FFFDF9")));
    QCOMPARE(background.paperShadeColor(), QColor(QStringLiteral("#F6EEE0")));
    QCOMPARE(background.paperSeparatorColor(), QColor(QStringLiteral("#24000000")));
    QCOMPARE(background.paperShadowColor(), QColor(QStringLiteral("#14000000")));
    QCOMPARE(background.paperTextColor(), QColor(QStringLiteral("#000000")));

    layoutRenderer.setPaperAspectRatio(0.0);
    QCOMPARE(layoutRenderer.paperAspectRatio(), background.aspectRatio());
    QCOMPARE(layoutRenderer.canvasColor(), background.canvasColor());
    QCOMPARE(layoutRenderer.paperBorderColor(), background.paperBorderColor());
    QCOMPARE(layoutRenderer.paperColor(), background.paperColor());
    QCOMPARE(layoutRenderer.paperHighlightColor(), background.paperHighlightColor());
    QCOMPARE(layoutRenderer.paperShadeColor(), background.paperShadeColor());
    QCOMPARE(layoutRenderer.paperSeparatorColor(), background.paperSeparatorColor());
    QCOMPARE(layoutRenderer.paperShadowColor(), background.paperShadowColor());
    QCOMPARE(layoutRenderer.paperTextColor(), background.paperTextColor());
}

void WhatSonCppRegressionTests::displayPaperModels_hostPageAndPrintViewModeObjectsUnderModelsDirectory()
{
    const QString a4PaperBackgroundHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsA4PaperBackground.hpp"));
    const QString a4PaperBackgroundImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsA4PaperBackground.cpp"));
    const QString paperSelectionHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsPaperSelection.hpp"));
    const QString paperSelectionImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsPaperSelection.cpp"));
    const QString paperRendererHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsTextFormatRenderer.hpp"));
    const QString paperRendererImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/ContentsTextFormatRenderer.cpp"));
    const QString printLayoutHeaderSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"));
    const QString printLayoutImplSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/ContentsPagePrintLayoutRenderer.cpp"));
    const QString appCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/CMakeLists.txt"));
    const QString displayCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/CMakeLists.txt"));
    const QString paperCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/CMakeLists.txt"));
    const QString printCmakeSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/display/paper/print/CMakeLists.txt"));
    const QString registrarSource = readUtf8SourceFile(
        QStringLiteral("src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp"));

    QVERIFY(!a4PaperBackgroundHeaderSource.isEmpty());
    QVERIFY(!a4PaperBackgroundImplSource.isEmpty());
    QVERIFY(!paperSelectionHeaderSource.isEmpty());
    QVERIFY(!paperSelectionImplSource.isEmpty());
    QVERIFY(!paperRendererHeaderSource.isEmpty());
    QVERIFY(!paperRendererImplSource.isEmpty());
    QVERIFY(!printLayoutHeaderSource.isEmpty());
    QVERIFY(!printLayoutImplSource.isEmpty());
    QVERIFY(readUtf8SourceFile(QStringLiteral("src/app/editor/renderer/ContentsTextFormatRenderer.hpp")).isEmpty());
    QVERIFY(readUtf8SourceFile(QStringLiteral("src/app/editor/renderer/ContentsPagePrintLayoutRenderer.hpp")).isEmpty());

    QVERIFY(appCmakeSource.contains(QStringLiteral("add_subdirectory(models/display)")));
    QVERIFY(displayCmakeSource.contains(QStringLiteral("add_subdirectory(paper)")));
    QVERIFY(paperCmakeSource.contains(QStringLiteral("add_subdirectory(print)")));
    QVERIFY(paperCmakeSource.contains(QStringLiteral("whatson_app_register_directory_sources")));
    QVERIFY(printCmakeSource.contains(QStringLiteral("whatson_app_register_directory_sources")));

    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"display/paper/ContentsPaperSelection.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"display/paper/ContentsA4PaperBackground.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("qmlRegisterType<ContentsPaperSelection>(")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"display/paper/ContentsTextFormatRenderer.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("#include \"display/paper/print/ContentsPagePrintLayoutRenderer.hpp\"")));
    QVERIFY(registrarSource.contains(
        QStringLiteral("qmlRegisterType<ContentsA4PaperBackground>(")));
}

void WhatSonCppRegressionTests::noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks()
{
    const QString sourceText =
        QStringLiteral("브랜드 사이트 <weblink href=\"www.iisacc.com\">아이작닷컴</weblink>");
    const QString bodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), sourceText);

    QVERIFY(bodyDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">아이작닷컴</weblink>")));
    QCOMPARE(WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocument), sourceText);

    const QString htmlProjection =
        WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(bodyDocument);
    QVERIFY(htmlProjection.contains(
        QStringLiteral("<a href=\"https://www.iisacc.com\" style=\"color:#8CB4FF;text-decoration: underline;\">")));
    QVERIFY(htmlProjection.contains(QStringLiteral("아이작닷컴</a>")));

    const QString autoWrappedDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
        QStringLiteral("note"),
        QStringLiteral("Visit www.iisacc.com"));
    QVERIFY(autoWrappedDocument.contains(
        QStringLiteral("<weblink href=\"www.iisacc.com\">www.iisacc.com</weblink>")));
}

void WhatSonCppRegressionTests::logicalTextBridge_advancesCursorPastClosingWebLinkTag()
{
    ContentsLogicalTextBridge bridge;
    const QString sourceText =
        QStringLiteral("<weblink href=\"www.iisacc.com\">site</weblink>!");
    bridge.setText(sourceText);

    QCOMPARE(bridge.logicalText(), QStringLiteral("site!"));
    QCOMPARE(bridge.logicalLengthForSourceText(sourceText), QStringLiteral("site!").size());
    QCOMPARE(bridge.sourceOffsetForLogicalOffset(QStringLiteral("site").size()), sourceText.indexOf(QLatin1Char('!')));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaLayer.qml"));
    const QString calloutLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutLayer.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaLayerSource.isEmpty());
    QVERIFY(!calloutLayerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("paperPaletteEnabled: documentFlow.paperPaletteEnabled")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("paperPaletteEnabled: documentBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("readonly property color editorTextColor: paperPaletteEnabled ? \"#111111\" : LV.Theme.bodyColor")));
    QVERIFY(textBlockSource.contains(QStringLiteral("paperPaletteEnabled: textBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textColor: textBlock.editorTextColor")));

    QVERIFY(!agendaBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("color: \"#80FFFFFF\"")));
    QVERIFY(agendaBlockSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutBlockSource.contains(
        QStringLiteral("readonly property color bodyTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaLayerSource.contains(QStringLiteral("color: \"#FFFFFF\"")));
    QVERIFY(agendaLayerSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutLayerSource.contains(
        QStringLiteral("readonly property color textColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsView.showPrintEditorLayout")));
}

void WhatSonCppRegressionTests::qmlEditors_routeRenderedHyperlinksToExternalBrowser()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onLinkActivated: function (link)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.openUrlExternally(link);")));
    QVERIFY(displayViewSource.contains(QStringLiteral("onLinkActivated: function (link)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("Qt.openUrlExternally(link);")));
}

void WhatSonCppRegressionTests::resourceBitmapViewer_projectsRenderableImagePreviewState()
{
    ResourceBitmapViewer viewer;

    viewer.setResourceEntry(QVariantMap{
        {QStringLiteral("displayName"), QStringLiteral("Cover.PNG")},
        {QStringLiteral("renderMode"), QStringLiteral("image")},
        {QStringLiteral("format"), QStringLiteral(".PNG")},
        {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/WhatSon/Cover.PNG")}
    });

    QCOMPARE(viewer.normalizedFormat(), QStringLiteral(".png"));
    QVERIFY(viewer.bitmapPreviewCandidate());
    QVERIFY(viewer.bitmapFormatCompatible());
    QVERIFY(viewer.bitmapRenderable());
    QCOMPARE(
        viewer.openTarget(),
        QUrl::fromLocalFile(QStringLiteral("/tmp/WhatSon/Cover.PNG")).toString());
    QCOMPARE(viewer.viewerSource(), viewer.openTarget());
    QVERIFY(viewer.incompatibilityReason().isEmpty());

    viewer.setResourceEntry(QVariantMap{
        {QStringLiteral("displayName"), QStringLiteral("Source.psd")},
        {QStringLiteral("format"), QStringLiteral(".psd")},
        {QStringLiteral("resolvedPath"), QStringLiteral("/tmp/WhatSon/Source.psd")}
    });

    QVERIFY(viewer.bitmapPreviewCandidate());
    QVERIFY(!viewer.bitmapFormatCompatible());
    QVERIFY(!viewer.bitmapRenderable());
    QVERIFY(!viewer.incompatibilityReason().trimmed().isEmpty());
}

void WhatSonCppRegressionTests::editorSessionController_preservesLocalEditorAuthorityAgainstSameNoteModelSync()
{
    ContentsEditorSessionController controller;

    controller.setEditorBoundNoteId(QStringLiteral("note-1"));
    controller.setEditorText(QStringLiteral("editor-owned text"));
    controller.setLocalEditorAuthority(true);
    controller.setLastLocalEditTimestampMs(0);
    controller.setTypingIdleThresholdMs(1000);
    controller.setPendingBodySave(false);

    QVERIFY(!controller.isTypingSessionActive());
    QVERIFY(!controller.requestSyncEditorTextFromSelection(
        QStringLiteral("note-1"),
        QStringLiteral("raw-owned text"),
        QStringLiteral("note-1")));
    QCOMPARE(controller.editorBoundNoteId(), QStringLiteral("note-1"));
    QCOMPARE(controller.editorText(), QStringLiteral("editor-owned text"));
    QVERIFY(controller.localEditorAuthority());

    controller.setLocalEditorAuthority(false);
    QVERIFY(controller.requestSyncEditorTextFromSelection(
        QStringLiteral("note-1"),
        QStringLiteral("raw-owned text"),
        QStringLiteral("note-1")));
    QCOMPARE(controller.editorText(), QStringLiteral("raw-owned text"));
}

void WhatSonCppRegressionTests::noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("authoritative-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    FakeContentPersistenceViewModel contentViewModel;
    contentViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentViewModel(&contentViewModel);

    QSignalSpy reconcileSpy(
        &coordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("editor-after");
    request.preferViewSessionOnMismatch = true;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QVERIFY(result.viewSessionPersisted);
    QVERIFY(result.snapshotRefreshRequested);

    coordinator.handleRequestFinished(result);
    QCOMPARE(reconcileSpy.count(), 1);

    const QList<QVariant> reconcileArguments = reconcileSpy.takeFirst();
    QCOMPARE(reconcileArguments.at(0).toString(), noteId);
    QVERIFY(reconcileArguments.at(1).toBool());
    QVERIFY(reconcileArguments.at(2).toBool());
    QCOMPARE(reconcileArguments.at(3).toString(), QString());

    QCOMPARE(contentViewModel.applyPersistedBodyStateCallCount, 1);
    QCOMPARE(contentViewModel.lastAppliedNoteId, noteId);
    QCOMPARE(contentViewModel.lastAppliedBodyPlainText, QStringLiteral("editor-after"));
    QCOMPARE(contentViewModel.lastAppliedBodySourceText, QStringLiteral("editor-after"));
    QCOMPARE(contentViewModel.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentViewModel.lastReloadedNoteId, noteId);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read reconciled note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("editor-after"));
}

void WhatSonCppRegressionTests::noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteId = QStringLiteral("refresh-only-note");
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        noteId,
        QStringLiteral("raw-before"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    FakeContentPersistenceViewModel contentViewModel;
    contentViewModel.setNoteDirectoryPath(noteId, noteDirectoryPath);

    ContentsNoteManagementCoordinator coordinator;
    coordinator.setContentViewModel(&contentViewModel);

    QSignalSpy reconcileSpy(
        &coordinator,
        &ContentsNoteManagementCoordinator::viewSessionSnapshotReconciled);

    ContentsNoteManagementCoordinator::Request request;
    request.kind = ContentsNoteManagementCoordinator::RequestKind::ReconcileViewSessionSnapshot;
    request.noteId = noteId;
    request.noteDirectoryPath = noteDirectoryPath;
    request.text = QStringLiteral("editor-after");
    request.preferViewSessionOnMismatch = false;

    const ContentsNoteManagementCoordinator::Result result =
        ContentsNoteManagementCoordinator::performWorkerRequest(request);
    QVERIFY(result.success);
    QVERIFY(!result.viewSessionPersisted);
    QVERIFY(result.snapshotRefreshRequested);

    coordinator.handleRequestFinished(result);
    QCOMPARE(reconcileSpy.count(), 1);

    const QList<QVariant> reconcileArguments = reconcileSpy.takeFirst();
    QCOMPARE(reconcileArguments.at(0).toString(), noteId);
    QVERIFY(reconcileArguments.at(1).toBool());
    QVERIFY(reconcileArguments.at(2).toBool());
    QCOMPARE(reconcileArguments.at(3).toString(), QString());

    QCOMPARE(contentViewModel.applyPersistedBodyStateCallCount, 0);
    QCOMPARE(contentViewModel.reloadNoteMetadataCallCount, 1);
    QCOMPARE(contentViewModel.lastReloadedNoteId, noteId);

    WhatSonLocalNoteFileStore fileStore;
    WhatSonLocalNoteDocument document;
    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = noteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;

    QString readError;
    QVERIFY2(
        fileStore.readNote(readRequest, &document, &readError),
        qPrintable(QStringLiteral("Failed to read refreshed note: %1").arg(readError)));
    QCOMPARE(
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.bodySourceText),
        QStringLiteral("raw-before"));
}

void WhatSonCppRegressionTests::noteFileStatSupport_incrementsOpenCountAndPersistsLastOpenedAt()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    QString createError;
    const QString noteDirectoryPath = createLocalNoteForRegression(
        workspaceDir.path(),
        QStringLiteral("selection-note"),
        QStringLiteral("selection body"),
        &createError);
    QVERIFY2(
        !noteDirectoryPath.isEmpty(),
        qPrintable(QStringLiteral("Failed to create note fixture: %1").arg(createError)));

    const QString headerPath = WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath);
    QVERIFY(QFileInfo::exists(headerPath));

    const QDateTime beforeIncrementUtc = QDateTime::currentDateTimeUtc();
    QString incrementError;
    QVERIFY2(
        WhatSon::NoteFileStatSupport::incrementOpenCountForNoteHeader(
            QStringLiteral("selection-note"),
            noteDirectoryPath,
            &incrementError),
        qPrintable(incrementError));

    QFile headerFile(headerPath);
    QVERIFY(headerFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString headerText = QString::fromUtf8(headerFile.readAll());
    headerFile.close();

    QVERIFY(headerText.contains(QStringLiteral("<lastOpened>")));

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser parser;
    QString parseError;
    QVERIFY2(parser.parse(headerText, &headerStore, &parseError), qPrintable(parseError));

    QCOMPARE(headerStore.openCount(), 1);
    QCOMPARE(headerStore.lastModifiedAt(), QStringLiteral("2026-04-18-00-00-00"));
    QVERIFY(!headerStore.lastOpenedAt().isEmpty());

    const QDateTime lastOpenedUtc = QDateTime::fromString(headerStore.lastOpenedAt(), Qt::ISODate).toUTC();
    QVERIFY(lastOpenedUtc.isValid());
    QVERIFY(lastOpenedUtc >= beforeIncrementUtc.addSecs(-1));
    QVERIFY(lastOpenedUtc <= QDateTime::currentDateTimeUtc().addSecs(1));
}

void WhatSonCppRegressionTests::unusedNoteSensors_filterNoteIdsByLastOpenedWindow()
{
    QTemporaryDir workspaceDir;
    QVERIFY(workspaceDir.isValid());

    const QString hubPath = QDir(workspaceDir.path()).filePath(QStringLiteral("Workspace.wshub"));
    const QString contentsDirectoryPath = QDir(hubPath).filePath(QStringLiteral(".wscontents"));
    QVERIFY(QDir().mkpath(contentsDirectoryPath));

    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    const auto rewriteHeader = [](
                                   const QString& noteDirectoryPath,
                                   const QString& createdAt,
                                   const QString& lastModifiedAt,
                                   const QString& lastOpenedAt,
                                   const int openCount)
    {
        const QString headerPath = WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath);
        QFile headerFile(headerPath);
        if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }

        const QString headerText = QString::fromUtf8(headerFile.readAll());
        headerFile.close();

        WhatSonNoteHeaderStore headerStore;
        WhatSonNoteHeaderParser parser;
        QString parseError;
        if (!parser.parse(headerText, &headerStore, &parseError))
        {
            return false;
        }

        headerStore.setCreatedAt(createdAt);
        headerStore.setLastModifiedAt(lastModifiedAt);
        headerStore.setLastOpenedAt(lastOpenedAt);
        headerStore.setOpenCount(openCount);

        WhatSonNoteHeaderCreator creator(noteDirectoryPath, QString());
        QFile writeFile(headerPath);
        if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            return false;
        }

        const QByteArray encoded = creator.createHeaderText(headerStore).toUtf8();
        const bool success = writeFile.write(encoded) == encoded.size();
        writeFile.close();
        return success;
    };

    QString createError;
    const QString monthlyOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("monthly-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!monthlyOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        monthlyOldNoteDirectoryPath,
        nowUtc.addDays(-90).toString(Qt::ISODate),
        nowUtc.addDays(-60).toString(Qt::ISODate),
        nowUtc.addDays(-40).toString(Qt::ISODate),
        4));

    createError.clear();
    const QString weeklyOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("weekly-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!weeklyOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        weeklyOldNoteDirectoryPath,
        nowUtc.addDays(-20).toString(Qt::ISODate),
        nowUtc.addDays(-10).toString(Qt::ISODate),
        nowUtc.addDays(-8).toString(Qt::ISODate),
        2));

    createError.clear();
    const QString recentNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("recent-note"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!recentNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        recentNoteDirectoryPath,
        nowUtc.addDays(-6).toString(Qt::ISODate),
        nowUtc.addDays(-4).toString(Qt::ISODate),
        nowUtc.addDays(-2).toString(Qt::ISODate),
        7));

    createError.clear();
    const QString neverOpenedOldNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("never-opened-old"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!neverOpenedOldNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        neverOpenedOldNoteDirectoryPath,
        nowUtc.addDays(-50).toString(Qt::ISODate),
        nowUtc.addDays(-45).toString(Qt::ISODate),
        QString(),
        0));

    createError.clear();
    const QString neverOpenedRecentNoteDirectoryPath = createLocalNoteForRegression(
        contentsDirectoryPath,
        QStringLiteral("never-opened-recent"),
        QStringLiteral("body"),
        &createError);
    QVERIFY2(!neverOpenedRecentNoteDirectoryPath.isEmpty(), qPrintable(createError));
    QVERIFY(rewriteHeader(
        neverOpenedRecentNoteDirectoryPath,
        nowUtc.addDays(-3).toString(Qt::ISODate),
        nowUtc.addDays(-2).toString(Qt::ISODate),
        QString(),
        0));

    WeeklyUnusedNote weeklySensor;
    MonthlyUnusedNote monthlySensor;
    QSignalSpy weeklyChangedSpy(&weeklySensor, &WeeklyUnusedNote::unusedNotesChanged);
    QSignalSpy monthlyChangedSpy(&monthlySensor, &MonthlyUnusedNote::unusedNotesChanged);

    weeklySensor.setHubPath(hubPath);
    monthlySensor.setHubPath(hubPath);

    QCOMPARE(weeklySensor.lastError(), QString());
    QCOMPARE(monthlySensor.lastError(), QString());
    const QStringList expectedWeeklyNoteIds{
        QStringLiteral("monthly-old"),
        QStringLiteral("never-opened-old"),
        QStringLiteral("weekly-old")
    };
    const QStringList expectedMonthlyNoteIds{
        QStringLiteral("monthly-old"),
        QStringLiteral("never-opened-old")
    };
    QCOMPARE(weeklySensor.unusedNoteIds(), expectedWeeklyNoteIds);
    QCOMPARE(monthlySensor.unusedNoteIds(), expectedMonthlyNoteIds);
    QCOMPARE(weeklySensor.unusedNoteCount(), 3);
    QCOMPARE(monthlySensor.unusedNoteCount(), 2);
    QCOMPARE(weeklyChangedSpy.count(), 1);
    QCOMPARE(monthlyChangedSpy.count(), 1);

    const QVariantMap weeklyNeverOpenedEntry = weeklySensor.unusedNotes().at(1).toMap();
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("noteId")).toString(), QStringLiteral("never-opened-old"));
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("activitySource")).toString(), QStringLiteral("createdAt"));
    QCOMPARE(weeklyNeverOpenedEntry.value(QStringLiteral("lastOpenedAt")).toString(), QString());
}

QTEST_APPLESS_MAIN(WhatSonCppRegressionTests)

#include "whatson_cpp_regression_tests.moc"
