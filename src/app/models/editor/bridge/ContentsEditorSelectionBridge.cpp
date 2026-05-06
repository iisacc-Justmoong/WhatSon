#include "app/models/editor/bridge/ContentsEditorSelectionBridge.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/editor/persistence/ContentsEditorPersistenceController.hpp"

#include <QAbstractItemModel>
#include <QDir>
#include <QMetaProperty>
#include <QQmlEngine>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";
    constexpr auto kNoteBodySourceTextForNoteIdSignature = "noteBodySourceTextForNoteId(QString)";

    bool noteBackedSelectionEnabled(const QObject* noteListModel);
    bool hasReadableContractProperty(const QObject* object, const char* propertyName);
    QVariantMap readCurrentNoteEntry(const QObject* noteListModel);
    QString noteIdFromEntry(const QVariantMap& noteEntry);
    QString noteDirectoryPathFromEntry(const QVariantMap& noteEntry);

    QString readTraceStringProperty(const QObject* object, const char* propertyName)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return {};
        }

        const QMetaObject* metaObject = object->metaObject();
        if (metaObject == nullptr)
        {
            return {};
        }

        const int propertyIndex = metaObject->indexOfProperty(propertyName);
        if (propertyIndex < 0)
        {
            return {};
        }

        const QMetaProperty property = metaObject->property(propertyIndex);
        if (!property.isReadable())
        {
            return {};
        }

        return property.read(object).toString();
    }

    int readTraceIntProperty(const QObject* object, const char* propertyName)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return 0;
        }

        const QMetaObject* metaObject = object->metaObject();
        if (metaObject == nullptr)
        {
            return 0;
        }

        const int propertyIndex = metaObject->indexOfProperty(propertyName);
        if (propertyIndex < 0)
        {
            return 0;
        }

        const QMetaProperty property = metaObject->property(propertyIndex);
        if (!property.isReadable())
        {
            return 0;
        }

        return std::max(0, property.read(object).toInt());
    }

    QString summarizeTraceNoteListModel(const QObject* noteListModel)
    {
        return QStringLiteral(
                   "noteListModel=0x%1 noteBacked=%2 currentIndex=%3 itemCount=%4 currentNoteId=%5 currentBodyText={%6} currentNoteDirectoryPath=%7")
            .arg(QString::number(reinterpret_cast<quintptr>(noteListModel), 16))
            .arg(noteBackedSelectionEnabled(noteListModel))
            .arg(readTraceIntProperty(noteListModel, "currentIndex"))
            .arg(readTraceIntProperty(noteListModel, "itemCount"))
            .arg(readTraceStringProperty(noteListModel, "currentNoteId").trimmed())
            .arg(WhatSon::Debug::summarizeText(readTraceStringProperty(noteListModel, "currentBodyText"), 48))
            .arg(readTraceStringProperty(noteListModel, "currentNoteDirectoryPath").trimmed());
    }

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

    bool hasReadableContractProperty(const QObject* object, const char* propertyName)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return false;
        }

        const QMetaObject* metaObject = object->metaObject();
        if (metaObject == nullptr)
        {
            return false;
        }

        const int propertyIndex = metaObject->indexOfProperty(propertyName);
        if (propertyIndex < 0)
        {
            return false;
        }

        return metaObject->property(propertyIndex).isReadable();
    }

    QVariantMap readCurrentNoteEntry(const QObject* noteListModel)
    {
        if (!hasReadableContractProperty(noteListModel, "currentNoteEntry"))
        {
            return {};
        }

        QVariantMap noteEntry = noteListModel->property("currentNoteEntry").toMap();
        if (!noteEntry.contains(QStringLiteral("noteId")) && noteEntry.contains(QStringLiteral("id")))
        {
            noteEntry.insert(QStringLiteral("noteId"), noteEntry.value(QStringLiteral("id")));
        }
        return noteEntry;
    }

    QString noteIdFromEntry(const QVariantMap& noteEntry)
    {
        const QString noteId = noteEntry.value(QStringLiteral("noteId")).toString().trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        return noteEntry.value(QStringLiteral("id")).toString().trimmed();
    }

    QString noteDirectoryPathFromEntry(const QVariantMap& noteEntry)
    {
        return noteEntry.value(QStringLiteral("noteDirectoryPath")).toString().trimmed();
    }

    void stabilizeQmlBindingOwnership(QObject* object)
    {
        if (object != nullptr)
        {
            QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        }
    }

    QMetaObject::Connection connectIfSignalExists(
        QObject* sender,
        const char* signalSignature,
        QObject* receiver,
        const char* slotSignature)
    {
        if (sender == nullptr || receiver == nullptr || signalSignature == nullptr || slotSignature == nullptr)
        {
            return {};
        }

        const QMetaObject* senderMetaObject = sender->metaObject();
        const QMetaObject* receiverMetaObject = receiver->metaObject();
        if (senderMetaObject == nullptr || receiverMetaObject == nullptr)
        {
            return {};
        }

        const QByteArray normalizedSignalSignature = QMetaObject::normalizedSignature(signalSignature);
        const QByteArray normalizedSlotSignature = QMetaObject::normalizedSignature(slotSignature);
        const int signalIndex = senderMetaObject->indexOfSignal(normalizedSignalSignature.constData());
        const int slotIndex = receiverMetaObject->indexOfSlot(normalizedSlotSignature.constData());
        if (signalIndex < 0 || slotIndex < 0)
        {
            return {};
        }

        QByteArray encodedSignalSignature("2");
        encodedSignalSignature.append(normalizedSignalSignature);
        QByteArray encodedSlotSignature("1");
        encodedSlotSignature.append(normalizedSlotSignature);
        return QObject::connect(
            sender,
            encodedSignalSignature.constData(),
            receiver,
            encodedSlotSignature.constData(),
            Qt::UniqueConnection);
    }
}

ContentsEditorSelectionBridge::ContentsEditorSelectionBridge(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("selectionBridge"), QStringLiteral("ctor"));
    m_persistenceController = new ContentsEditorPersistenceController(this);
    connect(
        m_persistenceController,
        &ContentsEditorPersistenceController::contentPersistenceContractAvailableChanged,
        this,
        &ContentsEditorSelectionBridge::contentPersistenceContractAvailableChanged);
    connect(
        m_persistenceController,
        &ContentsEditorPersistenceController::noteBodyTextLoaded,
        this,
        &ContentsEditorSelectionBridge::handleNoteBodyTextLoaded);
    connect(
        m_persistenceController,
        &ContentsEditorPersistenceController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::viewSessionSnapshotReconciled);
    connect(
        m_persistenceController,
        &ContentsEditorPersistenceController::viewSessionSnapshotReconciled,
        this,
        &ContentsEditorSelectionBridge::handleViewSessionSnapshotReconciledInternal);
}

ContentsEditorSelectionBridge::~ContentsEditorSelectionBridge()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 resolved=%3 loading=%4")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
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
        QStringLiteral("previous=0x%1 next={%2}")
            .arg(QString::number(reinterpret_cast<quintptr>(m_noteListModel.data()), 16))
            .arg(summarizeTraceNoteListModel(model)));
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
        m_currentIndexChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "currentIndexChanged()",
            this,
            "handleNoteListSelectionChanged()");
        m_currentNoteIdChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "currentNoteIdChanged()",
            this,
            "handleNoteListSelectionChanged()");
        m_currentNoteDirectoryPathChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "currentNoteDirectoryPathChanged()",
            this,
            "handleNoteListSelectionChanged()");
        m_currentBodyTextChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "currentBodyTextChanged()",
            this,
            "handleNoteListBodyTextChanged()");
        m_currentNoteEntryChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "currentNoteEntryChanged()",
            this,
            "handleNoteListEntrySelectionChanged()");
        m_itemCountChangedConnection = connectIfSignalExists(
            m_noteListModel,
            "itemCountChanged(int)",
            this,
            "handleNoteListCountChanged()");

        if (const auto* itemModel = qobject_cast<const QAbstractItemModel*>(m_noteListModel.data()))
        {
            m_modelResetConnection = connect(
                itemModel,
                &QAbstractItemModel::modelReset,
                this,
                [this]() {
                    WhatSon::Debug::traceEditorSelf(
                        this,
                        QStringLiteral("selectionBridge"),
                        QStringLiteral("selectionFlow.modelReset"),
                        QStringLiteral("selectedNoteId=%1 {%2}")
                            .arg(m_selectedNoteId)
                            .arg(summarizeTraceNoteListModel(m_noteListModel)));
                    m_noteSelectionRefreshRequiresRebind = true;
                    scheduleNoteSelectionRefresh();
                    QTimer::singleShot(0, this, [this]() {
                        m_noteSelectionRefreshRequiresRebind = true;
                        scheduleNoteSelectionRefresh();
                    });
                });
            m_rowsInsertedConnection = connect(
                itemModel,
                &QAbstractItemModel::rowsInserted,
                this,
                [this](const QModelIndex&, int, int) {
                    m_noteSelectionRefreshRequiresRebind = true;
                    scheduleNoteSelectionRefresh();
                });
            m_rowsRemovedConnection = connect(
                itemModel,
                &QAbstractItemModel::rowsRemoved,
                this,
                [this](const QModelIndex&, int, int) {
                    m_noteSelectionRefreshRequiresRebind = true;
                    scheduleNoteSelectionRefresh();
                });
            m_dataChangedConnection = connect(
                itemModel,
                &QAbstractItemModel::dataChanged,
                this,
                [this](const QModelIndex&, const QModelIndex&, const QList<int>&) {
                    m_noteSelectionRefreshRequiresRebind = true;
                    scheduleNoteSelectionRefresh();
                });
            m_layoutChangedConnection = connect(
                itemModel,
                &QAbstractItemModel::layoutChanged,
                this,
                [this]() {
                    m_noteSelectionRefreshRequiresRebind = true;
                    scheduleNoteSelectionRefresh();
                });
        }
    }

    emit noteListModelChanged();
    m_noteSelectionRefreshRequiresRebind = true;
    scheduleNoteSelectionRefresh();
    refreshNoteCountState();
}

QObject* ContentsEditorSelectionBridge::contentController() const noexcept
{
    return m_contentController;
}

void ContentsEditorSelectionBridge::setContentController(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setContentController"),
        QStringLiteral("previous=0x%1 next=0x%2")
            .arg(QString::number(reinterpret_cast<quintptr>(m_contentController.data()), 16))
            .arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    if (m_contentController == model)
    {
        return;
    }

    disconnectContentController();
    stabilizeQmlBindingOwnership(model);
    m_contentController = model;

    if (m_contentController != nullptr)
    {
        m_contentControllerDestroyedConnection = connect(
            m_contentController,
            &QObject::destroyed,
            this,
            &ContentsEditorSelectionBridge::handleContentControllerDestroyed);
    }

    if (m_persistenceController != nullptr)
    {
        m_persistenceController->setContentController(m_contentController);
    }

    emit contentControllerChanged();
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
    return m_persistenceController != nullptr
        && m_persistenceController->contentPersistenceContractAvailable();
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

bool ContentsEditorSelectionBridge::selectedNoteBodyResolved() const noexcept
{
    return m_selectedNoteBodyResolved;
}

bool ContentsEditorSelectionBridge::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

int ContentsEditorSelectionBridge::visibleNoteCount() const noexcept
{
    return m_visibleNoteCount;
}

bool ContentsEditorSelectionBridge::reconcileViewSessionAndRefreshSnapshotForNote(
    const QString& noteId,
    const QString& viewSessionText,
    const bool preferViewSessionOnMismatch)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString selectedNoteDirectoryPath =
        normalizedNoteId == m_selectedNoteId.trimmed()
        ? m_selectedNoteDirectoryPath.trimmed()
        : QString();
    const bool accepted = m_persistenceController != nullptr
        && !normalizedNoteId.isEmpty()
        && (!selectedNoteDirectoryPath.isEmpty()
                ? m_persistenceController->reconcileViewSessionAndRefreshSnapshotForNoteAtPath(
                    normalizedNoteId,
                    selectedNoteDirectoryPath,
                    viewSessionText,
                    preferViewSessionOnMismatch)
                : m_persistenceController->reconcileViewSessionAndRefreshSnapshotForNote(
                    normalizedNoteId,
                    viewSessionText,
                    preferViewSessionOnMismatch));
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("reconcileViewSessionAndRefreshSnapshotForNote"),
        QStringLiteral("accepted=%1 preferViewSession=%2 noteId=%3 noteDirectoryPath=%4 %5")
            .arg(accepted)
            .arg(preferViewSessionOnMismatch)
            .arg(normalizedNoteId)
            .arg(selectedNoteDirectoryPath)
            .arg(WhatSon::Debug::summarizeText(viewSessionText)));
    return accepted;
}

bool ContentsEditorSelectionBridge::directPersistenceAvailable() const noexcept
{
    return m_persistenceController != nullptr
        && m_persistenceController->directPersistenceAvailable();
}

bool ContentsEditorSelectionBridge::refreshSelectedNoteSnapshot()
{
    const bool reloaded = m_persistenceController != nullptr
        && m_persistenceController->refreshNoteSnapshotForNote(m_selectedNoteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.refreshSelectedNoteSnapshot"),
        QStringLiteral("reloaded=%1 selectedNoteId=%2 bodyNoteId=%3 bodyResolved=%4 bodyLoading=%5")
            .arg(reloaded)
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_selectedNoteBodyLoading));
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
        QStringLiteral("selectionFlow.noteListSelectionChanged"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 queued=%3 {%4}")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_noteSelectionRefreshQueued)
            .arg(summarizeTraceNoteListModel(m_noteListModel)));
    scheduleNoteSelectionRefresh();
}

void ContentsEditorSelectionBridge::handleNoteListEntrySelectionChanged()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.noteListEntrySelectionChanged"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 queued=%3 {%4}")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_noteSelectionRefreshQueued)
            .arg(summarizeTraceNoteListModel(m_noteListModel)));
    m_noteSelectionRefreshRequiresRebind = true;
    scheduleNoteSelectionRefresh();
}

void ContentsEditorSelectionBridge::handleNoteListBodyTextChanged()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.noteListBodyTextChanged"),
        QStringLiteral("selectedNoteId=%1 snapshotNoteId=%2 {%3}")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodySnapshotNoteId)
            .arg(summarizeTraceNoteListModel(m_noteListModel)));
    m_selectedNoteBodySnapshotNoteId.clear();
    scheduleNoteSelectionRefresh();
}

void ContentsEditorSelectionBridge::handleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    const QString normalizedNoteId = noteId.trimmed();
    const bool accepted =
        sequence != 0
        && !normalizedNoteId.isEmpty()
        && normalizedNoteId == m_selectedNoteId.trimmed()
        && sequence == m_selectedNoteBodyRequestSequence;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.bodyLoadFinished"),
        QStringLiteral("sequence=%1 success=%2 accepted=%3 noteId=%4 selectedNoteId=%5 expectedSequence=%6 %7")
            .arg(sequence)
            .arg(success)
            .arg(accepted)
            .arg(normalizedNoteId)
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyRequestSequence)
            .arg(WhatSon::Debug::summarizeText(text)));

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
        QString fallbackText;
        bool fallbackResolved = tryResolveSelectedNoteBodySourceText(normalizedNoteId, &fallbackText);
        if (!fallbackResolved
            && m_selectedNoteBodyNoteId == normalizedNoteId
            && m_selectedNoteBodyResolved)
        {
            fallbackText = m_selectedNoteBodyText;
            fallbackResolved = true;
        }
        m_selectedNoteBodySnapshotNoteId = fallbackResolved ? normalizedNoteId : QString();
        setSelectedNoteBodyState(normalizedNoteId, fallbackText, false, fallbackResolved);
        return;
    }

    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    setSelectedNoteBodyState(normalizedNoteId, text, false, true);
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
        QStringLiteral("selectionFlow.viewSessionSnapshotReconciled"),
        QStringLiteral("success=%1 refreshed=%2 noteId=%3 selectedNoteId=%4")
            .arg(success)
            .arg(refreshed)
            .arg(noteId.trimmed())
            .arg(m_selectedNoteId));

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
        QStringLiteral("selectionFlow.refreshFlush"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 bodyLoading=%4")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_selectedNoteBodyLoading));
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

void ContentsEditorSelectionBridge::handleContentControllerDestroyed()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("handleContentControllerDestroyed"));
    disconnectContentController();
    m_contentController = nullptr;
    if (m_persistenceController != nullptr)
    {
        m_persistenceController->setContentController(nullptr);
    }
    m_selectedNoteBodyRequestSequence = 0;
    emit contentControllerChanged();
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
        return -1;
    }

    bool converted = false;
    const int value = object->property(propertyName).toInt(&converted);
    if (!converted)
    {
        return -1;
    }

    return value;
}

bool ContentsEditorSelectionBridge::adoptPendingEditorBodyText(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty() || m_persistenceController == nullptr)
    {
        return false;
    }

    QString pendingEditorText;
    if (!m_persistenceController->pendingEditorTextForNote(normalizedNoteId, &pendingEditorText))
    {
        return false;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.bodyLoadFromPendingEditor"),
        QStringLiteral("noteId=%1 %2")
            .arg(normalizedNoteId)
            .arg(WhatSon::Debug::summarizeText(pendingEditorText)));

    m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
    m_selectedNoteBodyRequestSequence = 0;
    setSelectedNoteBodyState(normalizedNoteId, pendingEditorText, false, true);
    return true;
}


QString ContentsEditorSelectionBridge::readNoteIdFromModelRow(int row) const
{
    const auto* itemModel = qobject_cast<const QAbstractItemModel*>(m_noteListModel.data());
    if (itemModel == nullptr || row < 0 || row >= itemModel->rowCount())
    {
        return {};
    }

    const QModelIndex index = itemModel->index(row, 0);
    if (!index.isValid())
    {
        return {};
    }

    const QVariant noteIdValue = itemModel->data(index, Qt::UserRole + 2);
    const QString noteId = noteIdValue.toString().trimmed();
    if (!noteId.isEmpty())
    {
        return noteId;
    }

    return itemModel->data(index, Qt::UserRole + 1).toString().trimmed();
}

QString ContentsEditorSelectionBridge::readNoteDirectoryPathFromModelRow(int row) const
{
    const auto* itemModel = qobject_cast<const QAbstractItemModel*>(m_noteListModel.data());
    if (itemModel == nullptr || row < 0 || row >= itemModel->rowCount())
    {
        return {};
    }

    const QModelIndex index = itemModel->index(row, 0);
    if (!index.isValid())
    {
        return {};
    }

    return itemModel->data(index, Qt::UserRole + 3).toString().trimmed();
}

QString ContentsEditorSelectionBridge::resolveCurrentNoteIdFromSelectionContract() const
{
    const QVariantMap currentNoteEntry = readCurrentNoteEntry(m_noteListModel.data());
    const QString entryNoteId = noteIdFromEntry(currentNoteEntry);
    if (!entryNoteId.isEmpty())
    {
        return entryNoteId;
    }

    const QString directNoteId = readStringProperty(m_noteListModel, "currentNoteId").trimmed();
    if (!directNoteId.isEmpty())
    {
        return directNoteId;
    }

    const int currentIndex = readIntProperty(m_noteListModel, "currentIndex");
    if (currentIndex < 0)
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.resolveCurrentNoteId.noCurrentIndex"),
            QStringLiteral("currentIndex=%1 {%2}")
                .arg(currentIndex)
                .arg(summarizeTraceNoteListModel(m_noteListModel)));
        return {};
    }

    const QString rowNoteId = readNoteIdFromModelRow(currentIndex);
    if (!rowNoteId.isEmpty())
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.resolveCurrentNoteId.rowFallback"),
            QStringLiteral("currentIndex=%1 rowNoteId=%2 committedEntryKeys=%3 {%4}")
                .arg(currentIndex)
                .arg(rowNoteId)
                .arg(currentNoteEntry.keys().join(QLatin1Char(',')))
                .arg(summarizeTraceNoteListModel(m_noteListModel)));
        return rowNoteId;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.resolveCurrentNoteId.unresolved"),
        QStringLiteral("currentIndex=%1 committedEntryKeys=%2 {%3}")
            .arg(currentIndex)
            .arg(currentNoteEntry.keys().join(QLatin1Char(',')))
            .arg(summarizeTraceNoteListModel(m_noteListModel)));
    return {};
}

QString ContentsEditorSelectionBridge::resolveCurrentNoteDirectoryPathFromSelectionContract(const QString& noteId) const
{
    const QVariantMap currentNoteEntry = readCurrentNoteEntry(m_noteListModel.data());
    const QString entryPath = noteDirectoryPathFromEntry(currentNoteEntry);
    if (!entryPath.isEmpty())
    {
        return entryPath;
    }
    const QString directPath = readStringProperty(m_noteListModel, "currentNoteDirectoryPath").trimmed();
    if (!directPath.isEmpty())
    {
        return directPath;
    }
    if (noteId.trimmed().isEmpty())
    {
        return {};
    }
    const int currentIndex = readIntProperty(m_noteListModel, "currentIndex");
    QString rowPath = readNoteDirectoryPathFromModelRow(currentIndex);
    if (!rowPath.trimmed().isEmpty())
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.resolveCurrentNoteDirectoryPath.rowFallback"),
            QStringLiteral("noteId=%1 currentIndex=%2 rowPath=%3 {%4}")
                .arg(noteId)
                .arg(currentIndex)
                .arg(rowPath)
                .arg(summarizeTraceNoteListModel(m_noteListModel)));
        return rowPath;
    }
    const QString resolvedPath = resolveSelectedNoteDirectoryPath(noteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.resolveCurrentNoteDirectoryPath"),
        QStringLiteral("noteId=%1 directPath=%2 resolvedPath=%3 {%4}")
            .arg(noteId)
            .arg(directPath)
            .arg(resolvedPath)
            .arg(summarizeTraceNoteListModel(m_noteListModel)));
    return resolvedPath;
}

bool ContentsEditorSelectionBridge::currentSelectionCanLoadSelectedNoteBody(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QVariantMap currentNoteEntry = readCurrentNoteEntry(m_noteListModel.data());
    const QString entryNoteId = noteIdFromEntry(currentNoteEntry);
    if (!entryNoteId.isEmpty())
    {
        return entryNoteId == normalizedNoteId;
    }

    if (!hasReadableProperty(m_noteListModel, "currentNoteId"))
    {
        return false;
    }

    return readStringProperty(m_noteListModel, "currentNoteId").trimmed() == normalizedNoteId;
}

QString ContentsEditorSelectionBridge::resolveSelectedNoteDirectoryPath(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    if (m_noteListModel != nullptr
        && hasReadableProperty(m_noteListModel, "currentNoteId")
        && hasReadableProperty(m_noteListModel, "currentNoteDirectoryPath"))
    {
        const QString currentNoteId = readStringProperty(m_noteListModel, "currentNoteId").trimmed();
        if (currentNoteId == normalizedNoteId)
        {
            const QString noteDirectoryPath = QDir::cleanPath(
                readStringProperty(m_noteListModel, "currentNoteDirectoryPath").trimmed());
            if (!noteDirectoryPath.isEmpty() && noteDirectoryPath != QStringLiteral("."))
            {
                return noteDirectoryPath;
            }
        }
    }

    if (m_persistenceController != nullptr)
    {
        const QString controllerPath = m_persistenceController->noteDirectoryPathForNote(normalizedNoteId).trimmed();
        if (!controllerPath.isEmpty())
        {
            return controllerPath;
        }
    }

    if (m_contentController == nullptr
        || !hasInvokableMethod(m_contentController, kNoteDirectoryPathForNoteIdSignature))
    {
        return {};
    }

    QString noteDirectoryPath;
    if (!QMetaObject::invokeMethod(
            m_contentController,
            "noteDirectoryPathForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, noteDirectoryPath),
            Q_ARG(QString, normalizedNoteId)))
    {
        return {};
    }

    return noteDirectoryPath.trimmed();
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
    if (normalizedNoteId.isEmpty() || m_contentController == nullptr
        || !hasInvokableMethod(m_contentController, kNoteBodySourceTextForNoteIdSignature))
    {
        return false;
    }

    QString resolvedBodyText;
    if (!QMetaObject::invokeMethod(
            m_contentController,
            "noteBodySourceTextForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, resolvedBodyText),
            Q_ARG(QString, normalizedNoteId)))
    {
        return false;
    }

    const QString resolvedNoteDirectoryPath = resolveSelectedNoteDirectoryPath(normalizedNoteId);
    if (resolvedBodyText.isEmpty()
        && !resolvedNoteDirectoryPath.isEmpty()
        && m_persistenceController != nullptr)
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
    const bool loading,
    const bool resolved)
{
    const bool noteIdChanged = m_selectedNoteBodyNoteId != noteId;
    const bool loadingChanged = m_selectedNoteBodyLoading != loading;
    const bool bodyTextChanged = m_selectedNoteBodyText != bodyText;
    const bool resolvedChanged = m_selectedNoteBodyResolved != resolved;

    m_selectedNoteBodyNoteId = std::move(noteId);
    m_selectedNoteBodyLoading = loading;
    m_selectedNoteBodyText = std::move(bodyText);
    m_selectedNoteBodyResolved = resolved;

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("setSelectedNoteBodyState"),
        QStringLiteral("noteId=%1 resolved=%2 loading=%3 %4")
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
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
    if (resolvedChanged)
    {
        emit selectedNoteBodyResolvedChanged();
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
        QStringLiteral("selectionFlow.bodyLoadStart"),
        QStringLiteral("noteId=%1 clearCachedBody=%2 selectedNoteId=%3 bodyNoteId=%4 bodyResolved=%5 bodyLoading=%6 snapshotNoteId=%7")
            .arg(normalizedNoteId)
            .arg(clearCachedBody)
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_selectedNoteBodyLoading)
            .arg(m_selectedNoteBodySnapshotNoteId));
    if (normalizedNoteId.isEmpty())
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.bodyCleared"),
            QStringLiteral("reason=no-selected-note"));
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), QString(), false, false);
        return;
    }

    if (clearCachedBody)
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
    }

    if (!currentSelectionCanLoadSelectedNoteBody(normalizedNoteId))
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.bodyLoadSkipped"),
            QStringLiteral("noteId=%1 reason=missing-committed-body-selection-contract {%2}")
                .arg(normalizedNoteId)
                .arg(summarizeTraceNoteListModel(m_noteListModel)));
        setSelectedNoteBodyState(QString(), QString(), false, false);
        return;
    }

    if (adoptPendingEditorBodyText(normalizedNoteId))
    {
        return;
    }

    const bool noteBodyOwnedBySelection = m_selectedNoteBodyNoteId == normalizedNoteId;
    const QString currentBodyText = clearCachedBody || !noteBodyOwnedBySelection
        ? QString()
        : m_selectedNoteBodyText;
    QString immediateBodyText = currentBodyText;
    bool immediateBodyResolved =
        !clearCachedBody
        && noteBodyOwnedBySelection
        && m_selectedNoteBodyResolved
        && m_selectedNoteBodySnapshotNoteId == normalizedNoteId;

    const QString noteDirectoryPath = resolveCurrentNoteDirectoryPathFromSelectionContract(normalizedNoteId);
    const bool canDeferToWsnoteLoad = !noteDirectoryPath.isEmpty() && m_persistenceController != nullptr;
    if (!immediateBodyResolved && !canDeferToWsnoteLoad
        && tryResolveSelectedNoteBodySourceText(normalizedNoteId, &immediateBodyText))
    {
        m_selectedNoteBodySnapshotNoteId = normalizedNoteId;
        immediateBodyResolved = true;
    }
    else if (!immediateBodyResolved)
    {
        m_selectedNoteBodySnapshotNoteId.clear();
    }

    setSelectedNoteBodyState(normalizedNoteId, immediateBodyText, !immediateBodyResolved, immediateBodyResolved);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.bodyLoadImmediate"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 resolved=%3 loading=%4 snapshotNoteId=%5 %6")
            .arg(normalizedNoteId)
            .arg(noteDirectoryPath)
            .arg(immediateBodyResolved)
            .arg(!immediateBodyResolved)
            .arg(m_selectedNoteBodySnapshotNoteId)
            .arg(WhatSon::Debug::summarizeText(immediateBodyText)));
    const quint64 requestSequence = m_persistenceController != nullptr
        ? (!noteDirectoryPath.isEmpty()
                ? m_persistenceController->loadNoteBodyTextForNoteAtPath(normalizedNoteId, noteDirectoryPath)
                : m_persistenceController->loadNoteBodyTextForNote(normalizedNoteId))
        : 0;
    if (requestSequence != 0)
    {
        m_selectedNoteBodyRequestSequence = requestSequence;
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.bodyLoadAsyncRequested"),
            QStringLiteral("noteId=%1 noteDirectoryPath=%2 requestSequence=%3")
                .arg(normalizedNoteId)
                .arg(noteDirectoryPath)
                .arg(requestSequence));
        return;
    }

    QString fallbackBodyText;
    bool fallbackResolved = tryResolveSelectedNoteBodySourceText(normalizedNoteId, &fallbackBodyText);
    if (!fallbackResolved
        && noteBodyOwnedBySelection
        && m_selectedNoteBodyResolved)
    {
        fallbackBodyText = m_selectedNoteBodyText;
        fallbackResolved = true;
    }

    m_selectedNoteBodyRequestSequence = 0;
    m_selectedNoteBodySnapshotNoteId = fallbackResolved ? normalizedNoteId : QString();
    setSelectedNoteBodyState(normalizedNoteId, fallbackBodyText, false, fallbackResolved);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        fallbackResolved
            ? QStringLiteral("selectionFlow.bodyLoadFallbackResolved")
            : QStringLiteral("selectionFlow.bodyLoadFallbackEmpty"),
        QStringLiteral("noteId=%1 snapshotNoteId=%2 %3")
            .arg(normalizedNoteId)
            .arg(m_selectedNoteBodySnapshotNoteId)
            .arg(WhatSon::Debug::summarizeText(fallbackBodyText)));
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
        QStringLiteral("selectionFlow.refreshScheduled"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 queued=%3")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_noteSelectionRefreshQueued));
    QMetaObject::invokeMethod(
        this,
        &ContentsEditorSelectionBridge::flushPendingNoteSelectionRefresh,
        Qt::QueuedConnection);
}

void ContentsEditorSelectionBridge::refreshNoteSelectionState()
{
    const bool currentNoteEntryReadable = hasReadableProperty(m_noteListModel, "currentNoteEntry");
    const QVariantMap nextCurrentNoteEntry = currentNoteEntryReadable
        ? readCurrentNoteEntry(m_noteListModel.data())
        : QVariantMap();
    const bool rowSelectionFallbackAvailable = qobject_cast<const QAbstractItemModel*>(m_noteListModel.data()) != nullptr
        && hasReadableProperty(m_noteListModel, "currentIndex");
    const bool nextContractAvailable = noteBackedSelectionEnabled(m_noteListModel)
        && (currentNoteEntryReadable
            || hasReadableProperty(m_noteListModel, "currentNoteId")
            || rowSelectionFallbackAvailable);
    const QString previousNoteId = m_selectedNoteId;
    const QString previousNoteDirectoryPath = m_selectedNoteDirectoryPath;
    const QString proposedNoteId = nextContractAvailable
        ? resolveCurrentNoteIdFromSelectionContract()
        : QString();
    const int visibleItemCount = hasReadableProperty(m_noteListModel, "itemCount")
        ? readIntProperty(m_noteListModel, "itemCount")
        : 0;
    const QString nextNoteId = proposedNoteId;
    QString nextNoteDirectoryPath;
    if (!nextNoteId.trimmed().isEmpty())
    {
        nextNoteDirectoryPath = resolveCurrentNoteDirectoryPathFromSelectionContract(nextNoteId);
    }
    const bool noteDirectoryPathChanged = previousNoteDirectoryPath != nextNoteDirectoryPath;
    const bool committedEntryChanged = currentNoteEntryReadable && m_selectedCurrentNoteEntry != nextCurrentNoteEntry;
    const bool requiresRebind = m_noteSelectionRefreshRequiresRebind || committedEntryChanged;
    m_noteSelectionRefreshRequiresRebind = false;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        QStringLiteral("selectionFlow.refreshState"),
        QStringLiteral("contract=%1 previousNoteId=%2 previousNoteDirectoryPath=%3 proposedNoteId=%4 nextNoteId=%5 nextNoteDirectoryPath=%6 visibleItemCount=%7 requiresRebind=%8 committedEntryChanged=%9 {%10}")
            .arg(nextContractAvailable)
            .arg(previousNoteId)
            .arg(previousNoteDirectoryPath)
            .arg(proposedNoteId)
            .arg(nextNoteId)
            .arg(nextNoteDirectoryPath)
            .arg(visibleItemCount)
            .arg(requiresRebind)
            .arg(committedEntryChanged)
            .arg(summarizeTraceNoteListModel(m_noteListModel)));

    if (m_noteSelectionContractAvailable != nextContractAvailable)
    {
        m_noteSelectionContractAvailable = nextContractAvailable;
        emit noteSelectionContractAvailableChanged();
    }
    if (m_persistenceController != nullptr
        && (m_selectedNoteId != nextNoteId || noteDirectoryPathChanged || requiresRebind))
    {
        if (nextNoteId.trimmed().isEmpty())
        {
            m_persistenceController->clearSelectedNote();
        }
        else if (!nextNoteDirectoryPath.isEmpty())
        {
            m_persistenceController->bindSelectedNoteAtPath(nextNoteId, nextNoteDirectoryPath);
        }
        else
        {
            m_persistenceController->bindSelectedNote(nextNoteId);
        }
    }

    const bool noteIdChanged = m_selectedNoteId != nextNoteId;
    if (!noteIdChanged)
    {
        m_selectedCurrentNoteEntry = nextCurrentNoteEntry;
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("selectionBridge"),
            QStringLiteral("selectionFlow.noteStable"),
            QStringLiteral("noteId=%1 noteDirectoryPathChanged=%2 requiresRebind=%3 committedEntryChanged=%4 snapshotNoteId=%5 bodyResolved=%6")
                .arg(m_selectedNoteId)
                .arg(noteDirectoryPathChanged)
                .arg(requiresRebind)
                .arg(committedEntryChanged)
                .arg(m_selectedNoteBodySnapshotNoteId)
                .arg(m_selectedNoteBodyResolved));
        setSelectedNoteDirectoryPath(nextNoteDirectoryPath);
        if (!m_selectedNoteId.trimmed().isEmpty()
            && (m_selectedNoteBodySnapshotNoteId != m_selectedNoteId
                || noteDirectoryPathChanged
                || requiresRebind)
            && !m_selectedNoteBodyLoading)
        {
            startSelectedNoteBodyLoad(
                m_selectedNoteId,
                noteDirectoryPathChanged || (!m_selectedNoteBodyResolved && m_selectedNoteBodyText.isEmpty()));
        }
        return;
    }

    m_selectedCurrentNoteEntry = nextCurrentNoteEntry;
    m_selectedNoteId = nextNoteId;
    emit selectedNoteIdChanged();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("selectionBridge"),
        nextNoteId.trimmed().isEmpty()
            ? QStringLiteral("selectionFlow.noteCleared")
            : QStringLiteral("selectionFlow.noteChanged"),
        QStringLiteral("previousNoteId=%1 nextNoteId=%2 requiresRebind=%3")
            .arg(previousNoteId)
            .arg(nextNoteId)
            .arg(requiresRebind));
    setSelectedNoteDirectoryPath(nextNoteDirectoryPath);

    if (nextNoteId.trimmed().isEmpty())
    {
        m_selectedNoteBodySnapshotNoteId.clear();
        m_selectedNoteBodyRequestSequence = 0;
        setSelectedNoteBodyState(QString(), QString(), false, false);
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
    if (m_currentIndexChangedConnection)
    {
        disconnect(m_currentIndexChangedConnection);
        m_currentIndexChangedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteIdChangedConnection)
    {
        disconnect(m_currentNoteIdChangedConnection);
        m_currentNoteIdChangedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteDirectoryPathChangedConnection)
    {
        disconnect(m_currentNoteDirectoryPathChangedConnection);
        m_currentNoteDirectoryPathChangedConnection = QMetaObject::Connection();
    }
    if (m_currentBodyTextChangedConnection)
    {
        disconnect(m_currentBodyTextChangedConnection);
        m_currentBodyTextChangedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteEntryChangedConnection)
    {
        disconnect(m_currentNoteEntryChangedConnection);
        m_currentNoteEntryChangedConnection = QMetaObject::Connection();
    }
    if (m_itemCountChangedConnection)
    {
        disconnect(m_itemCountChangedConnection);
        m_itemCountChangedConnection = QMetaObject::Connection();
    }
    if (m_modelResetConnection)
    {
        disconnect(m_modelResetConnection);
        m_modelResetConnection = QMetaObject::Connection();
    }
    if (m_rowsInsertedConnection)
    {
        disconnect(m_rowsInsertedConnection);
        m_rowsInsertedConnection = QMetaObject::Connection();
    }
    if (m_rowsRemovedConnection)
    {
        disconnect(m_rowsRemovedConnection);
        m_rowsRemovedConnection = QMetaObject::Connection();
    }
    if (m_dataChangedConnection)
    {
        disconnect(m_dataChangedConnection);
        m_dataChangedConnection = QMetaObject::Connection();
    }
    if (m_layoutChangedConnection)
    {
        disconnect(m_layoutChangedConnection);
        m_layoutChangedConnection = QMetaObject::Connection();
    }
}

void ContentsEditorSelectionBridge::disconnectContentController()
{
    if (m_contentControllerDestroyedConnection)
    {
        disconnect(m_contentControllerDestroyedConnection);
        m_contentControllerDestroyedConnection = QMetaObject::Connection();
    }
}
