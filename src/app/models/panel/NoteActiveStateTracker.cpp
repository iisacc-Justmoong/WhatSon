#include "app/models/panel/NoteActiveStateTracker.hpp"

#include "app/models/sidebar/IActiveHierarchyContextSource.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QAbstractItemModel>
#include <QDebug>
#include <QDir>
#include <QMetaObject>
#include <QMetaProperty>
#include <QQmlEngine>

#include <algorithm>
#include <utility>

namespace
{
    bool hasReadableProperty(const QObject* object, const char* propertyName)
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

    bool hasSignal(const QObject* object, const char* signalSignature)
    {
        if (object == nullptr || signalSignature == nullptr)
        {
            return false;
        }

        const QMetaObject* metaObject = object->metaObject();
        return metaObject != nullptr
            && metaObject->indexOfSignal(QMetaObject::normalizedSignature(signalSignature)) >= 0;
    }

    int readIntProperty(const QObject* object, const char* propertyName, const int fallbackValue)
    {
        if (!hasReadableProperty(object, propertyName))
        {
            return fallbackValue;
        }

        bool converted = false;
        const int value = object->property(propertyName).toInt(&converted);
        return converted ? value : fallbackValue;
    }

    QString normalizeNoteDirectoryPath(QString noteDirectoryPath)
    {
        noteDirectoryPath = QDir::cleanPath(noteDirectoryPath.trimmed());
        if (noteDirectoryPath == QStringLiteral("."))
        {
            noteDirectoryPath.clear();
        }
        return noteDirectoryPath;
    }

    bool noteBackedSelectionEnabled(const QObject* noteListModel)
    {
        if (noteListModel == nullptr)
        {
            return false;
        }
        if (!hasReadableProperty(noteListModel, "noteBacked"))
        {
            return true;
        }
        return noteListModel->property("noteBacked").toBool();
    }

    QVariantMap normalizeNoteEntry(QVariantMap noteEntry)
    {
        noteEntry.remove(QStringLiteral("bodyText"));

        if (!noteEntry.contains(QStringLiteral("noteId")) && noteEntry.contains(QStringLiteral("id")))
        {
            noteEntry.insert(QStringLiteral("noteId"), noteEntry.value(QStringLiteral("id")));
        }

        if (noteEntry.contains(QStringLiteral("noteDirectoryPath")))
        {
            const QString normalizedPath = normalizeNoteDirectoryPath(
                noteEntry.value(QStringLiteral("noteDirectoryPath")).toString());
            if (normalizedPath.isEmpty())
            {
                noteEntry.remove(QStringLiteral("noteDirectoryPath"));
            }
            else
            {
                noteEntry.insert(QStringLiteral("noteDirectoryPath"), normalizedPath);
            }
        }

        return noteEntry;
    }

    QVariantMap readRawCurrentNoteEntryProperty(const QObject* noteListModel)
    {
        if (!hasReadableProperty(noteListModel, "currentNoteEntry"))
        {
            return {};
        }
        return noteListModel->property("currentNoteEntry").toMap();
    }

    QVariantMap readCurrentNoteEntryProperty(const QObject* noteListModel)
    {
        return normalizeNoteEntry(readRawCurrentNoteEntryProperty(noteListModel));
    }

    QVariantMap rowSnapshotAt(const QAbstractItemModel* model, const int row)
    {
        QVariantMap snapshot;
        if (model == nullptr || row < 0 || row >= model->rowCount())
        {
            return snapshot;
        }

        const QModelIndex modelIndex = model->index(row, 0);
        if (!modelIndex.isValid())
        {
            return snapshot;
        }

        const QHash<int, QByteArray> roleNames = model->roleNames();
        for (auto iterator = roleNames.constBegin(); iterator != roleNames.constEnd(); ++iterator)
        {
            const QByteArray roleName = iterator.value().trimmed();
            if (roleName.isEmpty() || roleName == QByteArrayLiteral("bodyText"))
            {
                continue;
            }
            snapshot.insert(QString::fromUtf8(roleName), modelIndex.data(iterator.key()));
        }

        return normalizeNoteEntry(std::move(snapshot));
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
        return normalizeNoteDirectoryPath(
            noteEntry.value(QStringLiteral("noteDirectoryPath")).toString());
    }

    void stabilizeQmlBindingOwnership(QObject* object)
    {
        if (object != nullptr)
        {
            QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        }
    }
}

NoteActiveStateTracker::NoteActiveStateTracker(QObject* parent)
    : QObject(parent)
{
}

NoteActiveStateTracker::~NoteActiveStateTracker() = default;

QObject* NoteActiveStateTracker::hierarchyContextSource() const noexcept
{
    return m_hierarchyContextSource.data();
}

void NoteActiveStateTracker::setHierarchyContextSource(QObject* source)
{
    if (source != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("NoteActiveStateTracker::setHierarchyContextSource")))
    {
        return;
    }

    if (source == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("NoteActiveStateTracker::setHierarchyContextSource")))
    {
        return;
    }

    IActiveHierarchyContextSource* contextSource = qobject_cast<IActiveHierarchyContextSource*>(source);
    if (source != nullptr && contextSource == nullptr)
    {
        qWarning().noquote()
            << QStringLiteral("NoteActiveStateTracker requires an IActiveHierarchyContextSource object.");
        return;
    }

    if (m_hierarchyContextSource == contextSource)
    {
        return;
    }

    disconnectHierarchyContextSource();
    m_hierarchyContextSource = contextSource;
    if (m_hierarchyContextSource != nullptr)
    {
        stabilizeQmlBindingOwnership(m_hierarchyContextSource);
        m_hierarchyConnections.append(connect(
            m_hierarchyContextSource,
            &IActiveHierarchyContextSource::activeBindingsChanged,
            this,
            &NoteActiveStateTracker::synchronizeActiveBindings));
        m_hierarchyConnections.append(connect(
            m_hierarchyContextSource,
            &IActiveHierarchyContextSource::activeHierarchyControllerChanged,
            this,
            &NoteActiveStateTracker::synchronizeActiveBindings));
        m_hierarchyConnections.append(connect(
            m_hierarchyContextSource,
            &IActiveHierarchyContextSource::activeNoteListModelChanged,
            this,
            &NoteActiveStateTracker::synchronizeActiveBindings));
        m_hierarchyConnections.append(connect(
            m_hierarchyContextSource,
            &QObject::destroyed,
            this,
            &NoteActiveStateTracker::handleHierarchyContextDestroyed));
    }

    emit hierarchyContextSourceChanged();
    synchronizeActiveBindings();
}

int NoteActiveStateTracker::activeHierarchyIndex() const noexcept
{
    return m_activeHierarchyIndex;
}

QObject* NoteActiveStateTracker::activeHierarchyController() const noexcept
{
    return m_activeHierarchyController.data();
}

QObject* NoteActiveStateTracker::activeNoteListModel() const noexcept
{
    return m_activeNoteListModel.data();
}

QVariantMap NoteActiveStateTracker::activeNoteEntry() const
{
    return m_activeNoteEntry;
}

QString NoteActiveStateTracker::activeNoteId() const
{
    return m_activeNoteId;
}

QString NoteActiveStateTracker::activeNoteDirectoryPath() const
{
    return m_activeNoteDirectoryPath;
}

bool NoteActiveStateTracker::hasActiveNote() const noexcept
{
    return !m_activeNoteId.trimmed().isEmpty();
}

QVariantMap NoteActiveStateTracker::readActiveNoteEntry() const
{
    return activeNoteEntry();
}

void NoteActiveStateTracker::refresh()
{
    synchronizeActiveBindings();
}

void NoteActiveStateTracker::handleHierarchyContextDestroyed()
{
    disconnectHierarchyContextSource();
    m_hierarchyContextSource = nullptr;
    emit hierarchyContextSourceChanged();
    synchronizeActiveBindings();
}

void NoteActiveStateTracker::handleActiveNoteListModelDestroyed()
{
    disconnectActiveNoteListModel();
    const bool hadModel = m_activeNoteListModel != nullptr;
    m_activeNoteListModel = nullptr;
    if (hadModel)
    {
        emit activeNoteListModelChanged();
    }
    refreshActiveNoteState();
}

void NoteActiveStateTracker::synchronizeActiveBindings()
{
    int nextHierarchyIndex = 0;
    QObject* nextHierarchyController = nullptr;
    QObject* nextNoteListModel = nullptr;

    if (m_hierarchyContextSource != nullptr)
    {
        nextHierarchyIndex = m_hierarchyContextSource->activeHierarchyIndex();
        nextHierarchyController = m_hierarchyContextSource->activeHierarchyController();
        nextNoteListModel = m_hierarchyContextSource->activeNoteListModel();
    }

    stabilizeQmlBindingOwnership(nextHierarchyController);
    if (m_activeHierarchyIndex != nextHierarchyIndex)
    {
        m_activeHierarchyIndex = nextHierarchyIndex;
        emit activeHierarchyIndexChanged();
    }
    if (m_activeHierarchyController != nextHierarchyController)
    {
        m_activeHierarchyController = nextHierarchyController;
        emit activeHierarchyControllerChanged();
    }

    setActiveNoteListModel(nextNoteListModel);
}

void NoteActiveStateTracker::refreshActiveNoteState()
{
    const QObject* noteListModel = m_activeNoteListModel.data();
    QVariantMap nextEntry;
    QString nextNoteId;
    QString nextNoteDirectoryPath;

    if (noteListModel != nullptr && noteBackedSelectionEnabled(noteListModel))
    {
        const bool currentNoteEntryReadable = hasReadableProperty(noteListModel, "currentNoteEntry");
        const bool currentNoteIdReadable = hasReadableProperty(noteListModel, "currentNoteId");
        const bool currentNoteDirectoryPathReadable = hasReadableProperty(noteListModel, "currentNoteDirectoryPath");

        if (currentNoteEntryReadable)
        {
            nextEntry = readCurrentNoteEntryProperty(noteListModel);
        }
        else if (!currentNoteIdReadable)
        {
            const int currentIndex = readIntProperty(noteListModel, "currentIndex", -1);
            nextEntry = rowSnapshotAt(qobject_cast<const QAbstractItemModel*>(noteListModel), currentIndex);
        }

        nextNoteId = noteIdFromEntry(nextEntry);
        if (nextNoteId.isEmpty() && currentNoteIdReadable)
        {
            nextNoteId = noteListModel->property("currentNoteId").toString().trimmed();
        }

        nextNoteDirectoryPath = noteDirectoryPathFromEntry(nextEntry);
        if (nextNoteDirectoryPath.isEmpty() && currentNoteDirectoryPathReadable)
        {
            nextNoteDirectoryPath = normalizeNoteDirectoryPath(
                noteListModel->property("currentNoteDirectoryPath").toString());
        }

        if (nextNoteId.isEmpty())
        {
            nextEntry.clear();
            nextNoteDirectoryPath.clear();
        }
        else
        {
            nextEntry.insert(QStringLiteral("noteId"), nextNoteId);
            if (!nextNoteDirectoryPath.isEmpty())
            {
                nextEntry.insert(QStringLiteral("noteDirectoryPath"), nextNoteDirectoryPath);
            }
        }
    }

    setActiveNoteState(
        std::move(nextEntry),
        std::move(nextNoteId),
        std::move(nextNoteDirectoryPath));
}

void NoteActiveStateTracker::disconnectHierarchyContextSource()
{
    for (const QMetaObject::Connection& connection : std::as_const(m_hierarchyConnections))
    {
        QObject::disconnect(connection);
    }
    m_hierarchyConnections.clear();
}

void NoteActiveStateTracker::disconnectActiveNoteListModel()
{
    for (const QMetaObject::Connection& connection : std::as_const(m_noteListConnections))
    {
        QObject::disconnect(connection);
    }
    m_noteListConnections.clear();
}

void NoteActiveStateTracker::setActiveNoteListModel(QObject* model)
{
    if (m_activeNoteListModel == model)
    {
        refreshActiveNoteState();
        return;
    }

    disconnectActiveNoteListModel();
    stabilizeQmlBindingOwnership(model);
    m_activeNoteListModel = model;
    if (m_activeNoteListModel != nullptr)
    {
        m_noteListConnections.append(connect(
            m_activeNoteListModel,
            &QObject::destroyed,
            this,
            &NoteActiveStateTracker::handleActiveNoteListModelDestroyed));

        const auto connectOptionalSignal = [this](const char* normalizedSignalSignature, const char* signal)
        {
            if (hasSignal(m_activeNoteListModel, normalizedSignalSignature))
            {
                m_noteListConnections.append(connect(
                    m_activeNoteListModel,
                    signal,
                    this,
                    SLOT(refreshActiveNoteState())));
            }
        };

        connectOptionalSignal("currentIndexChanged()", SIGNAL(currentIndexChanged()));
        connectOptionalSignal("currentNoteEntryChanged()", SIGNAL(currentNoteEntryChanged()));
        connectOptionalSignal("currentNoteIdChanged()", SIGNAL(currentNoteIdChanged()));
        connectOptionalSignal("currentNoteDirectoryPathChanged()", SIGNAL(currentNoteDirectoryPathChanged()));
        connectOptionalSignal("noteBackedChanged()", SIGNAL(noteBackedChanged()));

        if (auto* abstractModel = qobject_cast<QAbstractItemModel*>(m_activeNoteListModel.data()))
        {
            m_noteListConnections.append(connect(
                abstractModel,
                &QAbstractItemModel::dataChanged,
                this,
                &NoteActiveStateTracker::refreshActiveNoteState));
            m_noteListConnections.append(connect(
                abstractModel,
                &QAbstractItemModel::layoutChanged,
                this,
                &NoteActiveStateTracker::refreshActiveNoteState));
            m_noteListConnections.append(connect(
                abstractModel,
                &QAbstractItemModel::modelReset,
                this,
                &NoteActiveStateTracker::refreshActiveNoteState));
            m_noteListConnections.append(connect(
                abstractModel,
                &QAbstractItemModel::rowsInserted,
                this,
                &NoteActiveStateTracker::refreshActiveNoteState));
            m_noteListConnections.append(connect(
                abstractModel,
                &QAbstractItemModel::rowsRemoved,
                this,
                &NoteActiveStateTracker::refreshActiveNoteState));
        }
    }

    emit activeNoteListModelChanged();
    refreshActiveNoteState();
}

void NoteActiveStateTracker::setActiveNoteState(
    QVariantMap noteEntry,
    QString noteId,
    QString noteDirectoryPath)
{
    noteId = noteId.trimmed();
    noteDirectoryPath = normalizeNoteDirectoryPath(std::move(noteDirectoryPath));
    if (noteId.isEmpty())
    {
        noteDirectoryPath.clear();
    }

    const bool previousHasActiveNote = hasActiveNote();
    const bool entryChanged = m_activeNoteEntry != noteEntry;
    const bool noteIdChanged = m_activeNoteId != noteId;
    const bool noteDirectoryPathChanged = m_activeNoteDirectoryPath != noteDirectoryPath;

    if (entryChanged)
    {
        m_activeNoteEntry = std::move(noteEntry);
    }
    if (noteIdChanged)
    {
        m_activeNoteId = std::move(noteId);
    }
    if (noteDirectoryPathChanged)
    {
        m_activeNoteDirectoryPath = std::move(noteDirectoryPath);
    }
    if (entryChanged)
    {
        emit activeNoteEntryChanged();
    }
    if (noteIdChanged)
    {
        emit activeNoteIdChanged();
    }
    if (noteDirectoryPathChanged)
    {
        emit activeNoteDirectoryPathChanged();
    }
    if (previousHasActiveNote != hasActiveNote())
    {
        emit hasActiveNoteChanged();
    }
    if (entryChanged || noteIdChanged || noteDirectoryPathChanged)
    {
        emit activeNoteStateChanged();
    }
}
