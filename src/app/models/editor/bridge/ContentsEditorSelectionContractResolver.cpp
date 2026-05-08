#include "app/models/editor/bridge/ContentsEditorSelectionContractResolver.hpp"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMetaObject>
#include <QMetaProperty>
#include <QString>
#include <QVariant>

bool ContentsEditorSelectionContractResolver::noteBackedSelectionEnabled(const QObject* noteListModel)
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

bool ContentsEditorSelectionContractResolver::hasReadableProperty(const QObject* object, const char* propertyName)
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

bool ContentsEditorSelectionContractResolver::hasInvokableMethod(
    const QObject* object,
    const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    const QMetaObject* metaObject = object->metaObject();
    if (metaObject == nullptr)
    {
        return false;
    }

    return metaObject->indexOfMethod(QMetaObject::normalizedSignature(methodSignature).constData()) >= 0;
}

QString ContentsEditorSelectionContractResolver::readStringProperty(
    const QObject* object,
    const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return {};
    }

    return object->property(propertyName).toString();
}

int ContentsEditorSelectionContractResolver::readIntProperty(const QObject* object, const char* propertyName)
{
    if (!hasReadableProperty(object, propertyName))
    {
        return -1;
    }

    bool converted = false;
    const int value = object->property(propertyName).toInt(&converted);
    return converted ? value : -1;
}

QVariantMap ContentsEditorSelectionContractResolver::currentNoteEntry(const QObject* noteListModel)
{
    if (!hasReadableProperty(noteListModel, "currentNoteEntry"))
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

QString ContentsEditorSelectionContractResolver::noteIdFromEntry(const QVariantMap& noteEntry)
{
    const QString noteId = noteEntry.value(QStringLiteral("noteId")).toString().trimmed();
    if (!noteId.isEmpty())
    {
        return noteId;
    }

    return noteEntry.value(QStringLiteral("id")).toString().trimmed();
}

QString ContentsEditorSelectionContractResolver::noteDirectoryPathFromEntry(const QVariantMap& noteEntry)
{
    return noteEntry.value(QStringLiteral("noteDirectoryPath")).toString().trimmed();
}

QString ContentsEditorSelectionContractResolver::noteIdFromModelRow(const QObject* noteListModel, int row)
{
    const auto* itemModel = qobject_cast<const QAbstractItemModel*>(noteListModel);
    if (itemModel == nullptr || row < 0 || row >= itemModel->rowCount())
    {
        return {};
    }

    const QModelIndex modelIndex = itemModel->index(row, 0);
    if (!modelIndex.isValid())
    {
        return {};
    }

    const QString noteId = itemModel->data(modelIndex, Qt::UserRole + 2).toString().trimmed();
    if (!noteId.isEmpty())
    {
        return noteId;
    }

    return itemModel->data(modelIndex, Qt::UserRole + 1).toString().trimmed();
}

QString ContentsEditorSelectionContractResolver::noteDirectoryPathFromModelRow(
    const QObject* noteListModel,
    int row)
{
    const auto* itemModel = qobject_cast<const QAbstractItemModel*>(noteListModel);
    if (itemModel == nullptr || row < 0 || row >= itemModel->rowCount())
    {
        return {};
    }

    const QModelIndex modelIndex = itemModel->index(row, 0);
    if (!modelIndex.isValid())
    {
        return {};
    }

    const QVariant noteDirectoryPath = itemModel->data(modelIndex, Qt::UserRole + 3);
    return noteDirectoryPath.toString().trimmed();
}
