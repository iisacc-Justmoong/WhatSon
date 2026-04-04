#include "NoteListModelContractBridge.hpp"

#include <QAbstractItemModel>
#include <QMetaObject>
#include <QMetaProperty>

#include <algorithm>

namespace
{
    constexpr auto kSetSearchTextSignature = "setSearchText(QString)";
    constexpr auto kSetCurrentIndexSignature = "setCurrentIndex(int)";

    int roleForName(const QAbstractItemModel* model, const QByteArray& roleName)
    {
        if (model == nullptr || roleName.isEmpty())
        {
            return -1;
        }

        const QHash<int, QByteArray> roleNames = model->roleNames();
        for (auto iterator = roleNames.constBegin(); iterator != roleNames.constEnd(); ++iterator)
        {
            if (iterator.value() == roleName)
            {
                return iterator.key();
            }
        }

        return -1;
    }
}

NoteListModelContractBridge::NoteListModelContractBridge(QObject* parent)
    : QObject(parent)
{
}

NoteListModelContractBridge::~NoteListModelContractBridge() = default;

QObject* NoteListModelContractBridge::noteListModel() const noexcept
{
    return m_noteListModel;
}

void NoteListModelContractBridge::setNoteListModel(QObject* model)
{
    if (m_noteListModel == model)
    {
        return;
    }

    const bool hadNoteListModel = hasNoteListModel();
    disconnectNoteListModel();
    m_noteListModel = model;

    if (m_noteListModel != nullptr)
    {
        m_noteListDestroyedConnection = connect(
            m_noteListModel,
            &QObject::destroyed,
            this,
            &NoteListModelContractBridge::handleNoteListDestroyed);
        m_currentIndexChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentIndexChanged()),
            this,
            SLOT(handleCurrentIndexChanged()));
        m_currentNoteIdChangedConnection = connect(
            m_noteListModel,
            SIGNAL(currentNoteIdChanged()),
            this,
            SLOT(handleCurrentNoteIdChanged()));
    }

    emit noteListModelChanged();
    if (hadNoteListModel != hasNoteListModel())
    {
        emit hasNoteListModelChanged();
    }
    refreshContracts();
    emit currentIndexChanged();
    emit currentNoteIdChanged();
}

bool NoteListModelContractBridge::hasNoteListModel() const noexcept
{
    return m_noteListModel != nullptr;
}

bool NoteListModelContractBridge::searchContractAvailable() const noexcept
{
    return m_searchContractAvailable;
}

bool NoteListModelContractBridge::currentIndexContractAvailable() const noexcept
{
    return m_currentIndexContractAvailable;
}

int NoteListModelContractBridge::currentIndex() const
{
    return readIntProperty(m_noteListModel, "currentIndex", -1);
}

QString NoteListModelContractBridge::currentNoteId() const
{
    return readStringProperty(m_noteListModel, "currentNoteId").trimmed();
}

bool NoteListModelContractBridge::applySearchText(const QString& searchText)
{
    if (m_noteListModel == nullptr)
    {
        return false;
    }

    if (hasWritableProperty(m_noteListModel, "searchText"))
    {
        return m_noteListModel->setProperty("searchText", searchText);
    }

    if (hasInvokableMethod(m_noteListModel, kSetSearchTextSignature))
    {
        return QMetaObject::invokeMethod(
            m_noteListModel,
            "setSearchText",
            Qt::DirectConnection,
            Q_ARG(QString, searchText));
    }

    return false;
}

int NoteListModelContractBridge::readCurrentIndex() const
{
    return currentIndex();
}

QString NoteListModelContractBridge::readCurrentNoteId() const
{
    return currentNoteId();
}

QString NoteListModelContractBridge::readNoteIdAt(int index) const
{
    const auto* model = qobject_cast<QAbstractItemModel*>(m_noteListModel.data());
    if (model == nullptr)
    {
        return {};
    }

    const int normalizedIndex = std::max(-1, index);
    if (normalizedIndex < 0 || normalizedIndex >= model->rowCount())
    {
        return {};
    }

    const QModelIndex modelIndex = model->index(normalizedIndex, 0);
    if (!modelIndex.isValid())
    {
        return {};
    }

    int noteIdRole = roleForName(model, QByteArrayLiteral("noteId"));
    if (noteIdRole < 0)
    {
        noteIdRole = roleForName(model, QByteArrayLiteral("id"));
    }
    if (noteIdRole < 0)
    {
        return {};
    }

    return modelIndex.data(noteIdRole).toString().trimmed();
}

bool NoteListModelContractBridge::pushCurrentIndex(int index)
{
    if (m_noteListModel == nullptr)
    {
        return false;
    }

    const int normalizedIndex = std::max(-1, index);
    if (hasWritableProperty(m_noteListModel, "currentIndex"))
    {
        if (currentIndex() == normalizedIndex)
        {
            return true;
        }
        return m_noteListModel->setProperty("currentIndex", normalizedIndex);
    }

    if (hasInvokableMethod(m_noteListModel, kSetCurrentIndexSignature))
    {
        return QMetaObject::invokeMethod(
            m_noteListModel,
            "setCurrentIndex",
            Qt::DirectConnection,
            Q_ARG(int, normalizedIndex));
    }

    return false;
}

void NoteListModelContractBridge::handleCurrentIndexChanged()
{
    emit currentIndexChanged();
}

void NoteListModelContractBridge::handleCurrentNoteIdChanged()
{
    emit currentNoteIdChanged();
}

void NoteListModelContractBridge::handleNoteListDestroyed()
{
    disconnectNoteListModel();
    m_noteListModel = nullptr;
    emit noteListModelChanged();
    emit hasNoteListModelChanged();
    refreshContracts();
    emit currentIndexChanged();
    emit currentNoteIdChanged();
}

bool NoteListModelContractBridge::hasReadableProperty(const QObject* object, const char* propertyName)
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

bool NoteListModelContractBridge::hasWritableProperty(const QObject* object, const char* propertyName)
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

    return object->metaObject()->property(propertyIndex).isWritable();
}

bool NoteListModelContractBridge::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

QString NoteListModelContractBridge::readStringProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return {};
    }

    return object->property(propertyName).toString();
}

int NoteListModelContractBridge::readIntProperty(
    const QObject* object,
    const char* propertyName,
    int fallbackValue)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return fallbackValue;
    }

    bool converted = false;
    const int propertyValue = object->property(propertyName).toInt(&converted);
    if (!converted)
    {
        return fallbackValue;
    }
    return propertyValue;
}

void NoteListModelContractBridge::refreshContracts()
{
    const bool nextSearchContractAvailable = hasWritableProperty(m_noteListModel, "searchText")
        || hasInvokableMethod(m_noteListModel, kSetSearchTextSignature);
    if (m_searchContractAvailable != nextSearchContractAvailable)
    {
        m_searchContractAvailable = nextSearchContractAvailable;
        emit searchContractAvailableChanged();
    }

    const bool nextCurrentIndexContractAvailable = hasReadableProperty(m_noteListModel, "currentIndex")
        || hasWritableProperty(m_noteListModel, "currentIndex")
        || hasInvokableMethod(m_noteListModel, kSetCurrentIndexSignature);
    if (m_currentIndexContractAvailable != nextCurrentIndexContractAvailable)
    {
        m_currentIndexContractAvailable = nextCurrentIndexContractAvailable;
        emit currentIndexContractAvailableChanged();
    }
}

void NoteListModelContractBridge::disconnectNoteListModel()
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
}
