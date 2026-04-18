#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/note/ContentsNoteManagementCoordinator.hpp"
#include "file/note/WhatSonLocalNoteFileStore.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"
#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "runtime/scheduler/WhatSonCronExpression.hpp"
#include "runtime/scheduler/WhatSonUnixTimeAnalyzer.hpp"
#include "store/hub/SelectedHubStore.hpp"
#include "store/sidebar/ISidebarSelectionStore.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"
#include "viewmodel/content/ContentsEditorSessionController.hpp"
#include "viewmodel/content/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "viewmodel/content/ContentsStructuredDocumentMutationPolicy.hpp"
#include "viewmodel/content/ContentsResourceTagTextGenerator.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewModeViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewSectionViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewState.hpp"
#include "viewmodel/navigationbar/NavigationModeSectionViewModel.hpp"
#include "viewmodel/navigationbar/NavigationModeState.hpp"
#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"
#include "viewmodel/onboarding/IOnboardingHubController.hpp"
#include "viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/IActiveHierarchyContextSource.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

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
    void navigationModeViewModel_cyclesActiveSections();
    void editorViewModeViewModel_cyclesActiveSections();
    void onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle();
    void unixTimeAnalyzer_reportsStableEpochFields();
    void cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication();
    void resourceTagTextGenerator_and_noteFolderSemantics_normalizeDescriptorsAndXml();
    void structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches();
    void structuredMutationPolicy_buildsDeletionAndInsertionPayloads();
    void editorSessionController_preservesLocalEditorAuthorityAgainstSameNoteModelSync();
    void noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred();
    void noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative();

private:
    static QString createLocalNoteForRegression(
        const QString& parentDirectoryPath,
        const QString& noteId,
        const QString& bodySourceText,
        QString* errorMessage = nullptr);
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
        WhatSon::NoteFolders::leafFolderName(QStringLiteral(" /Research//Today/ ")),
        QStringLiteral("Today"));
    QVERIFY(WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/today")));
    QVERIFY(!WhatSon::NoteFolders::usesReservedTodayFolderSegment(QStringLiteral("Research/Tomorrow")));

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

    QVERIFY(coordinator.reconcileViewSessionAndRefreshSnapshotForNote(
        noteId,
        QStringLiteral("editor-after"),
        true));
    QTRY_COMPARE_WITH_TIMEOUT(reconcileSpy.count(), 1, 10000);

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

    QVERIFY(coordinator.reconcileViewSessionAndRefreshSnapshotForNote(
        noteId,
        QStringLiteral("editor-after"),
        false));
    QTRY_COMPARE_WITH_TIMEOUT(reconcileSpy.count(), 1, 10000);

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

QTEST_APPLESS_MAIN(WhatSonCppRegressionTests)

#include "whatson_cpp_regression_tests.moc"
