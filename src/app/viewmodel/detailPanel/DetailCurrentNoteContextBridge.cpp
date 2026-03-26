#include "DetailCurrentNoteContextBridge.hpp"

#include <QMetaObject>
#include <QVariant>

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
    const QString nextNoteId = m_noteListModel != nullptr
        ? m_noteListModel->property("currentNoteId").toString().trimmed()
        : QString();

    QString nextDirectoryPath;
    if (m_noteDirectorySourceViewModel != nullptr && !nextNoteId.isEmpty())
    {
        QMetaObject::invokeMethod(
            m_noteDirectorySourceViewModel,
            "noteDirectoryPathForNoteId",
            Qt::DirectConnection,
            Q_RETURN_ARG(QString, nextDirectoryPath),
            Q_ARG(QString, nextNoteId));
        nextDirectoryPath = nextDirectoryPath.trimmed();
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
