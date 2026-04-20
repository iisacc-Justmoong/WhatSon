#pragma once

#include "display/paper/ContentsA4PaperBackground.hpp"
#include "display/paper/ContentsPaperSelection.hpp"
#include "display/paper/ContentsTextFormatRenderer.hpp"
#include "display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "file/hub/WhatSonHubMountValidator.hpp"
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
#include "runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "runtime/startup/WhatSonStartupHubResolver.hpp"
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
#include <QProcess>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
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

    Q_INVOKABLE QString noteBodySourceTextForNoteId(const QString& noteId) const
    {
        return m_noteBodySourceTexts.value(noteId.trimmed());
    }

    void setNoteDirectoryPath(const QString& noteId, const QString& noteDirectoryPath)
    {
        m_noteDirectoryPaths.insert(noteId.trimmed(), QDir::cleanPath(noteDirectoryPath.trimmed()));
    }

    void setNoteBodySourceText(const QString& noteId, const QString& bodySourceText)
    {
        m_noteBodySourceTexts.insert(noteId.trimmed(), bodySourceText);
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
    QHash<QString, QString> m_noteBodySourceTexts;
};

class FakeSelectionNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText WRITE setCurrentBodyText NOTIFY currentBodyTextChanged)
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

    QString currentBodyText() const
    {
        return m_currentBodyText;
    }

    void setCurrentBodyText(QString bodyText)
    {
        if (m_currentBodyText == bodyText)
        {
            return;
        }

        m_currentBodyText = std::move(bodyText);
        emit currentBodyTextChanged();
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
    void currentBodyTextChanged();
    void searchTextChanged();
    void itemCountChanged(int itemCount);
    void noteBackedChanged();

private:
    int m_currentIndex = -1;
    QString m_currentNoteId;
    QString m_currentBodyText;
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

class FakeSelectedHubStore final : public ISelectedHubStore
{
public:
    [[nodiscard]] QString selectedHubPath() override
    {
        return m_selectedHubPath;
    }

    [[nodiscard]] QByteArray selectedHubAccessBookmark() override
    {
        return m_selectedHubAccessBookmark;
    }

    void clearSelectedHubPath() override
    {
        m_selectedHubPath.clear();
        m_selectedHubAccessBookmark.clear();
    }

    void setSelectedHubPath(const QString& hubPath) override
    {
        m_selectedHubPath = hubPath;
        m_selectedHubAccessBookmark.clear();
    }

    void setSelectedHubSelection(const QString& hubPath, const QByteArray& accessBookmark) override
    {
        m_selectedHubPath = hubPath;
        m_selectedHubAccessBookmark = accessBookmark;
    }

private:
    QString m_selectedHubPath;
    QByteArray m_selectedHubAccessBookmark;
};

class WhatSonCppRegressionTests final : public QObject
{
    Q_OBJECT

private slots:
    void selectedHubStore_persistsNormalizedSelectionsWithinSandboxedSettings();
    void hubMountValidator_acceptsCompleteHubPackage();
    void hubMountValidator_rejectsIncompleteHubPackage();
    void startupHubResolver_returnsEmptyWithoutPersistedSelection();
    void startupHubResolver_mountsPersistedCompleteHubPackage();
    void startupHubResolver_keepsPersistedFailureVisibleWithoutSwitchingToBlueprint();
    void iosBundleIconPackaging_declaresPrimaryAndIpadFallbackIconsInInfoPlist();
    void iosBundleIconPackaging_stagesBundleRootPngsEvenWithAssetCatalogsEnabled();
    void iosXcodeprojExport_surfacesSdkSigningAndPermissionPolicyOptionsInCmake();
    void iosXcodeprojExport_routesSimulatorPermissionFallbackThroughAppRuntimeCmake();
    void iosXcodeprojExport_patchScriptStripsQtPermissionsEvenWhenIconPhaseAlreadyExists();
    void iosXcodeprojExport_keepsBuildIosScriptOnHighLevelCmakeOptions();
    void appLaunchSupport_requiresMountedAndLoadedHubForStartupWorkspace();
    void sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals();
    void hierarchyViewModelProvider_normalizesMappingsAndAvoidsDuplicateSignals();
    void sidebarHierarchyViewModel_preservesFallbackAcrossStoreAttachDetach();
    void sidebarHierarchyViewModel_reactsToProviderMappingChanges();
    void sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings();
    void contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromNoteListSnapshot();
    void contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromDirectSourceSnapshot();
    void noteBackedHierarchyNoteLists_preserveRawBodySnapshotForEditorBootstrap();
    void noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately();
    void noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches();
    void navigationModeViewModel_cyclesActiveSections();
    void editorViewModeViewModel_cyclesActiveSections();
    void onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle();
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
    void contentsDisplayView_reservesLargeBottomAccessibilityMargin();
    void contentsDisplayView_usesSelectedNoteSnapshotWhileSessionBindingCatchesUp();
    void qmlInlineFormatEditor_keepsHiddenKeyboardTouchesScrollFirstOnMobile();
    void mobileChrome_usesSharedFigmaControlSurfaceColor();
    void paperSelection_tracksChosenPaperEnumState();
    void a4PaperBackground_exposesCanonicalMetricsAndAnchorsPrintRendererDefaults();
    void textFormatRenderer_wrapsCommittedUrlsIntoCanonicalWebLinks();
    void textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml();
    void displayPaperModels_hostPageAndPrintViewModeObjectsUnderModelsDirectory();
    void noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks();
    void noteBodyPersistence_stripsRenderedHtmlBlockArtifactsFromSourceProjection();
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
    static QString createMinimalHubFixture(
        const QString& workspaceRootPath,
        const QString& hubDirectoryName,
        QString* errorMessage = nullptr);
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
