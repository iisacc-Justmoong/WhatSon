#include "ContentsEditorSelectionBridge.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/sync/ContentsEditorIdleSyncController.hpp"

#include <QMetaProperty>
#include <QQmlEngine>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kNoteBodySourceTextForNoteIdSignature = "noteBodySourceTextForNoteId(QString)";

    bool noteBackedSelectionEnabled(const QObject* noteListModel)
    {
        if (noteListModel == nullptr)
        {
            return false;
        }

        const QMetaObject* metaObject = noteListModel->metaObject();
        if (metaObject == nullptr)
        {
            return false;
        }

        const int propertyIndex = metaObject->indexOfProperty("noteBacked");
        if (propertyIndex < 0)
        {
            return true;
        }

        const QMetaProperty property = metaObject->property(propertyIndex);
        if (!property.isReadable())
        {
            return true;
        }

        return property.read(noteListModel).toBool();
    }

    void stabilizeQmlBindingOwnership(QObject* object)
    {
        if (object != nullptr)
        {
            QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        }
    }
}

ContentsEditorSelectionBridge::ContentsEditorSelectionBridge(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("selectionBridge"), QStringLiteral("ctor"));
    m_idleSyncController = new ContentsEditorIdleSyncController(this);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorSelectionBridge::contentPersistenceContractAvailableChanged);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::editorTextPersistenceQueued,
        this,
        &ContentsEditorSelectionBridge::editorTextPersistenceQueued);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::editorTextPersistenceFinished,
        this,
        &ContentsEditorSelectionBridge::editorTextPersistenceFinished);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::editorTextPersistenceFinished,
        this,
        &ContentsEditorSelectionBridge::handleEditorTextPersistenceFinishedInternal);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::noteBodyTextLoaded,
        this,
        &ContentsEditorSelectionBridge::handleNoteBodyTextLoaded);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::viewSessionSnapshotReconciled);
    connect(
        m_idleSyncController,
        &ContentsEditorIdleSyncController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::handleViewSessionSnapshotReconciledInternal);
}

ContentsEditorSelectionBridge::~ContentsEditorSelectionBridge()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 loading=%3")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyLoading));
}

QObject* ContentsEditorSelectionBridge::noteListModel() const noexcept
{
    return m_noteListModel;
}

void ContentsEditorSelectionBridge::setNoteListModel(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setNoteListModel"),
        QStringLiteral("previous=0x%1 next=0x%2")
            .arg(QString::number(reinterpret_cast<quintptr>(m_noteListModel.data()), 16))
            .arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    if (m_noteListModel == model)
    {
        return;
    }

    disconnectNoteListModel();
    stabilizeQmlBindingOwnership(model);
    m_noteListModel = model;

    if (m_noteListModel != nullptr)
    {
        m_noteListDestroyedConnection = connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            &ContentsEditorSelectionBridge::handleNoteListDestroyed);
        m_currentNoteIdChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentNoteIdChanged()),
            this,
            SLOT(handleNoteListSelectionChanged()));
        m_itemCountChangedConnection = connect(
            m_noteListModel,
            SIGNAL(itemCountChanged(int)),
            this,
            SLOT(handleNoteListCountChanged()));
    }

    emit noteListModelChanged();
    m_noteSelectionRefreshRequiresRebind = true;
    scheduleNoteSelectionRefresh();
    refreshNoteCountState();
}

QObject* ContentsEditorSelectionBridge::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

void ContentsEditorSelectionBridge::setContentViewModel(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setContentViewModel"),
        QStringLiteral("previous=0x%1 next=0x%2")
            .arg(QString::number(reinterpret_cast<quintptr>(m_contentViewModel.data()), 16))
            .arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    if (m_contentViewModel == model)
    {
        return;
    }

    disconnectContentViewModel();
    stabilizeQmlBindingOwnership(model);
    m_contentViewModel = model;

    if (m_contentViewModel != nullptr)
    {
        m_contentViewModelDestroyedConnection = connect(
            m_contentViewModel,
            &QObject::destroyed,
            this,
            &ContentsEditorSelectionBridge::handleContentViewModelDestroyed);
    }

    if (m_idleSyncController != nullptr)
    {
        m_idleSyncController->setContentViewModel(m_contentViewModel);
    }

    emit contentViewModelChanged();
    m_noteSelectionRefreshRequiresRebind = true;
    scheduleNoteSelectionRefresh();
}

bool ContentsEditorSelectionBridge::noteSelectionContractAvailable() const noexcept
{
    return m_noteSelectionContractAvailable;
}

bool ContentsEditorSelectionBridge::noteCountContractAvailable() const noexcept
{
    return m_noteCountContractAvailable;
}

bool ContentsEditorSelectionBridge::contentPersistenceContractAvailable() const noexcept
{
    return m_idleSyncController != nullptr
        && m_idleSyncController->contentPersistenceContractAvailable();
}

QString ContentsEditorSelectionBridge::selectedNoteId() const
{
    return m_selectedNoteId;
}

QString ContentsEditorSelectionBridge::selectedNoteDirectoryPath() const
{
    return m_selectedNoteDirectoryPath;
}

QString ContentsEditorSelectionBridge::selectedNoteBodyNoteId() const
{
    return m_selectedNoteBodyNoteId;
}

QString ContentsEditorSelectionBridge::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

bool ContentsEditorSelectionBridge::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

int ContentsEditorSelectionBridge::visibleNoteCount() const noexcept
{
    return m_visibleNoteCount;
}

bool ContentsEditorSelectionBridge::persistEditorTextForNote(const QString& noteId, const QString& text)
{
    return stageEditorTextForIdleSync(noteId, text);
}

bool ContentsEditorSelectionBridge::stageEditorTextForIdleSync(const QString& noteId, const QString& text)
{
    const bool accepted = m_idleSyncController != nullptr
        && m_idleSyncController->stageEditorTextForIdleSync(noteId, text);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("stageEditorTextForIdleSync"),
        QStringLiteral("accepted=%1 noteId=%2 %3")
            .arg(accepted)
            .arg(noteId.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));
    return accepted;
}

bool ContentsEditorSelectionBridge::flushEditorTextForNote(const QString& noteId, const QString& text)
{
    const bool accepted = m_idleSyncController != nullptr
        && m_idleSyncController->flushEditorTextForNote(noteId, text);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("flushEditorTextForNote"),
        QStringLiteral("accepted=%1 noteId=%2 %3")
            .arg(accepted)
            .arg(noteId.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));
    return accepted;
}

bool ContentsEditorSelectionBridge::reconcileViewSessionAndRefreshSnapshotForNote(
    const QString& noteId,
    const QString& viewSessionText,
    const bool preferViewSessionOnMismatch)
{
    const QString normalizedNoteId = noteId.trimmed();
    const bool accepted = m_idleSyncController != nullptr
        && !normalizedNoteId.isEmpty()
        && m_idleSyncController->reconcileViewSessionAndRefreshSnapshotForNote(
            normalizedNoteId,
            viewSessionText,
            preferViewSessionOnMismatch);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("reconcileViewSessionAndRefreshSnapshotForNote"),
        QStringLiteral("accepted=%1 preferViewSession=%2 noteId=%3 %4")
            .arg(accepted)
            .arg(preferViewSessionOnMismatch)
            .arg(normalizedNoteId)
            .arg(WhatSon::Debug::summarizeText(viewSessionText)));
    return accepted;
}

bool ContentsEditorSelectionBridge::directPersistenceAvailable() const noexcept
{
    return m_idleSyncController != nullptr
        && m_idleSyncController->directPersistenceAvailable();
}

bool ContentsEditorSelectionBridge::refreshSelectedNoteSnapshot()
{
    const bool reloaded = m_idleSyncController != nullptr
        && m_idleSyncController->refreshNoteSnapshotForNote(m_selectedNoteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("refreshSelectedNoteSnapshot"),
        QStringLiteral("reloaded=%1 selectedNoteId=%2").arg(reloaded).arg(m_selectedNoteId));
    if (reloaded)
    {
        startSelectedNoteBodyLoad(m_selectedNoteId, false);
    }
    scheduleNoteSelectionRefresh();
    refreshNoteCountState();
    return reloaded;
}

void ContentsEditorSelectionBridge::handleNoteListSelectionChanged()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleNoteListSelectionChanged"),
        QStringLiteral("noteListModel=0x%1")
            .arg(QString::number(reinterpret_cast<quintptr>(m_noteListModel.data()), 16)));
    scheduleNoteSelectionRefresh();
}

void ContentsEditorSelectionBridge::handleEditorTextPersistenceFinishedInternal(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleEditorTextPersistenceFinishedInternal"),
        QStringLiteral("success=%1 noteId=%2 %3")
            .arg(success)
            .arg(noteId.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));

    if (!success)
    {
        return;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return;
    }

    const QString selectedNoteId = m_selectedNoteId.trimmed();
    const QString selectedBodyNoteId = m_selectedNoteBodyNoteId.trimmed();
    if (normalizedNoteId != selectedNoteId && normalizedNoteId != selectedBodyNoteId)
    {
        return;
    }

    if (normalizedNoteId == selectedNoteId)
    {
        setSelectedNoteDirectoryPath(resolveSelectedNoteDirectoryPath(normalizedNoteId));
    }

    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    m_selectedNoteBodyRequestSequence = 0;
    setSelectedNoteBodyState(normalizedNoteId, text, false);
}

void ContentsEditorSelectionBridge::handleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleNoteBodyTextLoaded"),
        QStringLiteral("sequence=%1 success=%2 noteId=%3 %4")
            .arg(sequence)
            .arg(success)
            .arg(noteId.trimmed())
            .arg(WhatSon::Debug::summarizeText(text)));

    const QString normalizedNoteId = noteId.trimmed();
    if (sequence == 0
        || normalizedNoteId.isEmpty()
        || normalizedNoteId != m_selectedNoteId.trimmed()
        || sequence != m_selectedNoteBodyRequestSequence)
    {
        return;
    }

    m_selectedNoteBodyRequestSequence = 0;
    if (adoptPendingEditorBodyText(normalizedNoteId))
    {
        return;
    }

    if (!success)
    {
        m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
        QString fallbackText;
        if (!tryResolveSelectedNoteBodySourceText(normalizedNoteId, &fallbackText))
        {
            fallbackText = m_selectedNoteBodyNoteId == normalizedNoteId
                ? m_selectedNoteBodyText
                : QString();
        }
        setSelectedNoteBodyState(normalizedNoteId, fallbackText, false);
        return;
    }

    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    setSelectedNoteBodyState(normalizedNoteId, text, false);
}

void ContentsEditorSelectionBridge::handleViewSessionSnapshotReconciledInternal(
    const QString& noteId,
    const bool refreshed,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleViewSessionSnapshotReconciledInternal"),
        QStringLiteral("success=%1 refreshed=%2 noteId=%3")
            .arg(success)
            .arg(refreshed)
            .arg(noteId.trimmed()));

    const QString normalizedNoteId = noteId.trimmed();
    if (!success || !refreshed || normalizedNoteId.isEmpty()
        || normalizedNoteId != m_selectedNoteId.trimmed())
    {
        return;
    }

    startSelectedNoteBodyLoad(normalizedNoteId, false);
}

void ContentsEditorSelectionBridge::flushPendingNoteSelectionRefresh()
{
    m_noteSelectionRefreshQueued = false;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("flushPendingNoteSelectionRefresh"),
        QStringLiteral("selectedNoteId=%1").arg(m_selectedNoteId));
    refreshNoteSelectionState();
}

void ContentsEditorSelectionBridge::handleNoteListCountChanged()
{
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleNoteListDestroyed()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleNoteListDestroyed"));
    disconnectNoteListModel();
    m_noteListModel = nullptr;
    emit noteListModelChanged();
    m_noteSelectionRefreshQueued = false;
    m_selectedNoteBodySnapshotNoteId.clear();
    m_selectedNoteBodyRequestSequence = 0;
    refreshNoteSelectionState();
    refreshNoteCountState();
}

void ContentsEditorSelectionBridge::handleContentViewModelDestroyed()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleContentViewModelDestroyed"));
    disconnectContentViewModel();
    m_contentViewModel = nullptr;
    if (m_idleSyncController != nullptr)
    {
        m_idleSyncController->setContentViewModel(nullptr);
    }
    m_selectedNoteBodyRequestSequence = 0;
    emit contentViewModelChanged();
}

bool ContentsEditorSelectionBridge::hasReadableProperty(const QObject* object, const char* propertyName)
{
    if (object == nullptr || propertyName == nullptr)
    {
        return false;
    }

    const int propertyIndex = object->metaObject()->indexOfProperty(propertyName);
    if (propertyIndex < 0)
    {
        return false;
    }

    return object->metaObject()->property(propertyIndex).isReadable();
}

bool ContentsEditorSelectionBridge::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString ContentsEditorSelectionBridge::readStringProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return {};
    }

    return object->property(propertyName).toString();
}

int ContentsEditorSelectionBridge::readIntProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return 0;
    }

    return std::max(0, object->property(propertyName).toInt());
}

bool ContentsEditorSelectionBridge::adoptPendingEditorBodyText(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || m_idleSyncController == nullptr)
    {
        return false;
    }

    QString pendingEditorText;
    if (!m_idleSyncController->pendingEditorTextForNote(normalizedNoteId, &pendingEditorText))
    {
        return false;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("adoptPendingEditorBodyText"),
        QStringLiteral("noteId=%1 %2")
            .arg(normalizedNoteId)
            .arg(WhatSon::Debug::summarizeText(pendingEditorText)));

    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    m_selectedNoteBodyRequestSequence = 0;
    setSelectedNoteBodyState(normalizedNoteId, pendingEditorText, false);
    return true;
}

QString ContentsEditorSelectionBridge::resolveSelectedNoteDirectoryPath(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || m_idleSyncController == nullptr)
    {
        return {};
    }
    return m_idleSyncController->noteDirectoryPathForNote(normalizedNoteId).trimmed();
}

bool ContentsEditorSelectionBridge::tryResolveSelectedNoteBodySourceText(
    const QString& noteId,
    QString* bodyText) const
{
    if (bodyText != nullptr)
    {
        bodyText->clear();
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty()
        || m_contentViewModel == nullptr
        || !hasInvokableMethod(m_contentViewModel, kNoteBodySourceTextForNoteIdSignature))
    {
        return false;
    }

    QString resolvedBodyText;
    if (!QMetaObject::invokeMethod(
            m_contentViewModel,
            "noteBodySourceTextForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, resolvedBodyText),
            Q_ARG(QString, normalizedNoteId)))
    {
        return false;
    }

    if (bodyText != nullptr)
    {
        *bodyText = resolvedBodyText;
    }

    return true;
}

void ContentsEditorSelectionBridge::setSelectedNoteDirectoryPath(QString noteDirectoryPath)
{
    noteDirectoryPath = noteDirectoryPath.trimmed();
    if (m_selectedNoteDirectoryPath == noteDirectoryPath)
    {
        return;
    }
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setSelectedNoteDirectoryPath"),
        QStringLiteral("previous=%1 next=%2")
            .arg(m_selectedNoteDirectoryPath)
            .arg(noteDirectoryPath));
    m_selectedNoteDirectoryPath = std::move(noteDirectoryPath);
    emit selectedNoteDirectoryPathChanged();
}

void ContentsEditorSelectionBridge::setSelectedNoteBodyState(
    QString noteId,
    QString bodyText,
    const bool loading)
{
    const bool noteIdChanged = m_selectedNoteBodyNoteId != noteId;
    const bool loadingChanged = m_selectedNoteBodyLoading != loading;
    const bool bodyTextChanged = m_selectedNoteBodyText != bodyText;

    m_selectedNoteBodyNoteId = std::move(noteId);
    m_selectedNoteBodyLoading = loading;
    m_selectedNoteBodyText = std::move(bodyText);

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setSelectedNoteBodyState"),
        QStringLiteral("noteId=%1 loading=%2 %3")
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyLoading)
            .arg(WhatSon::Debug::summarizeText(m_selectedNoteBodyText)));

    if (noteIdChanged)
    {
        emit selectedNoteBodyNoteIdChanged();
    }
    if (loadingChanged)
    {
        emit selectedNoteBodyLoadingChanged();
    }
    if (bodyTextChanged)
    {
        emit selectedNoteBodyTextChanged();
    }
}

void ContentsEditorSelectionBridge::startSelectedNoteBodyLoad(
    const QString& noteId,
    const bool clearCachedBody)
{
    const QString normalizedNoteId = noteId.trimmed();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("startSelectedNoteBodyLoad"),
        QStringLiteral("noteId=%1 clearCachedBody=%2").arg(normalizedNoteId).arg(clearCachedBody));
    if (normalizedNoteId.isEmpty())
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), QString(), false);
        return;
    }

    if (clearCachedBody)
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
    }

    if (adoptPendingEditorBodyText(normalizedNoteId))
    {
        return;
    }

    const bool noteBodyOwnedBySelection = m_selectedNoteBodyNoteId == normalizedNoteId;
    const QString currentBodyText = clearCachedBody || !noteBodyOwnedBySelection
        ? QString()
        : m_selectedNoteBodyText;
    const bool shouldShowLoading = clearCachedBody
        || !noteBodyOwnedBySelection
        || (currentBodyText.isEmpty() && m_selectedNoteBodySnapshotNoteId != normalizedNoteId);
    setSelectedNoteBodyState(normalizedNoteId, currentBodyText, shouldShowLoading);

    const quint64 requestSequence = m_idleSyncController != nullptr
        ? m_idleSyncController->loadNoteBodyTextForNote(normalizedNoteId)
        : 0;
    if (requestSequence != 0)
    {
        m_selectedNoteBodyRequestSequence = requestSequence;
        return;
    }

    QString fallbackBodyText;
    if (!tryResolveSelectedNoteBodySourceText(normalizedNoteId, &fallbackBodyText))
    {
        fallbackBodyText = currentBodyText;
    }

    m_selectedNoteBodyRequestSequence = 0;
    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    setSelectedNoteBodyState(normalizedNoteId, fallbackBodyText, false);
}

void ContentsEditorSelectionBridge::scheduleNoteSelectionRefresh()
{
    if (m_noteSelectionRefreshQueued)
    {
        return;
    }

    m_noteSelectionRefreshQueued = true;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("scheduleNoteSelectionRefresh"),
        QStringLiteral("selectedNoteId=%1").arg(m_selectedNoteId));
    QMetaObject::invokeMethod(
        this,
        &ContentsEditorSelectionBridge::flushPendingNoteSelectionRefresh,
        Qt::QueuedConnection);
}

void ContentsEditorSelectionBridge::refreshNoteSelectionState()
{
    const bool nextContractAvailable = noteBackedSelectionEnabled(m_noteListModel)
        && hasReadableProperty(m_noteListModel, "currentNoteId");
    const QString nextNoteId = nextContractAvailable ? readStringProperty(m_noteListModel, "currentNoteId") : QString();
    const bool requiresRebind = m_noteSelectionRefreshRequiresRebind;
    m_noteSelectionRefreshRequiresRebind = false;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("refreshNoteSelectionState"),
        QStringLiteral("contract=%1 previousNoteId=%2 nextNoteId=%3 requiresRebind=%4")
            .arg(nextContractAvailable)
            .arg(m_selectedNoteId)
            .arg(nextNoteId)
            .arg(requiresRebind));

    if (m_noteSelectionContractAvailable != nextContractAvailable)
    {
        m_noteSelectionContractAvailable = nextContractAvailable;
        emit noteSelectionContractAvailableChanged();
    }
    if (m_idleSyncController != nullptr
        && (m_selectedNoteId != nextNoteId || requiresRebind))
    {
        if (nextNoteId.trimmed().isEmpty())
        {
            m_idleSyncController->clearSelectedNote();
        }
        else
        {
            m_idleSyncController->bindSelectedNote(nextNoteId);
        }
    }

    const bool noteIdChanged = m_selectedNoteId != nextNoteId;
    if (!noteIdChanged)
    {
        setSelectedNoteDirectoryPath(resolveSelectedNoteDirectoryPath(m_selectedNoteId));
        if (!m_selectedNoteId.trimmed().isEmpty()
            && (m_selectedNoteBodySnapshotNoteId != m_selectedNoteId || requiresRebind)
            && !m_selectedNoteBodyLoading)
        {
            startSelectedNoteBodyLoad(m_selectedNoteId, m_selectedNoteBodyText.isEmpty());
        }
        return;
    }

    m_selectedNoteId = nextNoteId;
    emit selectedNoteIdChanged();
    setSelectedNoteDirectoryPath(resolveSelectedNoteDirectoryPath(nextNoteId));

    if (nextNoteId.trimmed().isEmpty())
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), QString(), false);
        return;
    }

    startSelectedNoteBodyLoad(nextNoteId, true);
}

void ContentsEditorSelectionBridge::refreshNoteCountState()
{
    const bool nextContractAvailable = hasReadableProperty(m_noteListModel, "itemCount");
    const int nextVisibleNoteCount = nextContractAvailable ? readIntProperty(m_noteListModel, "itemCount") : 0;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("refreshNoteCountState"),
        QStringLiteral("contract=%1 visibleNoteCount=%2")
            .arg(nextContractAvailable)
            .arg(nextVisibleNoteCount));

    if (m_noteCountContractAvailable != nextContractAvailable)
    {
        m_noteCountContractAvailable = nextContractAvailable;
        emit noteCountContractAvailableChanged();
    }
    if (m_visibleNoteCount != nextVisibleNoteCount)
    {
        m_visibleNoteCount = nextVisibleNoteCount;
        emit visibleNoteCountChanged();
    }
}

void ContentsEditorSelectionBridge::disconnectNoteListModel()
{
    if (m_noteListDestroyedConnection)
    {
        disconnect(m_noteListDestroyedConnection);
        m_noteListDestroyedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteIdChangedConnection)
    {
        disconnect(m_currentNoteIdChangedConnection);
        m_currentNoteIdChangedConnection = QMetaObject::Connection();
    }
    if (m_itemCountChangedConnection)
    {
        disconnect(m_itemCountChangedConnection);
        m_itemCountChangedConnection = QMetaObject::Connection();
    }
}

void ContentsEditorSelectionBridge::disconnectContentViewModel()
{
    if (m_contentViewModelDestroyedConnection)
    {
        disconnect(m_contentViewModelDestroyedConnection);
        m_contentViewModelDestroyedConnection = QMetaObject::Connection();
    }
}
