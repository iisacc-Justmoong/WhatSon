#pragma once

#include "app/models/display/paper/ContentsA4PaperBackground.hpp"
#include "app/models/display/paper/ContentsPaperSelection.hpp"
#include "app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp"
#include "app/models/editor/format/ContentsPlainTextSourceMutator.hpp"
#include "app/models/editor/format/ContentsTextFormatRenderer.hpp"
#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"
#include "app/models/editor/minimap/ContentsMinimapLayoutMetrics.hpp"
#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"
#include "app/models/display/paper/print/ContentsPagePrintLayoutRenderer.hpp"
#include "app/models/file/hub/WhatSonHubMountValidator.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "app/models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "app/models/file/import/WhatSonClipboardResourceImportFileNamePolicy.hpp"
#include "app/models/file/viewer/ResourceBitmapViewer.hpp"
#define private public
#include "app/models/file/note/ContentsNoteManagementCoordinator.hpp"
#undef private
#include "app/models/file/note/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/WhatSonNoteHeaderCreator.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/note/WhatSonNoteHeaderParser.hpp"
#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"
#include "app/models/file/statistic/WhatSonNoteFileStatSupport.hpp"
#include "app/runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "app/runtime/startup/WhatSonStartupHubResolver.hpp"
#include "app/runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "app/runtime/scheduler/WhatSonCronExpression.hpp"
#include "app/runtime/scheduler/WhatSonUnixTimeAnalyzer.hpp"
#include "app/models/sensor/MonthlyUnusedNote.hpp"
#include "app/models/sensor/UnusedResourcesSensor.hpp"
#include "app/models/sensor/WeeklyUnusedNote.hpp"
#include "app/store/hub/SelectedHubStore.hpp"
#include "app/store/sidebar/ISidebarSelectionStore.hpp"
#include "app/store/sidebar/SidebarSelectionStore.hpp"
#include "app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"
#include "app/models/editor/persistence/ContentsEditorPersistenceController.hpp"
#include "app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp"
#include "app/models/editor/session/ContentsEditorSessionController.hpp"
#include "app/models/editor/tags/ContentsAgendaBackend.hpp"
#include "app/models/editor/tags/ContentsCalloutBackend.hpp"
#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentBlocksModel.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentHost.hpp"
#include "app/models/editor/structure/ContentsStructuredDocumentMutationPolicy.hpp"
#include "app/models/editor/tags/ContentsResourceTagTextGenerator.hpp"
#include "app/models/file/hierarchy/IHierarchyController.hpp"
#include "app/models/file/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "app/models/file/hierarchy/library/LibraryNoteListModel.hpp"
#include "app/models/file/hierarchy/progress/ProgressHierarchyControllerSupport.hpp"
#include "app/models/file/hierarchy/resources/ResourcesHierarchyController.hpp"
#include "app/models/navigationbar/EditorViewModeController.hpp"
#include "app/models/navigationbar/EditorViewSectionController.hpp"
#include "app/models/navigationbar/EditorViewState.hpp"
#include "app/models/navigationbar/NavigationModeSectionController.hpp"
#include "app/models/navigationbar/NavigationModeState.hpp"
#include "app/models/navigationbar/NavigationModeController.hpp"
#include "app/models/onboarding/IOnboardingHubController.hpp"
#include "app/models/onboarding/OnboardingRouteBootstrapController.hpp"
#include "app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.hpp"
#include "app/models/detailPanel/DetailCurrentNoteContextBridge.hpp"
#include "app/models/detailPanel/ResourceDetailPanelController.hpp"
#include "app/models/file/hierarchy/resources/ResourcesListModel.hpp"
#include "app/models/panel/NoteListModelContractBridge.hpp"
#include "app/models/panel/NoteActiveStateTracker.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/sidebar/HierarchyControllerProvider.hpp"
#include "app/models/sidebar/IActiveHierarchyContextSource.hpp"
#include "app/models/sidebar/SidebarHierarchyController.hpp"

#include <QAbstractListModel>
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
#include <QVariantMap>
#include <QtTest>

#include <cmath>
#include <memory>
#include <vector>

class FakeHierarchyController final : public IHierarchyController
{
    Q_OBJECT

public:
    explicit FakeHierarchyController(const QString& labelPrefix, QObject* parent = nullptr)
        : IHierarchyController(parent)
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

class FakeContentPersistenceController final : public QObject
{
    Q_OBJECT

public:
    explicit FakeContentPersistenceController(QObject* parent = nullptr)
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

class FakeMobileSidebarBindingSource final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        int resolvedActiveHierarchyIndex READ resolvedActiveHierarchyIndex WRITE setResolvedActiveHierarchyIndex NOTIFY
            resolvedActiveHierarchyIndexChanged)
    Q_PROPERTY(
        QVariant resolvedHierarchyController READ resolvedHierarchyController WRITE setResolvedHierarchyController NOTIFY
            resolvedHierarchyControllerChanged)

public:
    explicit FakeMobileSidebarBindingSource(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    int resolvedActiveHierarchyIndex() const noexcept
    {
        return m_resolvedActiveHierarchyIndex;
    }

    void setResolvedActiveHierarchyIndex(const int value)
    {
        if (m_resolvedActiveHierarchyIndex == value)
        {
            return;
        }

        m_resolvedActiveHierarchyIndex = value;
        emit resolvedActiveHierarchyIndexChanged();
    }

    QVariant resolvedHierarchyController() const
    {
        return m_resolvedHierarchyController;
    }

    void setResolvedHierarchyController(const QVariant& value)
    {
        if (m_resolvedHierarchyController == value)
        {
            return;
        }

        m_resolvedHierarchyController = value;
        emit resolvedHierarchyControllerChanged();
    }

    void setHierarchyControllerForIndex(const int index, QObject* controller)
    {
        m_indexedHierarchyControllers.insert(index, QVariant::fromValue(controller));
    }

    Q_INVOKABLE QVariant hierarchyControllerForIndex(const QVariant& hierarchyIndex) const
    {
        bool ok = false;
        const int resolvedIndex = hierarchyIndex.toInt(&ok);
        if (!ok)
        {
            return {};
        }

        return m_indexedHierarchyControllers.value(resolvedIndex);
    }

signals:
    void resolvedActiveHierarchyIndexChanged();
    void resolvedHierarchyControllerChanged();

private:
    int m_resolvedActiveHierarchyIndex = 0;
    QVariant m_resolvedHierarchyController;
    QHash<int, QVariant> m_indexedHierarchyControllers;
};

class FakeSelectionNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(
        QString
            currentNoteDirectoryPath READ currentNoteDirectoryPath WRITE setCurrentNoteDirectoryPath
                NOTIFY currentNoteDirectoryPathChanged)
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

    QString currentNoteDirectoryPath() const
    {
        return m_currentNoteDirectoryPath;
    }

    void setCurrentNoteDirectoryPath(QString noteDirectoryPath)
    {
        noteDirectoryPath = QDir::cleanPath(noteDirectoryPath.trimmed());
        if (noteDirectoryPath == QStringLiteral("."))
        {
            noteDirectoryPath.clear();
        }
        if (m_currentNoteDirectoryPath == noteDirectoryPath)
        {
            return;
        }

        m_currentNoteDirectoryPath = std::move(noteDirectoryPath);
        emit currentNoteDirectoryPathChanged();
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
    void currentNoteDirectoryPathChanged();
    void currentBodyTextChanged();
    void searchTextChanged();
    void itemCountChanged(int itemCount);
    void noteBackedChanged();

private:
    int m_currentIndex = -1;
    QString m_currentNoteId;
    QString m_currentNoteDirectoryPath;
    QString m_currentBodyText;
    QString m_searchText;
    int m_itemCount = 0;
    bool m_noteBacked = true;
};

class FakeIndexDrivenSelectionNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentNoteDirectoryPath READ currentNoteDirectoryPath NOTIFY currentNoteDirectoryPathChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText NOTIFY currentBodyTextChanged)
    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool noteBacked READ noteBacked WRITE setNoteBacked NOTIFY noteBackedChanged)

public:
    struct Entry
    {
        QString noteId;
        QString noteDirectoryPath;
        QString bodyText;
    };

    explicit FakeIndexDrivenSelectionNoteListModel(QObject* parent = nullptr)
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
        return m_entries.value(m_currentIndex).noteId;
    }

    QString currentNoteDirectoryPath() const
    {
        return m_entries.value(m_currentIndex).noteDirectoryPath;
    }

    QString currentBodyText() const
    {
        return m_entries.value(m_currentIndex).bodyText;
    }

    void setEntry(const int index, QString noteId, QString bodyText, QString noteDirectoryPath = QString())
    {
        const int normalizedIndex = std::max(0, index);
        Entry entry;
        entry.noteId = noteId.trimmed();
        entry.noteDirectoryPath = QDir::cleanPath(noteDirectoryPath.trimmed());
        if (entry.noteDirectoryPath == QStringLiteral("."))
        {
            entry.noteDirectoryPath.clear();
        }
        entry.bodyText = std::move(bodyText);
        m_entries.insert(normalizedIndex, std::move(entry));
        if (normalizedIndex + 1 > m_itemCount)
        {
            setItemCount(normalizedIndex + 1);
        }
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
    void currentNoteDirectoryPathChanged();
    void currentBodyTextChanged();
    void itemCountChanged(int itemCount);
    void noteBackedChanged();

private:
    QHash<int, Entry> m_entries;
    int m_currentIndex = -1;
    int m_itemCount = 0;
    bool m_noteBacked = true;
};

class FakeCurrentNoteEntryOnlyListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap currentNoteEntry READ currentNoteEntry WRITE setCurrentNoteEntry NOTIFY currentNoteEntryChanged)
    Q_PROPERTY(bool noteBacked READ noteBacked WRITE setNoteBacked NOTIFY noteBackedChanged)

public:
    explicit FakeCurrentNoteEntryOnlyListModel(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    QVariantMap currentNoteEntry() const
    {
        return m_currentNoteEntry;
    }

    void setCurrentNoteEntry(QVariantMap currentNoteEntry)
    {
        if (m_currentNoteEntry == currentNoteEntry)
        {
            return;
        }

        m_currentNoteEntry = std::move(currentNoteEntry);
        emit currentNoteEntryChanged();
    }

    bool noteBacked() const noexcept
    {
        return m_noteBacked;
    }

    void setNoteBacked(const bool noteBacked)
    {
        if (m_noteBacked == noteBacked)
        {
            return;
        }

        m_noteBacked = noteBacked;
        emit noteBackedChanged();
    }

signals:
    void currentNoteEntryChanged();
    void noteBackedChanged();

private:
    QVariantMap m_currentNoteEntry;
    bool m_noteBacked = true;
};

class FakeRowOnlySelectionNoteListModel final : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool noteBacked READ noteBacked WRITE setNoteBacked NOTIFY noteBackedChanged)

public:
    struct Entry
    {
        QString noteId;
        QString noteDirectoryPath;
        QString bodyText;
    };

    explicit FakeRowOnlySelectionNoteListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid())
        {
            return 0;
        }
        return static_cast<int>(m_entries.size());
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        {
            return {};
        }

        const Entry& entry = m_entries[static_cast<size_t>(index.row())];
        switch (role)
        {
        case Qt::UserRole + 1:
            return entry.noteId;
        case Qt::UserRole + 3:
            return entry.noteDirectoryPath;
        case Qt::UserRole + 4:
            return entry.bodyText;
        default:
            return {};
        }
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

    int itemCount() const noexcept
    {
        return rowCount();
    }

    bool noteBacked() const noexcept
    {
        return m_noteBacked;
    }

    void setNoteBacked(const bool noteBacked)
    {
        if (m_noteBacked == noteBacked)
        {
            return;
        }

        m_noteBacked = noteBacked;
        emit noteBackedChanged();
    }

    void appendEntry(QString noteId, QString noteDirectoryPath, QString bodyText)
    {
        const int nextRow = rowCount();
        beginInsertRows(QModelIndex(), nextRow, nextRow);
        Entry entry;
        entry.noteId = std::move(noteId);
        entry.noteDirectoryPath = std::move(noteDirectoryPath);
        entry.bodyText = std::move(bodyText);
        m_entries.push_back(std::move(entry));
        endInsertRows();
        emit itemCountChanged(itemCount());
    }

signals:
    void currentIndexChanged();
    void itemCountChanged(int itemCount);
    void noteBackedChanged();

private:
    std::vector<Entry> m_entries;
    int m_currentIndex = -1;
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
    void appLaunchSupport_requiresMountedHubForStartupWorkspace();
    void qmlLaunchSupport_routesRootLoadingThroughLvrsAppEntry();
    void qmlContextBinder_usesLvrsBindPlanForWorkspaceContextObjects();
    void qmlInternalTypeRegistrar_usesLvrsManifestRegistration();
    void foregroundServiceGate_startsSchedulerAndPermissionsAfterVisibleWorkspace();
    void startupRuntimeLoad_usesLvrsAfterFirstIdleLifecycleTask();
    void runtimeParallelLoader_usesLvrsBootstrapParallelForDomainLoads();
    void cmakeDependencyWiring_declaresLocalXmlAndHtmlBlockPackages();
    void debugTraceFilter_suppressesIiXmlDebugSpamByDefault();
    void sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals();
    void hierarchyControllerProvider_normalizesMappingsAndAvoidsDuplicateSignals();
    void architecturePolicyLock_blocksMutableWiringAfterLock();
    void hierarchyControllerProvider_rejectsMappingMutationAfterLock();
    void sidebarHierarchyController_preservesFallbackAcrossStoreAttachDetach();
    void sidebarHierarchyController_rejectsSelectionStoreMutationAfterLock();
    void sidebarHierarchyController_reactsToProviderMappingChanges();
    void noteListModelContractBridge_rejectsWiringMutationAfterLock();
    void noteActiveStateTracker_rejectsHierarchyContextMutationAfterLock();
    void detailCurrentNoteContextBridge_rejectsWiringMutationAfterLock();
    void onboardingRouteBootstrapController_rejectsHubControllerMutationAfterLock();
    void sourceTree_forbidsDeprecatedPresentationLayerVocabulary();
    void sourceTree_keepsMinimapUnderEditorChromeModels();
    void sourceTree_keepsContentsQmlUnderViewContents();
    void sidebarAndSelectionBridge_forceCppOwnershipAcrossHierarchySwitchBindings();
    void contentsEditorSelectionBridge_tracksSelectionFromCurrentIndexSignal();
    void contentsEditorSelectionBridge_preservesNoSelectionSentinelBeforeIndexCommit();
    void contentsEditorSelectionBridge_requiresCommittedSelectionContractForNoteIdentity();
    void contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromNoteListSnapshot();
    void contentsEditorSelectionBridge_prefillsSelectedNoteBodyFromDirectSourceSnapshot();
    void contentsEditorSelectionBridge_treatsDirectEmptySourceAsResolvedEmptyNote();
    void contentsEditorSelectionBridge_ignoresNoteListBodySnapshotWithoutDirectSource();
    void contentsEditorSelectionBridge_rebindsSameNoteIdWhenPackagePathChanges();
    void contentsEditorSelectionBridge_clearsSelectedNoteAcrossTransientEmptyCurrentNoteId();
    void contentsEditorSelectionBridge_reloadsBodyWhenCommittedNoteEntryChangesWithoutNoteIdChange();
    void contentsEditorSelectionBridge_updatesBodySnapshotBeforePersistenceFinishedSignal();
    void contentsEditorSelectionBridge_emitsTraceForNoteSelectionFlow();
    void editorPersistenceController_definesEditorPersistenceBoundary();
    void noteBackedHierarchyNoteLists_preserveRawBodySnapshotForEditorBootstrap();
    void noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately();
    void noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches();
    void noteListModelContractBridge_exposesCurrentNoteEntryFromCurrentSelection();
    void noteActiveStateTracker_tracksCurrentNoteAcrossActiveHierarchyChanges();
    void noteActiveStateTracker_clearsReadableEmptyAndNonNoteBackedSelections();
    void noteActiveStateTracker_syncsAttachedEditorSessionFromActiveNote();
    void libraryNoteListModel_emitsCurrentNoteEntryChangedWhenInitialSelectionMaterializes();
    void libraryNoteListModel_emitsCurrentNoteEntryChangedWhenSelectedRowReplacesCurrentSelection();
    void navigationModeController_cyclesActiveSections();
    void editorViewModeController_cyclesActiveSections();
    void onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle();
    void clipboardImportFileNamePolicy_generatesRandom32CharacterAlphaNumericPngNames();
    void unixTimeAnalyzer_reportsStableEpochFields();
    void cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication();
    void hierarchyTreeItemSupport_clampsNegativeSelectionToFirstVisibleRow();
    void progressHierarchySupport_defaultsFirstVisibleItemToFirstDraft();
    void resourcePackageSupport_roundTripsAnnotationMetadataAndBitmap();
    void resourcePackageSupport_normalizesTerminalFormatForMultiDotAssetNames();
    void resourceRenderer_resolvesIiXmlResourceTagsAndInlineHtmlBlockPlaceholders();
    void unusedResourcesSensor_reportsHubPackagesMissingFromAllNoteEmbeddings();
    void unusedResourcesSensor_refreshesAfterRawBodyEmbedsAResource();
    void resourcesImportController_wiresAnnotationBitmapGenerationIntoPackageCreation();
    void resourceTagTextGenerator_and_noteFolderSemantics_normalizeDescriptorsAndXml();
    void editorTagsBoundary_groupsEditorTagInsertionResponsibilities();
    void contentsCalloutBackend_exitsOnPlainEnterAndSplitsAtCursor();
    void editorTagInsertionController_buildsBodyTagInsertionPayloads();
    void foldersHierarchyParser_escapesLiteralSlashLabelsIntoSingleSegments();
    void foldersHierarchySessionService_preservesEscapedLiteralSlashFolderPaths();
    void sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels();
    void sidebarHierarchyView_bindsInlineHelperDependenciesAtStartup();
    void resourcesHierarchyController_defaultsSelectionToImageAndFiltersList();
    void resourcesHierarchyController_collapsesMultiDotImageFormatsIntoTerminalSuffix();
    void structuredCollectionPolicy_normalizesEntriesAndPrefersResolvedMatches();
    void structuredCollectionPolicy_normalizesQmlJsArrayEntries();
    void structuredCollectionPolicy_flattensImplicitInteractiveTextBlocksIntoSingleGroups();
    void structuredBlockRenderer_publishesSingleNormalizedInteractiveStream();
    void structuredBlockRenderer_reportsAsyncRenderProfileForLargeStructuredDocuments();
    void structuredBlockRenderer_keepsEmptyNotesFocusableWithOneTextGroup();
    void structuredBlockRenderer_keepsTrailingResourceInsertionsEditable();
    void structuredBlockRenderer_keepsResourceOnlyTrailingLineEditable();
    void structuredBlockRenderer_keepsEmptyParagraphBetweenResourcesEditable();
    void structuredMutationPolicy_buildsDeletionAndInsertionPayloads();
    void structuredMutationPolicy_buildsParagraphBoundaryMergeAndSplitPayloads();
    void structuredDocumentBlocksModel_updatesRowsWithoutResettingStableSuffixBlocks();
    void structuredDocumentBlocksModel_removesOnlyChangedMiddleRows();
    void structuredDocumentHost_tracksSelectionClearRevisionAcrossInteractions();
    void editorSurfaceModeSupport_switchesToResourceEditorForResourceListModels();
    void qmlResourceEditorView_staysTransparentAndViewerOnly();
    void resourceDetailPanelController_tracksCurrentResourceSelection();
    void detailCurrentNoteContextBridge_prefersCurrentNoteEntryAndClearsNonNoteBackedSelection();
    void detailCurrentNoteContextBridge_clearsReadableEmptyCurrentNoteEntrySelection();
    void detailPanelRouting_separatesNoteAndResourceViewsAndControllers();
    void qmlContextMenus_treatRightClickAndLongPressAsSymmetricPointerTriggers();
    void qmlHierarchyNoteDrop_keepsDropSurfaceOpenUntilCapabilityRejectsTarget();
    void qmlHierarchyExpansion_preservesUserControlledStateAcrossModelRefreshes();
    void listBarLayout_rendersResolvedNoteListModelByIndex();
    void qmlInlineSelectionHelpers_bindOwnersAfterControllerFileDeletion();
    void qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening();
    void qmlStructuredEditors_refreshesDocumentProjectionOnEditorOpen();
    void qmlStructuredEditors_mountsEditorAndMinimapInDisplayLayout();
    void contentsMinimapLayoutMetrics_resolvesRuntimeVisibilityAndDesignRows();
    void qmlStructuredEditors_rejectStaleSourceRangeMutations();
    void qmlStructuredEditors_preserveNativeMobileInputDuringFocusedEdits();
    void qmlStructuredEditors_commitsPlainTextBlocksDirectlyToRawSource();
    void qmlStructuredEditors_insertsInlineFormatTagsAtCollapsedCursor();
    void qmlStructuredEditors_wrapsSelectedTextIntoRawInlineStyleTags();
    void qmlStructuredEditors_insertStructuredShortcutsThroughRawSourceMutations();
    void qmlStructuredEditors_acceptsPlatformCommandModifierForInlineFormatting();
    void qmlStructuredEditors_routesInlineFormatShortcutThroughDocumentFlow();
    void qmlStructuredEditors_requireCommittedRawMutationForTagCommands();
    void qmlStructuredEditors_bindSessionAndFlushTagMutationsToRawPersistence();
    void qmlStructuredEditors_pressRightClickRequestsContextMenuAndFocusedBodyTagShortcuts();
    void qmlStructuredEditors_mapsBottomMarginToTerminalBodyClick();
    void qmlStructuredEditors_backspaceDeletesPreviousResourceFromEmptyTextBlock();
    void qmlStructuredEditors_deletesEmptyCalloutWithBackspace();
    void qmlStructuredEditors_renderInlineStyleOverlayAtRuntime();
    void qmlEditorInputPolicyAdapter_centralizesNativeInputDecisions();
    void qmlEditorViewDirectory_containsOnlyViewSurfaceFiles();
    void qmlStructuredEditors_lockCustomInputToTagManagementOnly();
    void qmlInlineFormatEditor_keepsNativeTextEditInputUncovered();
    void qmlInlineFormatEditor_keepsKeyboardSelectionAndOsImeNative();
    void qmlInlineFormatEditor_keepsRenderedOverlayDuringNativeSelection();
    void qmlInlineFormatEditor_ignoresEmptyFormattingTagsDuringRenderedSelection();
    void qmlInlineFormatEditor_keepsRenderedOverlayPassiveForNativeEditing();
    void qmlInlineFormatEditor_keepsResourceOverlayPinnedDuringNativeEditing();
    void qmlInlineFormatEditor_projectsVisibleGeometryFromRenderedDisplay();
    void qmlInlineFormatEditor_positionsVisibleProbeFromLogicalDisplayText();
    void qmlInlineFormatEditor_mapsRenderedPointerSelectionToCharacterRawRange();
    void qmlInlineFormatEditor_skipsHiddenInlineTagsDuringNativeCursorMovement();
    void qmlInlineFormatEditor_reportsWrappedVisualLineCountForMinimap();
    void qmlStructuredDocumentFlow_routesBottomBlankClickToBodyEnd();
    void qmlInlineFormatEditor_forwardsInlineFormatShortcutsToTagManagementHook();
    void qmlStructuredDocumentFlow_appliesInlineFormatShortcutToSelectedRawRange();
    void mobileChrome_usesSharedFigmaControlSurfaceColor();
    void mobileHierarchyRouteStateStore_tracksNormalizedSelectionRestoreState();
    void mobileHierarchySelectionCoordinator_prefersExplicitSidebarBindingsAndFallbacks();
    void mobileHierarchyNavigationCoordinator_routesBackAsDismissTargets();
    void sourceTree_usesRepositoryAbsoluteProjectIncludes();
    void paperSelection_tracksChosenPaperEnumState();
    void a4PaperBackground_exposesCanonicalMetricsAndAnchorsPrintRendererDefaults();
    void plainTextSourceMutator_wrapsCommittedUrlsIntoCanonicalWebLinks();
    void inlineStyleOverlayRenderer_republishesHtmlOverlayVisibility();
    void textFormatRenderer_appliesPaperPaletteToEditorAndPreviewHtml();
    void editorPresentationProjection_publishesHtmlBlockPipelineToQmlHost();
    void textFormatRenderer_preservesMarkdownUnorderedListMarkersWithoutRegexWarnings();
    void textFormatRenderer_keepsEnterNewlinesAsEditorParagraphSlots();
    void editorTagInsertionController_replacesLegacyInlineStyleMutationSupport();
    void editorTagInsertionController_buildsShortcutSourceWrapMutations();
    void displayPaperModels_hostPageAndPrintViewModeObjectsUnderModelsDirectory();
    void noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks();
    void noteBodyPersistence_stripsRenderedHtmlBlockArtifactsFromSourceProjection();
    void noteBodyPersistence_preservesEmptyParagraphCursorLineAfterResource();
    void noteBodyPersistence_preservesEmptyParagraphBoundariesAroundResources();
    void noteHeaderParser_usesIiXmlDocumentTreeForWsnHead();
    void localNoteFileStore_usesIiXmlDocumentTreeForWsnBodyRead();
    void editorRendererPipeline_routesIiXmlTreeThroughIiHtmlBlockObjects();
    void editorRendererPipeline_materializesEnterNewlinesAsParagraphSlots();
    void logicalTextBridge_advancesCursorPastClosingWebLinkTag();
    void logicalTextBridge_mapsSourceCursorInsideInlineTagsToVisibleBoundary();
    void qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode();
    void qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds();
    void qmlStructuredEditors_wireInlineResourceRendererToIiXmlHtmlBlockPipeline();
    void qmlEditors_routeRenderedHyperlinksToExternalBrowser();
    void qmlContentsView_composesFigmaFrameFromLvrsParts();
    void qmlContentsView_partsKeepEditorProjectionReadOnlyAndNativeInputSafe();
    void qmlLvrsTokens_replaceDirectHardcodedVisualTokensOutsideContents();
    void resourceBitmapViewer_projectsRenderableImagePreviewState();
    void editorSessionController_preservesLocalEditorAuthorityAgainstSameNoteModelSync();
    void editorSessionController_rebindsWhenSameNoteIdUsesDifferentPackagePath();
    void editorSessionController_commitsRawMutationsThroughSessionAuthority();
    void editorSessionBoundary_usesCppControllerWithoutQmlWrapper();
    void noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred();
    void noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative();
    void noteManagementCoordinator_loadNoteBodyText_preservesCanonicalSourceText();
    void noteManagementCoordinator_loadNoteBodyText_prefersExplicitNoteDirectoryPath();
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
