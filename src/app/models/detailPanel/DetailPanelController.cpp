#include "app/models/detailPanel/DetailPanelController.hpp"

#include "app/models/detailPanel/DetailPanelToolbarItemsFactory.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"

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

DetailPanelController::DetailPanelController(QObject* parent)
    : QObject(parent)
      , m_propertiesController(this)
      , m_fileStatController(this)
      , m_insertController(DetailContentState::Insert, this)
      , m_fileHistoryController(DetailContentState::FileHistory, this)
      , m_layerController(DetailContentState::Layer, this)
      , m_helpController(DetailContentState::Help, this)
      , m_noteHeaderSessionStore(this)
      , m_currentNoteContextBridge(this)
      , m_projectSelectionSourceController(DetailNoteHeaderSelectionSourceController::Field::Project, this)
      , m_bookmarkSelectionSourceController(DetailNoteHeaderSelectionSourceController::Field::Bookmark, this)
      , m_progressSelectionSourceController(DetailNoteHeaderSelectionSourceController::Field::Progress, this)
      , m_projectSelectionController(QStringLiteral("DetailContent.ProjectSelection"), this)
      , m_bookmarkSelectionController(QStringLiteral("DetailContent.BookmarkSelection"), this)
      , m_progressSelectionController(QStringLiteral("DetailContent.ProgressSelection"), this)
{
    m_projectSelectionSourceController.setSessionStore(&m_noteHeaderSessionStore);
    m_bookmarkSelectionSourceController.setSessionStore(&m_noteHeaderSessionStore);
    m_progressSelectionSourceController.setSessionStore(&m_noteHeaderSessionStore);

    QObject::connect(&m_currentNoteContextBridge,
                     &DetailCurrentNoteContextBridge::currentNoteIdChanged,
                     this,
                     [this]()
                     {
                         const QString noteId = m_currentNoteContextBridge.currentNoteId();
                         m_projectSelectionSourceController.setNoteId(noteId);
                         m_bookmarkSelectionSourceController.setNoteId(noteId);
                         m_progressSelectionSourceController.setNoteId(noteId);
                         reloadCurrentHeader(false);
                     });
    QObject::connect(&m_currentNoteContextBridge,
                     &DetailCurrentNoteContextBridge::currentNoteDirectoryPathChanged,
                     this,
                     [this]()
                     {
                         const QString noteDirectoryPath = m_currentNoteContextBridge.currentNoteDirectoryPath();
                         m_projectSelectionSourceController.setNoteDirectoryPath(noteDirectoryPath);
                         m_bookmarkSelectionSourceController.setNoteDirectoryPath(noteDirectoryPath);
                         m_progressSelectionSourceController.setNoteDirectoryPath(noteDirectoryPath);
                         reloadCurrentHeader(false);
                     });

    m_projectSelectionController.setSourceController(&m_projectSelectionSourceController);
    m_bookmarkSelectionController.setSourceController(&m_bookmarkSelectionSourceController);
    m_progressSelectionController.setSourceController(&m_progressSelectionSourceController);
    applyActiveContentController(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("activeState=%1 toolbarItemCount=%2")
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(this, QStringLiteral("detail.panel.controller"), QStringLiteral("ctor"), detail);
}

DetailPanelController::~DetailPanelController() = default;

int DetailPanelController::activeState() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_activeState);
}

QObject* DetailPanelController::activeContentController() const noexcept
{
    return m_activeContentController;
}

QString DetailPanelController::activeStateName() const
{
    return WhatSon::DetailPanel::stateName(m_activeState);
}

bool DetailPanelController::noteContextLinked() const noexcept
{
    return m_noteContextLinked;
}

QObject* DetailPanelController::contentControllerForState(int stateValue) const noexcept
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        return nullptr;
    }

    switch (WhatSon::DetailPanel::stateFromValue(stateValue))
    {
    case DetailContentState::Properties:
        return const_cast<DetailPropertiesController*>(&m_propertiesController);
    case DetailContentState::FileStat:
        return const_cast<DetailFileStatController*>(&m_fileStatController);
    case DetailContentState::Insert:
        return const_cast<DetailContentSectionController*>(&m_insertController);
    case DetailContentState::FileHistory:
        return const_cast<DetailContentSectionController*>(&m_fileHistoryController);
    case DetailContentState::Layer:
        return const_cast<DetailContentSectionController*>(&m_layerController);
    case DetailContentState::Help:
        return const_cast<DetailContentSectionController*>(&m_helpController);
    }

    return nullptr;
}

QObject* DetailPanelController::insertController() const noexcept
{
    return const_cast<DetailContentSectionController*>(&m_insertController);
}

QObject* DetailPanelController::fileHistoryController() const noexcept
{
    return const_cast<DetailContentSectionController*>(&m_fileHistoryController);
}

QObject* DetailPanelController::layerController() const noexcept
{
    return const_cast<DetailContentSectionController*>(&m_layerController);
}

QObject* DetailPanelController::projectSelectionController() const noexcept
{
    return const_cast<DetailHierarchySelectionController*>(&m_projectSelectionController);
}

QObject* DetailPanelController::bookmarkSelectionController() const noexcept
{
    return const_cast<DetailHierarchySelectionController*>(&m_bookmarkSelectionController);
}

QObject* DetailPanelController::progressSelectionController() const noexcept
{
    return const_cast<DetailHierarchySelectionController*>(&m_progressSelectionController);
}

QObject* DetailPanelController::fileStatController() const noexcept
{
    return const_cast<DetailFileStatController*>(&m_fileStatController);
}

QObject* DetailPanelController::helpController() const noexcept
{
    return const_cast<DetailContentSectionController*>(&m_helpController);
}

QObject* DetailPanelController::propertiesController() const noexcept
{
    return const_cast<DetailPropertiesController*>(&m_propertiesController);
}

QVariantList DetailPanelController::toolbarItems() const
{
    return m_toolbarItems;
}

void DetailPanelController::setActiveState(int stateValue)
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        const QString detail = QStringLiteral("requestedStateValue=%1 result=ignored_invalid").arg(stateValue);
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.panel.controller"),
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
            QStringLiteral("detail.panel.controller"),
            QStringLiteral("setActiveState.ignoredSame"),
            detail);
        return;
    }

    m_activeState = nextState;
    applyActiveContentController(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("requestedStateValue=%1 previousState=%2 nextState=%3 toolbarItemCount=%4")
                           .arg(stateValue)
                           .arg(previousStateName)
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.controller"),
        QStringLiteral("setActiveState.applied"),
        detail);
    emit activeStateChanged();
    emit toolbarItemsChanged();
}

void DetailPanelController::requestStateChange(int stateValue)
{
    const QString detail = QStringLiteral("requestedStateValue=%1").arg(stateValue);
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.controller"),
        QStringLiteral("requestStateChange"),
        detail);
    setActiveState(stateValue);
}

void DetailPanelController::setProjectSelectionSourceController(QObject* sourceController)
{
    m_projectSelectionSourceController.setOptionsSourceController(sourceController);
}

void DetailPanelController::setBookmarkSelectionSourceController(QObject* sourceController)
{
    m_bookmarkSelectionSourceController.setOptionsSourceController(sourceController);
}

void DetailPanelController::setProgressSelectionSourceController(QObject* sourceController)
{
    m_progressSelectionSourceController.setOptionsSourceController(sourceController);
}

void DetailPanelController::setTagsSourceController(QObject* sourceController)
{
    m_tagsSourceController = sourceController;
}

void DetailPanelController::setCurrentNoteListModel(QObject* noteListModel)
{
    m_currentNoteContextBridge.setNoteListModel(noteListModel);
    reconnectCurrentNoteListModelSignals(noteListModel);
}

void DetailPanelController::setCurrentNoteDirectorySourceController(QObject* sourceController)
{
    m_currentNoteContextBridge.setNoteDirectorySourceController(sourceController);
}

bool DetailPanelController::writeProjectSelection(int index)
{
    return writeSelectionIndex(
        m_projectSelectionSourceController,
        index,
        DetailNoteHeaderSelectionSourceController::Field::Project);
}

bool DetailPanelController::writeBookmarkSelection(int index)
{
    return writeSelectionIndex(
        m_bookmarkSelectionSourceController,
        index,
        DetailNoteHeaderSelectionSourceController::Field::Bookmark);
}

bool DetailPanelController::writeProgressSelection(int index)
{
    return writeSelectionIndex(
        m_progressSelectionSourceController,
        index,
        DetailNoteHeaderSelectionSourceController::Field::Progress);
}

bool DetailPanelController::assignFolderByName(const QString& folderPath)
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
    m_propertiesController.applyHeader(header);
    const int activeFolderIndex = indexOfFolderPath(header, resolution.folderPath);
    if (activeFolderIndex >= 0)
    {
        m_propertiesController.setActiveFolderIndex(activeFolderIndex);
    }
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

bool DetailPanelController::assignTagByName(const QString& tag)
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
    m_propertiesController.applyHeader(header);
    const int activeTagIndex = indexOfTag(header, normalizedTag);
    if (activeTagIndex >= 0)
    {
        m_propertiesController.setActiveTagIndex(activeTagIndex);
    }
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

void DetailPanelController::handleCurrentNoteItemsChanged()
{
    reloadCurrentHeader(true);
}

bool DetailPanelController::removeActiveFolder()
{
    return removeMetadataEntry(true);
}

bool DetailPanelController::removeActiveTag()
{
    return removeMetadataEntry(false);
}

bool DetailPanelController::ensureCurrentHeaderLoaded(QString* errorMessage)
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

QString DetailPanelController::currentNoteId() const
{
    return m_currentNoteContextBridge.currentNoteId().trimmed();
}

QString DetailPanelController::currentNoteDirectoryPath() const
{
    return m_currentNoteContextBridge.currentNoteDirectoryPath().trimmed();
}

bool DetailPanelController::writeSelectionIndex(
    const DetailNoteHeaderSelectionSourceController& selectionSourceController,
    int index,
    DetailNoteHeaderSelectionSourceController::Field field)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QVariantList hierarchyModel = selectionSourceController.hierarchyModel();
    if (index < 0 || index >= hierarchyModel.size())
    {
        return false;
    }
    if (index == selectionSourceController.selectedIndex())
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
    case DetailNoteHeaderSelectionSourceController::Field::Project:
        saved = m_noteHeaderSessionStore.updateProject(
            noteId,
            clearSelection ? QString() : label,
            &errorMessage);
        break;
    case DetailNoteHeaderSelectionSourceController::Field::Bookmark:
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
    case DetailNoteHeaderSelectionSourceController::Field::Progress:
    {
        const int progress = DetailNoteHeaderSelectionSourceController::progressValueForHierarchyEntry(
            hierarchyModel.at(index),
            index);
        saved = m_noteHeaderSessionStore.updateProgress(noteId, progress, &errorMessage);
        break;
    }
    }

    if (saved)
    {
        m_propertiesController.applyHeader(m_noteHeaderSessionStore.header(noteId));
        m_projectSelectionSourceController.synchronize(false);
        m_bookmarkSelectionSourceController.synchronize(false);
        m_progressSelectionSourceController.synchronize(false);
        synchronizeCurrentNoteMetadataConsumers(noteId);
    }
    return saved;
}

bool DetailPanelController::removeMetadataEntry(const bool removeFolder)
{
    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QString noteId = currentNoteId();
    const int activeIndex = removeFolder
        ? m_propertiesController.activeFolderIndex()
        : m_propertiesController.activeTagIndex();
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

    m_propertiesController.applyHeader(m_noteHeaderSessionStore.header(noteId));
    synchronizeCurrentNoteMetadataConsumers(noteId);
    return true;
}

void DetailPanelController::applyActiveContentController(DetailContentState activeState)
{
    m_propertiesController.setActive(activeState == DetailContentState::Properties);
    m_fileStatController.setActive(activeState == DetailContentState::FileStat);
    m_insertController.setActive(activeState == DetailContentState::Insert);
    m_fileHistoryController.setActive(activeState == DetailContentState::FileHistory);
    m_layerController.setActive(activeState == DetailContentState::Layer);
    m_helpController.setActive(activeState == DetailContentState::Help);
    m_activeContentController = contentControllerForState(WhatSon::DetailPanel::stateValue(activeState));
}

void DetailPanelController::setNoteContextLinked(const bool linked)
{
    if (m_noteContextLinked == linked)
    {
        return;
    }

    m_noteContextLinked = linked;
    emit noteContextLinkedChanged();
}

void DetailPanelController::reconnectCurrentNoteListModelSignals(QObject* noteListModel)
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

void DetailPanelController::disconnectCurrentNoteListModelSignals()
{
    if (m_currentNoteListItemsChangedConnection)
    {
        disconnect(m_currentNoteListItemsChangedConnection);
        m_currentNoteListItemsChangedConnection = {};
    }
}

void DetailPanelController::reloadCurrentHeader(const bool forceReload)
{
    const QString noteId = m_currentNoteContextBridge.currentNoteId();
    const QString noteDirectoryPath = m_currentNoteContextBridge.currentNoteDirectoryPath();
    bool linkedContext = false;
    QString loadError;
    if (!noteId.isEmpty()
        && !noteDirectoryPath.isEmpty()
        && m_noteHeaderSessionStore.ensureLoaded(noteId, noteDirectoryPath, &loadError, forceReload))
    {
        const WhatSonNoteHeaderStore header = m_noteHeaderSessionStore.header(noteId);
        m_propertiesController.applyHeader(header);
        m_fileStatController.applyHeader(header);
        m_projectSelectionSourceController.synchronize(false);
        m_bookmarkSelectionSourceController.synchronize(false);
        m_progressSelectionSourceController.synchronize(false);
        linkedContext = true;
    }
    else
    {
        m_propertiesController.clearHeader();
        m_fileStatController.clearHeader();
    }

    setNoteContextLinked(linkedContext);
}

void DetailPanelController::synchronizeCurrentNoteMetadataConsumers(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return;
    }

    QSet<QObject*> synchronizedConsumers;
    auto synchronizeConsumer = [&synchronizedConsumers, &normalizedNoteId](QObject* sourceController)
    {
        if (sourceController == nullptr || synchronizedConsumers.contains(sourceController))
        {
            return;
        }

        const QMetaObject* metaObject = sourceController->metaObject();
        if (metaObject == nullptr || metaObject->indexOfMethod("reloadNoteMetadataForNoteId(QString)") < 0)
        {
            return;
        }

        bool synchronized = false;
        QMetaObject::invokeMethod(
            sourceController,
            "reloadNoteMetadataForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, synchronized),
            Q_ARG(QString, normalizedNoteId));
        synchronizedConsumers.insert(sourceController);
    };

    synchronizeConsumer(m_currentNoteContextBridge.noteDirectorySourceController());
    synchronizeConsumer(m_projectSelectionSourceController.optionsSourceController());
    synchronizeConsumer(m_bookmarkSelectionSourceController.optionsSourceController());
    synchronizeConsumer(m_progressSelectionSourceController.optionsSourceController());
    synchronizeConsumer(m_tagsSourceController);
}
