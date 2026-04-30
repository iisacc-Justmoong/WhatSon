#include "app/models/panel/FocusedNoteDeletionBridge.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QMetaObject>
#include <QMetaProperty>

namespace
{
    constexpr auto kDeleteNoteByIdSignature = "deleteNoteById(QString)";
}

FocusedNoteDeletionBridge::FocusedNoteDeletionBridge(QObject* parent)
    : QObject(parent)
{
}

FocusedNoteDeletionBridge::~FocusedNoteDeletionBridge() = default;

QObject* FocusedNoteDeletionBridge::noteListModel() const noexcept
{
    return m_noteListModel;
}

void FocusedNoteDeletionBridge::setNoteListModel(QObject* model)
{
    if (model != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("FocusedNoteDeletionBridge::setNoteListModel")))
    {
        return;
    }

    if (model == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("FocusedNoteDeletionBridge::setNoteListModel")))
    {
        return;
    }

    if (m_noteListModel == model)
    {
        return;
    }

    disconnectNoteListModel();
    m_noteListModel = model;

    if (m_noteListModel != nullptr)
    {
        m_noteListDestroyedConnection = connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            &FocusedNoteDeletionBridge::handleNoteListDestroyed);
        m_currentNoteIdChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentNoteIdChanged()),
            this,
            SLOT(handleNoteListSelectionChanged()));
    }

    emit noteListModelChanged();
    refreshFocusedNoteState();
}

QObject* FocusedNoteDeletionBridge::deletionTarget() const noexcept
{
    return m_deletionTarget;
}

void FocusedNoteDeletionBridge::setDeletionTarget(QObject* target)
{
    if (target != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("FocusedNoteDeletionBridge::setDeletionTarget")))
    {
        return;
    }

    if (target == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("FocusedNoteDeletionBridge::setDeletionTarget")))
    {
        return;
    }

    if (m_deletionTarget == target)
    {
        return;
    }

    disconnectDeletionTarget();
    m_deletionTarget = target;

    if (m_deletionTarget != nullptr)
    {
        m_deletionTargetDestroyedConnection = connect(
            m_deletionTarget,
            &QObject::destroyed,
            this,
            &FocusedNoteDeletionBridge::handleDeletionTargetDestroyed);
    }

    emit deletionTargetChanged();
    refreshDeleteContractState();
}

QString FocusedNoteDeletionBridge::focusedNoteId() const
{
    return m_focusedNoteId;
}

void FocusedNoteDeletionBridge::setFocusedNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (m_focusedNoteId == normalizedNoteId)
    {
        return;
    }

    m_focusedNoteId = normalizedNoteId;
    emit focusedNoteIdChanged();
    refreshFocusedNoteState();
}

bool FocusedNoteDeletionBridge::focusedNoteAvailable() const noexcept
{
    return m_focusedNoteAvailable;
}

bool FocusedNoteDeletionBridge::deleteContractAvailable() const noexcept
{
    return m_deleteContractAvailable;
}

bool FocusedNoteDeletionBridge::deleteFocusedNote()
{
    if (m_deletionTarget == nullptr || !m_deleteContractAvailable)
    {
        return false;
    }

    const QString noteId = resolvedFocusedNoteId();
    if (noteId.isEmpty())
    {
        return false;
    }

    bool deleted = false;
    const bool invoked = QMetaObject::invokeMethod(
        m_deletionTarget,
        "deleteNoteById",
        Qt::DirectConnection,
        Q_RETURN_ARG(bool, deleted),
        Q_ARG(QString, noteId));
    if (!invoked || !deleted)
    {
        return false;
    }

    if (m_focusedNoteId == noteId)
    {
        setFocusedNoteId(QString());
    }
    return true;
}

void FocusedNoteDeletionBridge::handleNoteListSelectionChanged()
{
    refreshFocusedNoteState();
}

void FocusedNoteDeletionBridge::handleNoteListDestroyed()
{
    disconnectNoteListModel();
    m_noteListModel = nullptr;
    emit noteListModelChanged();
    refreshFocusedNoteState();
}

void FocusedNoteDeletionBridge::handleDeletionTargetDestroyed()
{
    disconnectDeletionTarget();
    m_deletionTarget = nullptr;
    emit deletionTargetChanged();
    refreshDeleteContractState();
}

bool FocusedNoteDeletionBridge::hasReadableProperty(const QObject* object, const char* propertyName)
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

bool FocusedNoteDeletionBridge::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString FocusedNoteDeletionBridge::readStringProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return {};
    }

    return object->property(propertyName).toString();
}

QString FocusedNoteDeletionBridge::resolvedFocusedNoteId() const
{
    if (!m_focusedNoteId.isEmpty())
    {
        return m_focusedNoteId;
    }

    return readStringProperty(m_noteListModel, "currentNoteId").trimmed();
}

void FocusedNoteDeletionBridge::refreshFocusedNoteState()
{
    const bool nextFocusedNoteAvailable = !resolvedFocusedNoteId().isEmpty();
    if (m_focusedNoteAvailable == nextFocusedNoteAvailable)
    {
        return;
    }

    m_focusedNoteAvailable = nextFocusedNoteAvailable;
    emit focusedNoteAvailableChanged();
}

void FocusedNoteDeletionBridge::refreshDeleteContractState()
{
    const bool nextDeleteContractAvailable = hasInvokableMethod(m_deletionTarget, kDeleteNoteByIdSignature);
    if (m_deleteContractAvailable == nextDeleteContractAvailable)
    {
        return;
    }

    m_deleteContractAvailable = nextDeleteContractAvailable;
    emit deleteContractAvailableChanged();
}

void FocusedNoteDeletionBridge::disconnectNoteListModel()
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
}

void FocusedNoteDeletionBridge::disconnectDeletionTarget()
{
    if (m_deletionTargetDestroyedConnection)
    {
        disconnect(m_deletionTargetDestroyedConnection);
        m_deletionTargetDestroyedConnection = QMetaObject::Connection();
    }
}
