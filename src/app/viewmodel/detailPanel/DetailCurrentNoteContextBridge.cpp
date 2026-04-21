#include "app/viewmodel/detailPanel/DetailCurrentNoteContextBridge.hpp"

#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>

namespace
{
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

    QString readCurrentNoteId(const QObject* noteListModel, bool* resolvedFromModel)
    {
        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = false;
        }

        if (noteListModel == nullptr)
        {
            return {};
        }

        if (!noteBackedSelectionEnabled(noteListModel))
        {
            if (resolvedFromModel != nullptr)
            {
                *resolvedFromModel = true;
            }
            return {};
        }

        const QMetaObject* metaObject = noteListModel->metaObject();
        if (metaObject == nullptr || metaObject->indexOfProperty("currentNoteId") < 0)
        {
            return {};
        }

        if (resolvedFromModel != nullptr)
        {
            *resolvedFromModel = true;
        }
        return noteListModel->property("currentNoteId").toString().trimmed();
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
    m_noteListModel = noteListModel;
    if (m_noteListModel != nullptr)
    {
        QObject::connect(m_noteListModel, SIGNAL(currentNoteIdChanged()), this, SLOT(refreshContext()), Qt::UniqueConnection);
        QObject::connect(m_noteListModel, &QObject::destroyed, this, &DetailCurrentNoteContextBridge::refreshContext, Qt::UniqueConnection);
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
    bool noteIdResolvedFromModel = false;
    QString nextNoteId = readCurrentNoteId(m_noteListModel.data(), &noteIdResolvedFromModel);
    if (!noteIdResolvedFromModel)
    {
        nextNoteId = m_currentNoteId;
    }

    QString nextDirectoryPath;
    bool directoryResolved = false;
    if (hasNoteDirectoryResolver(m_noteDirectorySourceViewModel.data()) && !nextNoteId.isEmpty())
    {
        directoryResolved = QMetaObject::invokeMethod(
            m_noteDirectorySourceViewModel,
            "noteDirectoryPathForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, nextDirectoryPath),
            Q_ARG(QString, nextNoteId));
        nextDirectoryPath = nextDirectoryPath.trimmed();
    }
    if (!directoryResolved)
    {
        nextDirectoryPath = nextNoteId == m_currentNoteId ? m_currentNoteDirectoryPath : QString();
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
