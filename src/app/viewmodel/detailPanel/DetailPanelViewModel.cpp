#include "DetailPanelViewModel.hpp"

#include "DetailPanelToolbarItemsFactory.hpp"
#include "file/WhatSonDebugTrace.hpp"

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
      , m_propertiesViewModel(this)
      , m_fileStatViewModel(DetailContentState::FileStat, this)
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

                         const QString noteId = m_currentNoteContextBridge.currentNoteId();
                         QString loadError;
                         if (!noteId.isEmpty()
                             && !noteDirectoryPath.isEmpty()
                             && m_noteHeaderSessionStore.ensureLoaded(noteId, noteDirectoryPath, &loadError))
                         {
                             m_propertiesViewModel.applyHeader(m_noteHeaderSessionStore.header(noteId));
                             m_projectSelectionSourceViewModel.synchronize(false);
                             m_bookmarkSelectionSourceViewModel.synchronize(false);
                             m_progressSelectionSourceViewModel.synchronize(false);
                         }
                         else
                         {
                             m_propertiesViewModel.clearHeader();
                         }
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
        return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
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
    return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
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

void DetailPanelViewModel::setCurrentNoteListModel(QObject* noteListModel)
{
    m_currentNoteContextBridge.setNoteListModel(noteListModel);
}

void DetailPanelViewModel::setCurrentNoteDirectorySourceViewModel(QObject* sourceViewModel)
{
    m_currentNoteContextBridge.setNoteDirectorySourceViewModel(sourceViewModel);
}

bool DetailPanelViewModel::writeProjectSelection(int index)
{
    return writeSelectionIndex(m_projectSelectionSourceViewModel.optionsSourceViewModel(), index, DetailNoteHeaderSelectionSourceViewModel::Field::Project);
}

bool DetailPanelViewModel::writeBookmarkSelection(int index)
{
    return writeSelectionIndex(m_bookmarkSelectionSourceViewModel.optionsSourceViewModel(), index, DetailNoteHeaderSelectionSourceViewModel::Field::Bookmark);
}

bool DetailPanelViewModel::writeProgressSelection(int index)
{
    return writeSelectionIndex(m_progressSelectionSourceViewModel.optionsSourceViewModel(), index, DetailNoteHeaderSelectionSourceViewModel::Field::Progress);
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

bool DetailPanelViewModel::writeSelectionIndex(QObject* optionsSourceViewModel, int index, DetailNoteHeaderSelectionSourceViewModel::Field field)
{
    if (optionsSourceViewModel == nullptr)
    {
        return false;
    }

    QString errorMessage;
    if (!ensureCurrentHeaderLoaded(&errorMessage))
    {
        return false;
    }

    const QVariantList hierarchyModel = optionsSourceViewModel->property("hierarchyModel").toList();
    if (index < 0 || index >= hierarchyModel.size())
    {
        return false;
    }

    const QString label = hierarchyModel.at(index).toMap().value(QStringLiteral("label")).toString().trimmed();
    const QString noteId = currentNoteId();

    bool saved = false;
    switch (field)
    {
    case DetailNoteHeaderSelectionSourceViewModel::Field::Project:
        saved = m_noteHeaderSessionStore.updateProject(noteId, label, &errorMessage);
        break;
    case DetailNoteHeaderSelectionSourceViewModel::Field::Bookmark:
        saved = m_noteHeaderSessionStore.updateBookmarked(
            noteId,
            !label.isEmpty(),
            label.isEmpty() ? QStringList{} : QStringList{label.toLower()},
            &errorMessage);
        break;
    case DetailNoteHeaderSelectionSourceViewModel::Field::Progress:
    {
        int progress = 0;
        if (label.compare(QStringLiteral("Pending"), Qt::CaseInsensitive) == 0)
            progress = 1;
        else if (label.compare(QStringLiteral("InProgress"), Qt::CaseInsensitive) == 0)
            progress = 2;
        else if (label.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0)
            progress = 3;
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
    }
    return saved;
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
