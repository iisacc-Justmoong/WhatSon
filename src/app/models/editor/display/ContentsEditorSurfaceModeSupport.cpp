#include "app/models/editor/display/ContentsEditorSurfaceModeSupport.hpp"

#include <QMetaProperty>
#include <QVariant>

ContentsEditorSurfaceModeSupport::ContentsEditorSurfaceModeSupport(QObject* parent)
    : QObject(parent)
{
}

QObject* ContentsEditorSurfaceModeSupport::noteListModel() const noexcept
{
    return m_noteListModel.data();
}

void ContentsEditorSurfaceModeSupport::setNoteListModel(QObject* value)
{
    if (m_noteListModel == value)
        return;

    if (m_noteListModel)
        QObject::disconnect(m_noteListModel, nullptr, this, nullptr);

    m_noteListModel = value;
    if (m_noteListModel)
    {
        connectPropertyNotify(m_noteListModel, "currentResourceEntry");
        connectPropertyNotify(m_noteListModel, "noteBacked");
        QObject::connect(m_noteListModel, &QObject::destroyed, this, [this]() {
            if (m_noteListModel)
                return;
            emit noteListModelChanged();
            emit surfaceModeChanged();
        });
    }

    emit noteListModelChanged();
    emit surfaceModeChanged();
}

bool ContentsEditorSurfaceModeSupport::resourceEditorVisible() const
{
    return resourceEditorVisibleFor(m_noteListModel);
}

QVariantMap ContentsEditorSurfaceModeSupport::currentResourceEntry() const
{
    return currentResourceEntryFor(m_noteListModel);
}

bool ContentsEditorSurfaceModeSupport::hasCurrentResourceEntry(QObject* noteListModel) const
{
    if (!noteListModel)
        return false;
    if (noteListModel->property("noteBacked").toBool())
        return false;
    return !noteListModel->property("currentResourceEntry").toMap().isEmpty();
}

QVariantMap ContentsEditorSurfaceModeSupport::currentResourceEntryFor(QObject* noteListModel) const
{
    if (!hasCurrentResourceEntry(noteListModel))
        return {};
    return noteListModel->property("currentResourceEntry").toMap();
}

bool ContentsEditorSurfaceModeSupport::resourceEditorVisibleFor(QObject* noteListModel) const
{
    return hasCurrentResourceEntry(noteListModel);
}

bool ContentsEditorSurfaceModeSupport::connectPropertyNotify(QObject* source, const char* propertyName)
{
    if (!source || !propertyName)
        return false;

    const QMetaObject* sourceMetaObject = source->metaObject();
    const int propertyIndex = sourceMetaObject->indexOfProperty(propertyName);
    if (propertyIndex < 0)
        return false;

    const QMetaProperty property = sourceMetaObject->property(propertyIndex);
    if (!property.hasNotifySignal())
        return false;

    const int signalIndex = metaObject()->indexOfSignal("surfaceModeChanged()");
    if (signalIndex < 0)
        return false;

    return QObject::connect(
        source,
        property.notifySignal(),
        this,
        metaObject()->method(signalIndex),
        Qt::UniqueConnection);
}
