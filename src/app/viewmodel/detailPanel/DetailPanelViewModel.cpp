#include "DetailPanelViewModel.hpp"

#include "DetailPanelToolbarItemsFactory.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"

#include <QSet>

namespace
{
    QString normalizeFolderLookupKey(QString value)
    {
        return WhatSon::NoteFolders::normalizeFolderPath(std::move(value)).toCaseFolded();
    }

    int indexOfFolderPath(const WhatSonNoteHeaderStore& header, const QString& folderPath)
    {
        const QString targetKey = normalizeFolderLookupKey(folderPath);
        if (targetKey.isEmpty())
        {
            return -1;
        }

        const QStringList folders = header.folders();
        for (int index = 0; index < folders.size(); ++index)
        {
            if (normalizeFolderLookupKey(folders.at(index)) == targetKey)
            {
                return index;
            }
        }

        return -1;
    }

    int indexOfTag(const WhatSonNoteHeaderStore& header, const QString& tag)
    {
        const QString targetKey = tag.trimmed().toCaseFolded();
        if (targetKey.isEmpty())
        {
            return -1;
        }

        const QStringList tags = header.tags();
        for (int index = 0; index < tags.size(); ++index)
        {
            if (tags.at(index).trimmed().toCaseFolded() == targetKey)
            {
                return index;
            }
        }

        return -1;
    }
}

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
      , m_propertiesViewModel(this)
      , m_fileStatViewModel(this)
      , m_insertViewModel(DetailContentState::Insert, this)
      , m_fileHistoryViewModel(DetailContentState::FileHistory, this)
      , m_layerViewModel(DetailContentState::Layer, this)
      , m_helpViewModel(DetailContentState::Help, this)
      , m_noteHeaderSessionStore(this)
      , m_currentNoteContextBridge(this)
      , m_projectSelectionSourceViewModel(DetailNoteHeaderSelectionSourceViewModel::Field::Project, this)
      , m_bookmarkSelectionSourceViewModel(DetailNoteHeaderSelectionSourceViewModel::Field::Bookmark, this)
      , m_progressSelectionSourceViewModel(DetailNoteHeaderSelectionSourceViewModel::Field::Progress, this)
      , m_projectSelectionViewModel(QStringLiteral("DetailContent.ProjectSelection"), this)
      , m_bookmarkSelectionViewModel(QStringLiteral("DetailContent.BookmarkSelection"), this)
      , m_progressSelectionViewModel(QStringLiteral("DetailContent.ProgressSelection"), this)
{
    m_projectSelectionSourceViewModel.setSessionStore(&m_noteHeaderSessionStore);
    m_bookmarkSelectionSourceViewModel.setSessionStore(&m_noteHeaderSessionStore);
    m_progressSelectionSourceViewModel.setSessionStore(&m_noteHeaderSessionStore);

    QObject::connect(&m_currentNoteContextBridge,
                     &DetailCurrentNoteContextBridge::currentNoteIdChanged,
                     this,
                     [this]()
                     {
                         const QString noteId = m_currentNoteContextBridge.currentNoteId();
                         m_projectSelectionSourceViewModel.setNoteId(noteId);
                         m_bookmarkSelectionSourceViewModel.setNoteId(noteId);
                         m_progressSelectionSourceViewModel.setNoteId(noteId);
                         reloadCurrentHeader(false);
                     });
    QObject::connect(&m_currentNoteContextBridge,
                     &DetailCurrentNoteContextBridge::currentNoteDirectoryPathChanged,
                     this,
                     [this]()
                     {
                         const QString noteDirectoryPath = m_currentNoteContextBridge.currentNoteDirectoryPath();
                         m_projectSelectionSourceViewModel.setNoteDirectoryPath(noteDirectoryPath);
                         m_bookmarkSelectionSourceViewModel.setNoteDirectoryPath(noteDirectoryPath);
                         m_progressSelectionSourceViewModel.setNoteDirectoryPath(noteDirectoryPath);
                         reloadCurrentHeader(false);
                     });

    m_projectSelectionViewModel.setSourceViewModel(&m_projectSelectionSourceViewModel);
    m_bookmarkSelectionViewModel.setSourceViewModel(&m_bookmarkSelectionSourceViewModel);
    m_progressSelectionViewModel.setSourceViewModel(&m_progressSelectionSourceViewModel);
    applyActiveContentViewModel(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("activeState=%1 toolbarItemCount=%2")
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(this, QStringLiteral("detail.panel.viewmodel"), QStringLiteral("ctor"), detail);
}

DetailPanelViewModel::~DetailPanelViewModel() = default;

int DetailPanelViewModel::activeState() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_activeState);
}

QObject* DetailPanelViewModel::activeContentViewModel() const noexcept
{
    return m_activeContentViewModel;
}

QString DetailPanelViewModel::activeStateName() const
{
    return WhatSon::DetailPanel::stateName(m_activeState);
}

QObject* DetailPanelViewModel::contentViewModelForState(int stateValue) const noexcept
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        return nullptr;
    }

    switch (WhatSon::DetailPanel::stateFromValue(stateValue))
    {
    case DetailContentState::Properties:
        return const_cast<DetailPropertiesViewModel*>(&m_propertiesViewModel);
    case DetailContentState::FileStat:
        return const_cast<DetailFileStatViewModel*>(&m_fileStatViewModel);
    case DetailContentState::Insert:
        return const_cast<DetailContentSectionViewModel*>(&m_insertViewModel);
    case DetailContentState::FileHistory:
        return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
    case DetailContentState::Layer:
        return const_cast<DetailContentSectionViewModel*>(&m_layerViewModel);
    case DetailContentState::Help:
        return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
    }

    return nullptr;
}

QObject* DetailPanelViewModel::insertViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_insertViewModel);
}

QObject* DetailPanelViewModel::fileHistoryViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
}

QObject* DetailPanelViewModel::layerViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_layerViewModel);
}

QObject* DetailPanelViewModel::projectSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_projectSelectionViewModel);
}

QObject* DetailPanelViewModel::bookmarkSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_bookmarkSelectionViewModel);
}

QObject* DetailPanelViewModel::progressSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_progressSelectionViewModel);
}

QObject* DetailPanelViewModel::fileStatViewModel() const noexcept
{
    return const_cast<DetailFileStatViewModel*>(&m_fileStatViewModel);
}

QObject* DetailPanelViewModel::helpViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
}

QObject* DetailPanelViewModel::propertiesViewModel() const noexcept
{
    return const_cast<DetailPropertiesViewModel*>(&m_propertiesViewModel);
}

QVariantList DetailPanelViewModel::toolbarItems() const
{
    return m_toolbarItems;
}

void DetailPanelViewModel::setActiveState(int stateValue)
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        const QString detail = QStringLiteral("requestedStateValue=%1 result=ignored_invalid").arg(stateValue);
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.panel.viewmodel"),
            QStringLiteral("setActiveState.ignoredInvalid"),
            detail);
        return;
    }

    const QString previousStateName = activeStateName();
    const DetailContentState nextState = WhatSon::DetailPanel::stateFromValue(stateValue);
    if (nextState == m_activeState)
    {
        const QString detail = QStringLiteral("requestedStateValue=%1 requestedStateName=%2 result=ignored_same")
                               .arg(stateValue)
                               .arg(previousStateName);
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.panel.viewmodel"),
            QStringLiteral("setActiveState.ignoredSame"),
            detail);
        return;
    }

    m_activeState = nextState;
    applyActiveContentViewModel(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("requestedStateValue=%1 previousState=%2 nextState=%3 toolbarItemCount=%4")
                           .arg(stateValue)
                           .arg(previousStateName)
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.viewmodel"),
        QStringLiteral("setActiveState.applied"),
        detail);
    emit activeStateChanged();
    emit toolbarItemsChanged();
}

void DetailPanelViewModel::requestStateChange(int stateValue)
{
    const QString detail = QStringLiteral("requestedStateValue=%1").arg(stateValue);
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.viewmodel"),
        QStringLiteral("requestStateChange"),
        detail);
    setActiveState(stateValue);
}

void DetailPanelViewModel::setProjectSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_projectSelectionSourceViewModel.setOptionsSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::setBookmarkSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_bookmarkSelectionSourceViewModel.setOptionsSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::setProgressSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_progressSelectionSourceViewModel.setOptionsSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::setTagsSourceViewModel(QObject* sourceViewModel)
{
    m_tagsSourceViewModel = sourceViewModel;
}

void DetailPanelViewModel::setCurrentNoteListModel(QObject* noteListModel)
{
    m_currentNoteContextBridge.setNoteListModel(noteListModel);
    reconnectCurrentNoteListModelSignals(noteListModel);
}

void DetailPanelViewModel::setCurrentNoteDirectorySourceViewModel(QObject* sourceViewModel)
{
    m_currentNoteContextBridge.setNoteDirectorySourceViewModel(sourceViewModel);
}

bool DetailPanelViewModel::writeProjectSelection(int index)
{
    return writeSelectionIndex(
        m_projectSelectionSourceViewModel,
        index,
        DetailNoteHeaderSelectionSourceViewModel::Field::Project);
}

bool DetailPanelViewModel::writeBookmarkSelection(int index)
{
    return writeSelectionIndex(
        m_bookmarkSelectionSourceViewModel,
        index,
        DetailNoteHeaderSelectionSourceViewModel::Field::Bookmark);
}

bool DetailPanelViewModel::writeProgressSelection(int index)
{
    return writeSelectionIndex(
        m_progressSelectionSourceViewModel,
        index,
        DetailNoteHeaderSelectionSourceViewModel::Field::Progress);
}

bool DetailPanelViewModel::assignFolderByName(const QString& folderPath)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    WhatSonFoldersHierarchySessionService::FolderResolution resolution;
    if (!m_foldersHierarchySessionService.ensureFolderEntry(
            currentNoteDirectoryPath(),
            folderPath,
            &resolution,
            &errorMessage))
    {
        return false;
    }

    const QString noteId = currentNoteId();
    if (!m_noteHeaderSessionStore.assignFolderBinding(
            noteId,
            resolution.folderPath,
            resolution.folderUuid,
            &errorMessage))
    {
        return false;
    }

    const WhatSonNoteHeaderStore header = m_noteHeaderSessionStore.header(noteId);
    m_propertiesViewModel.applyHeader(header);
    const int activeFolderIndex = indexOfFolderPath(header, resolution.folderPath);
    if (activeFolderIndex >= 0)
    {
        m_propertiesViewModel.setActiveFolderIndex(activeFolderIndex);
    }
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

bool DetailPanelViewModel::assignTagByName(const QString& tag)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QString normalizedTag = tag.trimmed();
    if (normalizedTag.isEmpty())
    {
        return false;
    }

    const QString noteId = currentNoteId();
    if (!m_noteHeaderSessionStore.assignTag(noteId, normalizedTag, &errorMessage))
    {
        return false;
    }

    const WhatSonNoteHeaderStore header = m_noteHeaderSessionStore.header(noteId);
    m_propertiesViewModel.applyHeader(header);
    const int activeTagIndex = indexOfTag(header, normalizedTag);
    if (activeTagIndex >= 0)
    {
        m_propertiesViewModel.setActiveTagIndex(activeTagIndex);
    }
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

void DetailPanelViewModel::handleCurrentNoteItemsChanged()
{
    reloadCurrentHeader(true);
}

bool DetailPanelViewModel::removeActiveFolder()
{
    return removeMetadataEntry(true);
}

bool DetailPanelViewModel::removeActiveTag()
{
    return removeMetadataEntry(false);
}

bool DetailPanelViewModel::ensureCurrentHeaderLoaded(QString* errorMessage)
{
    const QString noteId = currentNoteId();
    const QString noteDirectoryPath = currentNoteDirectoryPath();
    if (noteId.isEmpty() || noteDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Current note context is incomplete.");
        }
        return false;
    }
    return m_noteHeaderSessionStore.ensureLoaded(noteId, noteDirectoryPath, errorMessage);
}

QString DetailPanelViewModel::currentNoteId() const
{
    return m_currentNoteContextBridge.currentNoteId().trimmed();
}

QString DetailPanelViewModel::currentNoteDirectoryPath() const
{
    return m_currentNoteContextBridge.currentNoteDirectoryPath().trimmed();
}

bool DetailPanelViewModel::writeSelectionIndex(
    const DetailNoteHeaderSelectionSourceViewModel& selectionSourceViewModel,
    int index,
    DetailNoteHeaderSelectionSourceViewModel::Field field)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QVariantList hierarchyModel = selectionSourceViewModel.hierarchyModel();
    if (index < 0 || index >= hierarchyModel.size())
    {
        return false;
    }
    if (index == selectionSourceViewModel.selectedIndex())
    {
        return true;
    }

    const QVariantMap entry = hierarchyModel.at(index).toMap();
    const bool clearSelection = entry.value(QStringLiteral("clearSelection")).toBool();
    const QString label = entry.value(QStringLiteral("label")).toString().trimmed();
    const QString noteId = currentNoteId();

    bool saved = false;
    switch (field)
    {
    case DetailNoteHeaderSelectionSourceViewModel::Field::Project:
        saved = m_noteHeaderSessionStore.updateProject(
            noteId,
            clearSelection ? QString() : label,
            &errorMessage);
        break;
    case DetailNoteHeaderSelectionSourceViewModel::Field::Bookmark:
        if (clearSelection)
        {
            saved = m_noteHeaderSessionStore.updateBookmarked(noteId, false, {}, &errorMessage);
        }
        else
        {
            saved = m_noteHeaderSessionStore.updateBookmarked(
                noteId,
                !label.isEmpty(),
                label.isEmpty() ? QStringList{} : QStringList{label.toLower()},
                &errorMessage);
        }
        break;
    case DetailNoteHeaderSelectionSourceViewModel::Field::Progress:
    {
        const int progress = DetailNoteHeaderSelectionSourceViewModel::progressValueForHierarchyEntry(
            hierarchyModel.at(index),
            index);
        saved = m_noteHeaderSessionStore.updateProgress(noteId, progress, &errorMessage);
        break;
    }
    }

    if (saved)
    {
        m_propertiesViewModel.applyHeader(m_noteHeaderSessionStore.header(noteId));
        m_projectSelectionSourceViewModel.synchronize(false);
        m_bookmarkSelectionSourceViewModel.synchronize(false);
        m_progressSelectionSourceViewModel.synchronize(false);
        synchronizeCurrentNoteMetadataConsumers(noteId);
    }
    return saved;
}

bool DetailPanelViewModel::removeMetadataEntry(const bool removeFolder)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QString noteId = currentNoteId();
    const int activeIndex = removeFolder
        ? m_propertiesViewModel.activeFolderIndex()
        : m_propertiesViewModel.activeTagIndex();
    if (activeIndex < 0)
    {
        return false;
    }

    const bool saved = removeFolder
        ? m_noteHeaderSessionStore.removeFolderAt(noteId, activeIndex, &errorMessage)
        : m_noteHeaderSessionStore.removeTagAt(noteId, activeIndex, &errorMessage);
    if (!saved)
    {
        return false;
    }

    m_propertiesViewModel.applyHeader(m_noteHeaderSessionStore.header(noteId));
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

void DetailPanelViewModel::applyActiveContentViewModel(DetailContentState activeState)
{
    m_propertiesViewModel.setActive(activeState == DetailContentState::Properties);
    m_fileStatViewModel.setActive(activeState == DetailContentState::FileStat);
    m_insertViewModel.setActive(activeState == DetailContentState::Insert);
    m_fileHistoryViewModel.setActive(activeState == DetailContentState::FileHistory);
    m_layerViewModel.setActive(activeState == DetailContentState::Layer);
    m_helpViewModel.setActive(activeState == DetailContentState::Help);
    m_activeContentViewModel = contentViewModelForState(WhatSon::DetailPanel::stateValue(activeState));
}

void DetailPanelViewModel::reconnectCurrentNoteListModelSignals(QObject* noteListModel)
{
    disconnectCurrentNoteListModelSignals();
    if (noteListModel == nullptr)
    {
        return;
    }

    const QMetaObject* metaObject = noteListModel->metaObject();
    if (metaObject == nullptr || metaObject->indexOfSignal("itemsChanged()") < 0)
    {
        return;
    }

    m_currentNoteListItemsChangedConnection = QObject::connect(
        noteListModel,
        SIGNAL(itemsChanged()),
        this,
        SLOT(handleCurrentNoteItemsChanged()),
        Qt::UniqueConnection);
}

void DetailPanelViewModel::disconnectCurrentNoteListModelSignals()
{
    if (m_currentNoteListItemsChangedConnection)
    {
        disconnect(m_currentNoteListItemsChangedConnection);
        m_currentNoteListItemsChangedConnection = {};
    }
}

void DetailPanelViewModel::reloadCurrentHeader(const bool forceReload)
{
    const QString noteId = m_currentNoteContextBridge.currentNoteId();
    const QString noteDirectoryPath = m_currentNoteContextBridge.currentNoteDirectoryPath();
    QString loadError;
    if (!noteId.isEmpty()
        && !noteDirectoryPath.isEmpty()
        && m_noteHeaderSessionStore.ensureLoaded(noteId, noteDirectoryPath, &loadError, forceReload))
    {
        const WhatSonNoteHeaderStore header = m_noteHeaderSessionStore.header(noteId);
        m_propertiesViewModel.applyHeader(header);
        m_fileStatViewModel.applyHeader(header);
        m_projectSelectionSourceViewModel.synchronize(false);
        m_bookmarkSelectionSourceViewModel.synchronize(false);
        m_progressSelectionSourceViewModel.synchronize(false);
        return;
    }

    m_propertiesViewModel.clearHeader();
    m_fileStatViewModel.clearHeader();
}

void DetailPanelViewModel::synchronizeCurrentNoteMetadataConsumers(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return;
    }

    QSet<QObject*> synchronizedConsumers;
    auto synchronizeConsumer = [&synchronizedConsumers, &normalizedNoteId](QObject* sourceViewModel)
    {
        if (sourceViewModel == nullptr || synchronizedConsumers.contains(sourceViewModel))
        {
            return;
        }

        const QMetaObject* metaObject = sourceViewModel->metaObject();
        if (metaObject == nullptr || metaObject->indexOfMethod("reloadNoteMetadataForNoteId(QString)") < 0)
        {
            return;
        }

        bool synchronized = false;
        QMetaObject::invokeMethod(
            sourceViewModel,
            "reloadNoteMetadataForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, synchronized),
            Q_ARG(QString, normalizedNoteId));
        synchronizedConsumers.insert(sourceViewModel);
    };

    synchronizeConsumer(m_currentNoteContextBridge.noteDirectorySourceViewModel());
    synchronizeConsumer(m_projectSelectionSourceViewModel.optionsSourceViewModel());
    synchronizeConsumer(m_bookmarkSelectionSourceViewModel.optionsSourceViewModel());
    synchronizeConsumer(m_progressSelectionSourceViewModel.optionsSourceViewModel());
    synchronizeConsumer(m_tagsSourceViewModel);
}
