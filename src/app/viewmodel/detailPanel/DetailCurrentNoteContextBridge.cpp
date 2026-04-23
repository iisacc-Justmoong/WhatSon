#include "app/viewmodel/detailPanel/DetailCurrentNoteContextBridge.hpp"

#include <QDir>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>

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

    bool hasNoteDirectoryResolver(const QObject* sourceViewModel)
    {
        if (sourceViewModel == nullptr)
        {
            return false;
        }

        const QMetaObject* metaObject = sourceViewModel->metaObject();
        return metaObject != nullptr && metaObject->indexOfMethod("noteDirectoryPathForNoteId(QString)") >= 0;
    }

    QVariantMap readCurrentNoteEntry(const QObject* noteListModel)
    {
        if (noteListModel == nullptr)
        {
            return {};
        }

        if (!hasReadableProperty(noteListModel, "currentNoteEntry"))
        {
            return {};
        }

        QVariantMap noteEntry = noteListModel->property("currentNoteEntry").toMap();
        if (!noteEntry.contains(QStringLiteral("noteId")) && noteEntry.contains(QStringLiteral("id")))
        {
            noteEntry.insert(QStringLiteral("noteId"), noteEntry.value(QStringLiteral("id")));
        }

        const QString currentNoteDirectoryPath = normalizeNoteDirectoryPath(
            noteListModel->property("currentNoteDirectoryPath").toString());
        if (!noteEntry.contains(QStringLiteral("noteDirectoryPath")) && !currentNoteDirectoryPath.isEmpty())
        {
            noteEntry.insert(QStringLiteral("noteDirectoryPath"), currentNoteDirectoryPath);
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
        return normalizeNoteDirectoryPath(
            noteEntry.value(QStringLiteral("noteDirectoryPath")).toString());
    }

    QString readCurrentNoteIdProperty(const QObject* noteListModel, bool* resolvedFromModel)
    {
        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = false;
        }

        if (noteListModel == nullptr || !hasReadableProperty(noteListModel, "currentNoteId"))
        {
            return {};
        }

        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = true;
        }
        return noteListModel->property("currentNoteId").toString().trimmed();
    }

    QString readCurrentNoteDirectoryPathProperty(const QObject* noteListModel, bool* resolvedFromModel)
    {
        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = false;
        }

        if (noteListModel == nullptr || !hasReadableProperty(noteListModel, "currentNoteDirectoryPath"))
        {
            return {};
        }

        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = true;
        }
        return normalizeNoteDirectoryPath(
            noteListModel->property("currentNoteDirectoryPath").toString());
    }
}

DetailCurrentNoteContextBridge::DetailCurrentNoteContextBridge(QObject* parent)
    : QObject(parent)
{
}

QObject* DetailCurrentNoteContextBridge::noteListModel() const noexcept
{
    return m_noteListModel.data();
}

void DetailCurrentNoteContextBridge::setNoteListModel(QObject* noteListModel)
{
    if (m_noteListModel == noteListModel)
    {
        return;
    }

    disconnectNoteListModelSignals();
    m_noteListModel = noteListModel;
    if (m_noteListModel != nullptr)
    {
        m_noteListModelDestroyedConnection = connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            &DetailCurrentNoteContextBridge::refreshContext);

        if (hasSignal(m_noteListModel, "currentIndexChanged()"))
        {
            m_currentIndexChangedConnection = connect(
                m_noteListModel,
                SIGNAL(currentIndexChanged()),
                this,
                SLOT(refreshContext()));
        }
        if (hasSignal(m_noteListModel, "currentNoteEntryChanged()"))
        {
            m_currentNoteEntryChangedConnection = connect(
                m_noteListModel,
                SIGNAL(currentNoteEntryChanged()),
                this,
                SLOT(refreshContext()));
        }
        if (hasSignal(m_noteListModel, "currentNoteIdChanged()"))
        {
            m_currentNoteIdChangedConnection = connect(
                m_noteListModel,
                SIGNAL(currentNoteIdChanged()),
                this,
                SLOT(refreshContext()));
        }
        if (hasSignal(m_noteListModel, "currentNoteDirectoryPathChanged()"))
        {
            m_currentNoteDirectoryPathChangedConnection = connect(
                m_noteListModel,
                SIGNAL(currentNoteDirectoryPathChanged()),
                this,
                SLOT(refreshContext()));
        }
    }
    emit noteListModelChanged();
    refreshContext();
}

QObject* DetailCurrentNoteContextBridge::noteDirectorySourceViewModel() const noexcept
{
    return m_noteDirectorySourceViewModel.data();
}

void DetailCurrentNoteContextBridge::setNoteDirectorySourceViewModel(QObject* sourceViewModel)
{
    if (m_noteDirectorySourceViewModel == sourceViewModel)
    {
        return;
    }
    m_noteDirectorySourceViewModel = sourceViewModel;
    emit noteDirectorySourceViewModelChanged();
    refreshContext();
}

QString DetailCurrentNoteContextBridge::currentNoteId() const
{
    return m_currentNoteId;
}

QString DetailCurrentNoteContextBridge::currentNoteDirectoryPath() const
{
    return m_currentNoteDirectoryPath;
}

void DetailCurrentNoteContextBridge::refreshContext()
{
    const QObject* noteListModel = m_noteListModel.data();
    QString nextNoteId;
    QString nextDirectoryPath;

    if (noteListModel != nullptr && !noteBackedSelectionEnabled(noteListModel))
    {
        nextNoteId.clear();
        nextDirectoryPath.clear();
    }
    else
    {
        const QVariantMap currentNoteEntry = readCurrentNoteEntry(noteListModel);

        bool noteIdResolvedFromModel = false;
        nextNoteId = noteIdFromEntry(currentNoteEntry);
        if (!nextNoteId.isEmpty())
        {
            noteIdResolvedFromModel = true;
        }
        else
        {
            nextNoteId = readCurrentNoteIdProperty(noteListModel, &noteIdResolvedFromModel);
        }
        if (!noteIdResolvedFromModel)
        {
            nextNoteId = m_currentNoteId;
        }

        bool directoryResolved = false;
        nextDirectoryPath = noteDirectoryPathFromEntry(currentNoteEntry);
        if (!nextDirectoryPath.isEmpty())
        {
            directoryResolved = true;
        }
        else
        {
            nextDirectoryPath = readCurrentNoteDirectoryPathProperty(noteListModel, &directoryResolved);
        }

        if (!directoryResolved && hasNoteDirectoryResolver(m_noteDirectorySourceViewModel.data()) && !nextNoteId.isEmpty())
        {
            directoryResolved = QMetaObject::invokeMethod(
                m_noteDirectorySourceViewModel,
                "noteDirectoryPathForNoteId",
                Qt::DirectConnection,
                Q_RETURN_ARG(QString, nextDirectoryPath),
                Q_ARG(QString, nextNoteId));
            nextDirectoryPath = normalizeNoteDirectoryPath(nextDirectoryPath);
        }
        if (!directoryResolved)
        {
            nextDirectoryPath = nextNoteId == m_currentNoteId ? m_currentNoteDirectoryPath : QString();
        }
    }

    if (m_currentNoteId != nextNoteId)
    {
        m_currentNoteId = nextNoteId;
        emit currentNoteIdChanged();
    }
    if (m_currentNoteDirectoryPath != nextDirectoryPath)
    {
        m_currentNoteDirectoryPath = nextDirectoryPath;
        emit currentNoteDirectoryPathChanged();
    }
}

void DetailCurrentNoteContextBridge::disconnectNoteListModelSignals()
{
    if (m_noteListModelDestroyedConnection)
    {
        disconnect(m_noteListModelDestroyedConnection);
        m_noteListModelDestroyedConnection = QMetaObject::Connection();
    }
    if (m_currentIndexChangedConnection)
    {
        disconnect(m_currentIndexChangedConnection);
        m_currentIndexChangedConnection = QMetaObject::Connection();
    }
    if (m_currentNoteEntryChangedConnection)
    {
        disconnect(m_currentNoteEntryChangedConnection);
        m_currentNoteEntryChangedConnection = QMetaObject::Connection();
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
}
