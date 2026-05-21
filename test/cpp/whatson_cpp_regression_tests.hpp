#pragma once

#include "app/models/file/hub/WhatSonHubMountValidator.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/conflict/WhatSonTimestampConflictResolver.hpp"
#include "app/models/file/sync/WhatSonEditorRawPullController.hpp"
#include "app/models/file/sync/WhatSonEditorRawPushController.hpp"
#include "app/models/editor/EditorInputCommandFilter.hpp"
#include "app/models/clipboard/ClipboardEditorPaste.h"
#include "app/models/clipboard/FiletypeCapture.h"
#include "app/models/clipboard/InAppClipboardManager.h"
#include "app/models/clipboard/InAppClipboardStore.h"
#include "app/models/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "app/models/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#define private public
#include "app/models/file/note/session/ContentsNoteManagementCoordinator.hpp"
#undef private
#include "app/models/file/note/local/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/header/WhatSonNoteHeaderCreator.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/note/header/WhatSonNoteHeaderParser.hpp"
#include "app/models/file/note/folder/WhatSonNoteFolderSemantics.hpp"
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
#include "app/models/hierarchy/IHierarchyController.hpp"
#include "app/models/hierarchy/IHierarchyCapabilities.hpp"
#include "app/models/hierarchy/WhatSonHierarchyModel.hpp"
#include "app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "app/models/hierarchy/library/LibraryHierarchyController.hpp"
#include "app/models/hierarchy/library/LibraryNoteListModel.hpp"
#include "app/models/hierarchy/progress/ProgressHierarchyControllerSupport.hpp"
#include "app/models/hierarchy/resources/ResourcesHierarchyController.hpp"
#include "app/models/navigationbar/EditorViewSectionController.hpp"
#include "app/models/navigationbar/EditorViewState.hpp"
#include "app/models/navigationbar/EditorViewModeController.hpp"
#include "app/models/navigationbar/NavigationModeSectionController.hpp"
#include "app/models/navigationbar/NavigationModeState.hpp"
#include "app/models/navigationbar/NavigationModeController.hpp"
#include "app/models/onboarding/IOnboardingHubController.hpp"
#include "app/models/onboarding/OnboardingRouteBootstrapController.hpp"
#include "app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.hpp"
#include "app/models/detailPanel/DetailCurrentNoteContextBridge.hpp"
#include "app/models/detailPanel/ResourceDetailPanelController.hpp"
#include "app/models/hierarchy/resources/ResourcesListModel.hpp"
#include "app/models/editor/GetProperty.h"
#include "app/models/editor/component/Break.h"
#include "app/models/editor/component/Callout.h"
#include "app/models/editor/component/ResourceImageFrame.h"
#include "app/models/editor/TagInsertionWriter.hpp"
#include "app/models/editor/NoteEditorDocumentSession.hpp"
#include "app/models/editor/SetProperty.h"
#include "app/models/editor/SetTag.h"
#include "app/models/panel/HierarchyInteractionBridge.hpp"
#include "app/models/panel/NoteListModelContractBridge.hpp"
#include "app/models/panel/NoteActiveStateTracker.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/sidebar/HierarchyControllerProvider.hpp"
#include "app/models/sidebar/IActiveHierarchyContextSource.hpp"
#include "app/models/sidebar/SidebarHierarchyController.hpp"
#include "app/models/sidebar/SidebarHierarchyInteractionController.hpp"

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
#include <QSet>
#include <QThreadPool>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>
#include <QtTest>

#include <cmath>
#include <memory>
#include <vector>

class FakeHierarchyController : public IHierarchyController
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

class FakeSidebarHierarchyInteractionBridge final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool createFolderEnabled MEMBER createFolderEnabled)
    Q_PROPERTY(bool deleteFolderEnabled MEMBER deleteFolderEnabled)
    Q_PROPERTY(bool viewOptionsEnabled MEMBER viewOptionsEnabled)

public:
    bool createFolderEnabled = true;
    bool deleteFolderEnabled = true;
    bool viewOptionsEnabled = true;
    int setItemExpandedCallCount = 0;
    int setAllItemsExpandedCallCount = 0;
    int lastExpandedIndex = -1;
    bool lastExpandedValue = false;
    bool setItemExpandedResult = true;
    bool setAllItemsExpandedResult = true;

    Q_INVOKABLE bool setItemExpanded(int index, bool expanded)
    {
        ++setItemExpandedCallCount;
        lastExpandedIndex = index;
        lastExpandedValue = expanded;
        return setItemExpandedResult;
    }

    Q_INVOKABLE bool setAllItemsExpanded(bool expanded)
    {
        ++setAllItemsExpandedCallCount;
        lastExpandedValue = expanded;
        return setAllItemsExpandedResult;
    }
};

class FakeExpandableHierarchyController final : public FakeHierarchyController,
                                                public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyExpansionCapability)

public:
    explicit FakeExpandableHierarchyController(const QString& labelPrefix, QObject* parent = nullptr)
        : FakeHierarchyController(labelPrefix, parent)
    {
    }

    bool setItemExpanded(int index, bool expanded) override
    {
        QVariantList nodes = hierarchyModel();
        if (index < 0 || index >= nodes.size())
        {
            return false;
        }

        QVariantMap node = nodes.at(index).toMap();
        if (!node.value(QStringLiteral("showChevron")).toBool())
        {
            return false;
        }

        node.insert(QStringLiteral("expanded"), expanded);
        nodes[index] = node;
        setNodes(nodes);
        ++setItemExpandedCallCount;
        lastExpandedIndex = index;
        lastExpandedValue = expanded;
        return true;
    }

    bool expandedAt(int index) const
    {
        const QVariantList nodes = hierarchyModel();
        if (index < 0 || index >= nodes.size())
        {
            return false;
        }
        return nodes.at(index).toMap().value(QStringLiteral("expanded")).toBool();
    }

    int setItemExpandedCallCount = 0;
    int lastExpandedIndex = -1;
    bool lastExpandedValue = false;
};

class FakeNoteDropHierarchyController final : public FakeHierarchyController,
                                              public IHierarchyNoteDropCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyNoteDropCapability)

public:
    explicit FakeNoteDropHierarchyController(const QString& labelPrefix, QObject* parent = nullptr)
        : FakeHierarchyController(labelPrefix, parent)
    {
    }

    bool canAcceptNoteDrop(int index, const QString& noteId) const override
    {
        const QString normalizedNoteId = noteId.trimmed();
        return m_supportsNoteDrop
            && m_acceptedDropIndices.contains(index)
            && !normalizedNoteId.isEmpty()
            && !m_rejectedNoteIds.contains(normalizedNoteId)
            && !m_assignedNoteIdsByIndex.value(index).contains(normalizedNoteId);
    }

    bool assignNoteToFolder(int index, const QString& noteId) override
    {
        if (!canAcceptNoteDrop(index, noteId))
        {
            return false;
        }

        const QString normalizedNoteId = noteId.trimmed();
        m_assignedNoteIdsByIndex[index].insert(normalizedNoteId);
        m_assignedNoteIds.push_back(normalizedNoteId);
        return true;
    }

    bool supportsHierarchyNoteDrop() const noexcept override
    {
        return m_supportsNoteDrop;
    }

    void setAcceptedDropIndices(const QSet<int>& indices)
    {
        m_acceptedDropIndices = indices;
    }

    void setSupportsNoteDrop(bool supported) noexcept
    {
        m_supportsNoteDrop = supported;
    }

    void rejectNoteId(const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (!normalizedNoteId.isEmpty())
        {
            m_rejectedNoteIds.insert(normalizedNoteId);
        }
    }

    QStringList assignedNoteIds() const
    {
        return m_assignedNoteIds;
    }

private:
    QSet<int> m_acceptedDropIndices;
    QSet<QString> m_rejectedNoteIds;
    QHash<int, QSet<QString>> m_assignedNoteIdsByIndex;
    QStringList m_assignedNoteIds;
    bool m_supportsNoteDrop = true;
};

class FakeReorderHierarchyController final : public FakeHierarchyController,
                                             public IHierarchyReorderCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyReorderCapability)

public:
    explicit FakeReorderHierarchyController(const QString& labelPrefix, QObject* parent = nullptr)
        : FakeHierarchyController(labelPrefix, parent)
    {
    }

    bool applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey = QString()) override
    {
        if (!m_supportsReorder || hierarchyNodes.isEmpty())
        {
            return false;
        }

        m_appliedNodes = hierarchyNodes;
        m_appliedActiveItemKey = activeItemKey.trimmed();
        setNodes(hierarchyNodes);
        return true;
    }

    bool supportsHierarchyNodeReorder() const noexcept override
    {
        return m_supportsReorder;
    }

    bool applyHierarchyMove(
        int sourceIndex,
        int targetIndex,
        int targetDepth,
        const QString& activeItemKey = QString()) override
    {
        if (!m_supportsReorder || sourceIndex < 0 || targetIndex < 0)
        {
            return false;
        }

        m_appliedMoveSourceIndex = sourceIndex;
        m_appliedMoveTargetIndex = targetIndex;
        m_appliedMoveTargetDepth = targetDepth;
        m_appliedActiveItemKey = activeItemKey.trimmed();
        return true;
    }

    QVariantList appliedNodes() const
    {
        return m_appliedNodes;
    }

    QString appliedActiveItemKey() const
    {
        return m_appliedActiveItemKey;
    }

    int appliedMoveSourceIndex() const noexcept
    {
        return m_appliedMoveSourceIndex;
    }

    int appliedMoveTargetIndex() const noexcept
    {
        return m_appliedMoveTargetIndex;
    }

    int appliedMoveTargetDepth() const noexcept
    {
        return m_appliedMoveTargetDepth;
    }

    void setSupportsReorder(bool supported) noexcept
    {
        m_supportsReorder = supported;
    }

private:
    QVariantList m_appliedNodes;
    QString m_appliedActiveItemKey;
    int m_appliedMoveSourceIndex = -1;
    int m_appliedMoveTargetIndex = -1;
    int m_appliedMoveTargetDepth = -1;
    bool m_supportsReorder = true;
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

    Q_INVOKABLE bool saveCurrentBodyText(const QString& text)
    {
        ++saveCurrentBodyTextCallCount;
        lastSavedCurrentBodyText = text;
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
    int saveCurrentBodyTextCallCount = 0;
    QString lastAppliedNoteId;
    QString lastAppliedBodyPlainText;
    QString lastAppliedBodySourceText;
    QString lastAppliedLastModifiedAt;
    QString lastReloadedNoteId;
    QString lastSavedCurrentBodyText;

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
    void appLaunchSupport_requiresMountedHubForStartupWorkspace();
    void qmlLaunchSupport_routesRootLoadingThroughLvrsAppEntry();
    void foregroundServiceGate_startsSchedulerAndPermissionsAfterVisibleWorkspace();
    void startupRuntimeLoad_usesLvrsAfterFirstIdleLifecycleTask();
    void architecturePolicyLock_blocksMutableWiringAfterLock();
    void hierarchyControllerProvider_rejectsMappingMutationAfterLock();
    void sidebarHierarchyController_rejectsSelectionStoreMutationAfterLock();
    void noteListModelContractBridge_rejectsWiringMutationAfterLock();
    void noteActiveStateTracker_rejectsHierarchyContextMutationAfterLock();
    void detailCurrentNoteContextBridge_rejectsWiringMutationAfterLock();
    void onboardingRouteBootstrapController_rejectsHubControllerMutationAfterLock();
    void filetypeCapture_ownsClipboardFileTypeDetection();
    void inAppClipboard_extractsPlatformImageMimePayloads();
    void inAppClipboardStore_ownsResourceSnapshotState();
    void inAppClipboard_matchesMimeAndFileTypesToResourceTaxonomy();
    void inAppClipboard_acceptsNonImagePayloadsFromAppAndMimeData();
    void cronExpression_and_asyncScheduler_coverParsingMatchingAndDeduplication();
    void debugTraceFilter_suppressesIiXmlDebugSpamByDefault();
    void debugTrace_summarizesLargeTextFromPreviewOnly();
    void editorGetProperty_capturesTypedKeyValuePairsFromResourceTag();
    void editorGetProperty_updatesStoredStateAndClearsWhenNoTag();
    void editorGetProperty_readsBodyDocumentAttributesIntoAppStore();
    void editorSetProperty_setsDynamicAttributesWithInferredValueTypes();
    void editorSetProperty_updatesExistingAttributeAndRejectsInvalidNames();
    void editorSetProperty_serializesResourceAttributeIntoWsnbodyDocument();
    void editorSetTag_insertsStaticCalloutPairIntoSourceSelection();
    void editorSetTag_rejectsUnsupportedStaticNames();
    void editorSetTag_addsHeaderSubheaderAndResourceTemplates();
    void editorSetTag_togglesSameInlineFormatWhenSelectionMatchesWrappedContent();
    void editorSetTag_serializesInsertedStaticTagIntoWsnbodyDocument();
    void editorTagInsertionWriter_writesHeaderTagIntoLocalWsnbody();
    void editorTagInsertionWriter_writesStandaloneResourceAsBodyNode();
    void editorTagInsertionWriter_rejectsUnsupportedTagWithoutChangingBody();
    void cmakeDependencyWiring_declaresLocalXmlAndHtmlBlockPackages();
    void cmakeBuildTargets_cleanTransientBuildDiagnostics();
    void detailPanelRouting_separatesNoteAndResourceViewsAndControllers();
    void foldersHierarchyParser_escapesLiteralSlashLabelsIntoSingleSegments();
    void foldersHierarchySessionService_preservesEscapedLiteralSlashFolderPaths();
    void hierarchyControllerProvider_normalizesMappingsAndAvoidsDuplicateSignals();
    void hierarchyDragDropBridge_assignsDraggedNoteListItemsToFolderCapability();
    void hierarchyDragDropBridge_appliesReorderFromQmlArrayModel();
    void hierarchyTreeItemSupport_clampsNegativeSelectionToFirstVisibleRow();
    void hubMountValidator_acceptsCompleteHubPackage();
    void hubMountValidator_rejectsIncompleteHubPackage();
    void sourceTree_usesRepositoryAbsoluteProjectIncludes();
    void sourceTree_forbidsDeprecatedPresentationLayerVocabulary();
    void sourceTree_keepsEditorModelBackendRegistered();
    void sourceTree_keepsContentsQmlUnderViewContents();
    void sourceTree_keepsHierarchyBackendDecomposed();
    void sourceTree_keepsNoteFileShardClassifiedByResponsibility();
    void sourceTree_forbidsSharedFileIoObjectLayer();
    void iosBundleIconPackaging_declaresPrimaryAndIpadFallbackIconsInInfoPlist();
    void iosBundleIconPackaging_stagesBundleRootPngsEvenWithAssetCatalogsEnabled();
    void appleBundleIconPackaging_usesMacosAppBundleIconResourceContract();
    void iosXcodeprojExport_surfacesSdkSigningAndPermissionPolicyOptionsInCmake();
    void iosXcodeprojExport_routesSimulatorPermissionFallbackThroughAppRuntimeCmake();
    void iosXcodeprojExport_embedsLocalDynamicLibrariesIntoIosBundle();
    void iosXcodeprojExport_patchScriptStripsQtPermissionsEvenWhenIconPhaseAlreadyExists();
    void iosXcodeprojExport_keepsBuildIosScriptOnHighLevelCmakeOptions();
    void libraryHierarchyController_keepsInAppScaffoldIndependentFromHubSnapshots();
    void libraryHierarchyController_appliesLvrsMoveEventAsSingleFolderReparent();
    void libraryHierarchyController_mirrorsFoldersFileAfterHierarchyCommit();
    void libraryHierarchyController_clearsSelectionAfterDeletingFocusedFolder();
    void libraryNoteListModel_emitsCurrentNoteEntryChangedWhenInitialSelectionMaterializes();
    void libraryNoteListModel_emitsCurrentNoteEntryChangedWhenSelectedRowReplacesCurrentSelection();
    void libraryNoteListModel_hidesRawInlineTagsFromPreviewText();
    void mobileChrome_keepsRestoredShellWithEditorViewModeCombo();
    void mobileHierarchyRouteStateStore_tracksNormalizedSelectionRestoreState();
    void mobileHierarchySelectionCoordinator_prefersExplicitSidebarBindingsAndFallbacks();
    void mobileHierarchyNavigationCoordinator_routesBackAsDismissTargets();
    void navigationModeController_cyclesActiveSections();
    void editorViewModeController_cyclesActiveSections();
    void noteActiveStateTracker_tracksCurrentNoteAcrossActiveHierarchyChanges();
    void noteActiveStateTracker_clearsReadableEmptyAndNonNoteBackedSelections();
    void noteActiveStateTracker_publishesAtomicNoteSnapshotBeforeChangeSignals();
    void noteActiveStateTracker_publishesBodyPathForNoteEditorSessionResolution();
    void breakComponent_projectsStandaloneBreakAsLogicalEditorLine();
    void calloutComponent_rendersFigmaCalloutBlock();
    void calloutComponent_plansBoundaryEditsAgainstDecoratedCursor();
    void resourceFrame_rendersImageOnlyContainer();
    void noteEditorDocumentSession_mountsEditorHtmlFileAndPersistsBodyDocument();
    void noteEditorDocumentSession_keepsSessionSourceWhenSameNoteIsReselected();
    void noteEditorDocumentSession_incrementsOpenCountAfterSuccessfulOpen();
    void noteEditorDocumentSession_buildsInlineFormatSourceInsertion();
    void noteEditorDocumentSession_backspaceAtCalloutInitRemovesCalloutWrapper();
    void noteEditorDocumentSession_calloutFrameChromeDoesNotCreateExtraEditorLine();
    void noteEditorDocumentSession_enterInsideCalloutMovesCursorOutside();
    void noteEditorDocumentSession_projectsBreakSourceLineWithoutLiteralTagText();
    void noteEditorDocumentSession_usesSelectedTextToRepairDriftedFormatSelection();
    void noteEditorDocumentSession_mapsLogicalSelectionAgainstLoadedBodySourceBreaks();
    void noteEditorDocumentSession_formatsSelectionAgainstBodySourceWhenEditorHtmlDropsBlankLines();
    void noteEditorDocumentSession_formatsAgainstLoadedBodySourceWhenEditorProjectionDropsRawTags();
    void noteEditorDocumentSession_buildsStandaloneResourceSourceInsertion();
    void noteEditorDocumentSession_rendersImportedClipboardImageResourceFrame();
    void noteEditorDocumentSession_reprojectsCalloutFrameChromeOnTextChange();
    void noteEditorDocumentSession_persistsBackspacedResourceFrameAsComponentDeletion();
    void noteBodyPersistence_roundTripsAndProjectsCanonicalWebLinks();
    void noteBodyPersistence_projectsSourceToEditorHtmlWithExplicitBreaks();
    void noteBodyPersistence_recoversEditorHtmlBreaksAsCanonicalSourceLines();
    void noteBodyPersistence_recoversEditorFormattingTagsFromRichText();
    void noteBodyPersistence_roundTripsCanonicalStyleTagAttributes();
    void noteBodyPersistence_preservesCrossParagraphInlineSourceTagsWithoutEscaping();
    void noteBodyPersistence_projectsCalloutAsFigmaBlockAndRecoversSource();
    void noteBodyPersistence_preservesExplicitBlankLineBeforeStandaloneCallout();
    void noteBodyPersistence_doesNotReplicateParagraphsAroundRepeatedCalloutSaves();
    void noteBodyPersistence_persistsCalloutAsParagraphTag();
    void noteBodyPersistence_changedPlainTextSaveAdvancesModifiedCount();
    void noteBodyPersistence_stripsRenderedHtmlBlockArtifactsFromSourceProjection();
    void noteBodyPersistence_recoversRenderedResourceFrameMarkersAsSourceTags();
    void noteBodyPersistence_dropsDeletedSingleResourceObjectMarker();
    void noteBodyPersistence_preservesEmptyParagraphCursorLineAfterResource();
    void noteBodyPersistence_preservesEmptyParagraphBoundariesAroundResources();
    void noteFileStatSupport_incrementsOpenCountAndPersistsLastOpenedAt();
    void noteHeaderParser_usesIiXmlDocumentTreeForWsnHead();
    void localNoteFileStore_usesIiXmlDocumentTreeForWsnBodyRead();
    void noteListModelContractBridge_resolvesHierarchyBoundNoteListImmediately();
    void noteListModelContractBridge_prefersExplicitRowsAcrossHierarchySwitches();
    void noteManagementCoordinator_reconcilePersistsEditorSnapshotWhenPreferred();
    void noteManagementCoordinator_reconcileRefreshesWithoutPersistingWhenEditorIsNotAuthoritative();
    void noteManagementCoordinator_directBodyPersistAdvancesModifiedCount();
    void timestampConflictResolver_prefersNewestBodyAfterBaseDivergence();
    void timestampConflictResolver_reportsStrictlyNewerTimestamp();
    void localNoteFileStore_keepsFilesystemBodyWhenTimestampConflictIsNewer();
    void localNoteFileStore_acceptsIncomingBodyWhenTimestampConflictIsNewer();
    void editorRawPullController_requestsNoteEntryAndOpenPulls();
    void editorRawPullController_pullsActiveNoteEveryIdleInterval();
    void editorRawPushController_pushesOnIdleModifiedCountAndNoteDeparture();
    void hubSyncController_splitsFilesystemResponsibilitiesIntoDedicatedObjects();
    void hubSyncObservationBuilder_ignoresPrivateWhatSonBookkeeping();
    void hubSyncWiring_includesNoteEditorSessionVersionDiffMutations();
    void localNoteVersionStore_splitsVersionResponsibilitiesIntoDedicatedObjects();
    void localNoteVersionStore_capturesCommitSnapshotWhenNoteUpdateAdvancesModifiedCount();
    void localNoteVersionStore_reportsVersionDiffFilesystemPushForSync();
    void localNoteVersionStore_skipsModifiedCountWhenNoVersionDiffIsWritten();
    void localNoteVersionStore_prunesSnapshotsToLatestOneHundred();
    void noteManagementCoordinator_openCountReloadsPersistedMetadata();
    void noteManagementCoordinator_loadNoteBodyText_preservesCanonicalSourceText();
    void noteManagementCoordinator_loadNoteBodyText_prefersExplicitNoteDirectoryPath();
    void noteListModelContractBridge_exposesCurrentNoteEntryFromCurrentSelection();
    void detailCurrentNoteContextBridge_prefersCurrentNoteEntryAndClearsNonNoteBackedSelection();
    void detailCurrentNoteContextBridge_clearsReadableEmptyCurrentNoteEntrySelection();
    void onboardingRouteBootstrapController_syncsEmbeddedOnboardingLifecycle();
    void progressHierarchySupport_defaultsFirstVisibleItemToFirstDraft();
    void projectsHierarchyParser_roundTripsNestedProjectTree();
    void projectsHierarchyController_keepsNestedProjectPolicy();
    void qmlContentsView_keepsOnlyAllowedContentsViews();
    void qmlContentsViewsStayViewOnlyAndNativeInputSafe();
    void qmlNavigationCalendarBars_restoreTaskButtonWithoutLegacyHooks();
    void qmlNavigationCalendarButtons_mountCalendarPagesInContentSurface();
    void qmlOnboardingContent_routesMacCreateHubThroughDirectoryDialog();
    void qmlLvrsTokens_replaceDirectHardcodedVisualTokensOutsideContents();
    void qmlContextBinder_usesLvrsBindPlanForWorkspaceContextObjects();
    void qmlContentViewLayout_wiresEditorFormatShortcutsOutsideTextEditor();
    void qmlContentViewLayout_opensEditorFormatContextMenuForSelection();
    void noteEditorDocumentSession_pushesSurfaceTextToRawOnIdleRequest();
    void noteEditorDocumentSession_pushesQtSerializedCalloutToRawOnIdleRequest();
    void noteEditorDocumentSession_pushesSurfaceTextToRawOnModifiedCountIncrease();
    void noteEditorDocumentSession_pushesSurfaceTextToRawOnNoteDeparture();
    void noteEditorDocumentSession_emitsHubFilesystemMutationForVersionDiffPush();
    void noteEditorDocumentSession_routesOpenPullThroughSyncController();
    void noteEditorDocumentSession_pullsOnlyNewerFilesystemBodyOnIdle();
    void qmlContentsTextEditor_keepsLvrsTextEditorSurface();
    void qmlContentsTextEditor_excludesSnapshotProjectionPersistence();
    void qmlContentsTextEditor_keepsNativeSurfaceOnly();
    void qmlContentsTextEditor_keepsKeyboardSelectionAndOsImeNative();
    void resourceDetailPanelController_tracksCurrentResourceSelection();
    void resourcePackageSupport_roundTripsAnnotationMetadataAndBitmap();
    void resourcePackageSupport_normalizesTerminalFormatForMultiDotAssetNames();
    void resourcePackageSupport_normalizesMusicAliasToAudioTaxonomy();
    void noteFolderSemantics_normalizeDescriptorsAndXml();
    void resourcesHierarchyController_defaultsSelectionToImageAndFiltersList();
    void resourcesHierarchyController_collapsesMultiDotImageFormatsIntoTerminalSuffix();
    void resourcesHierarchyController_mergesLegacyMusicResourcesIntoAudio();
    void resourcesHierarchyController_publishesDepthItemsToSharedModel();
    void resourcesHierarchyController_updatesChevronExpansionThroughSharedModelRow();
    void resourcesHierarchyController_commitsChevronExpansionThroughSharedBridge();
    void inAppClipboard_wiresAnnotationBitmapGenerationIntoPackageCreation();
    void inAppClipboard_importsUrlsForEditorAsResourcePackages();
    void inAppClipboard_importsClipboardImageThroughManager();
    void inAppClipboard_importsClipboardImagesWithRandomAlnumResourceIds();
    void inAppClipboard_randomizesClipboardResourceNameBeforeConflictPreflight();
    void inAppClipboard_importsNonImageClipboardPayloadThroughManager();
    void clipboardEditorPaste_insertsImageResourceThroughPasteObject();
    void clipboardEditorPaste_capturesSystemClipboardImageForEditorPaste();
    void clipboardEditorPaste_importsPlatformImageMimePayloadForEditorPaste();
    void clipboardEditorPaste_rejectsStaleSnapshotWhenSystemClipboardCannotCapture();
    void inAppClipboard_refreshReplacesStaleSnapshotWithSystemClipboardImage();
    void clipboardEditorPaste_fallsBackForNonImageResource();
    void runtimeParallelLoader_usesLvrsBootstrapParallelForDomainLoads();
    void selectedHubStore_persistsNormalizedSelectionsWithinSandboxedSettings();
    void sidebarHierarchyController_forcesCppOwnershipAcrossHierarchySwitchBindings();
    void sidebarHierarchyController_preservesFallbackAcrossStoreAttachDetach();
    void hierarchyItemModel_usesSharedLvrsModelContract();
    void hierarchyControllers_exposeSharedLvrsHierarchyModel();
    void sidebarHierarchyInteractionController_keepsFooterDispatchOutOfCppPolicy();
    void hierarchyController_parentExpansionPolicyMutatesOnlyChevronRows();
    void hierarchyControllers_delegateChevronExpansionToParentPolicy();
    void sidebarHierarchyInteractionController_commitsExpansionStateThroughCppPolicy();
    void hierarchyInteractionBridge_bindsRuntimeControllerAfterArchitectureLock();
    void hierarchyInteractionBridge_rebindsActiveRuntimeControllerAfterArchitectureLock();
    void sidebarHierarchyController_reactsToProviderMappingChanges();
    void sidebarHierarchyRenameController_preservesLiteralSlashFolderLabels();
    void sidebarHierarchyView_bindsInlineHelperDependenciesAtStartup();
    void sidebarHierarchyView_usesResourcesSnapshotRenderModel();
    void sidebarHierarchyView_keepsResourcesChevronExpansionLocalToLvrsSnapshot();
    void sidebarHierarchyView_waitsForCreatedFolderRowBeforeInlineRename();
    void sidebarHierarchyView_noteDropSurfaceDoesNotInterceptHierarchyItemDrags();
    void sidebarHierarchyView_chevronHitTestUsesLvrsChevronSlotContentItem();
    void sidebarHierarchyView_doesNotOverlayLvrsChevronClicks();
    void sidebarHierarchyView_chevronTapFallbackScopesCommitToPressedItem();
    void sidebarHierarchyView_routesFooterActionsDirectlyFromQml();
    void sidebarSelectionStore_normalizesIndicesAndSuppressesDuplicateSignals();
    void startupHubResolver_returnsEmptyWithoutPersistedSelection();
    void startupHubResolver_mountsPersistedCompleteHubPackage();
    void startupHubResolver_keepsPersistedFailureVisibleWithoutSwitchingToBlueprint();
    void unixTimeAnalyzer_reportsStableEpochFields();
    void unusedNoteSensors_filterNoteIdsByLastOpenedWindow();
    void unusedResourcesSensor_reportsHubPackagesMissingFromAllNoteEmbeddings();
    void unusedResourcesSensor_refreshesAfterRawBodyEmbedsAResource();

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
