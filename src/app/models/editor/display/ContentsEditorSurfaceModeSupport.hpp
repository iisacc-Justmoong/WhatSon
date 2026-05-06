#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsEditorSurfaceModeSupport : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorSurfaceModeSupport)
    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged FINAL)
    Q_PROPERTY(bool resourceEditorVisible READ resourceEditorVisible NOTIFY surfaceModeChanged FINAL)
    Q_PROPERTY(QVariantMap currentResourceEntry READ currentResourceEntry NOTIFY surfaceModeChanged FINAL)

public:
    explicit ContentsEditorSurfaceModeSupport(QObject* parent = nullptr);

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* value);

    bool resourceEditorVisible() const;
    QVariantMap currentResourceEntry() const;

    Q_INVOKABLE bool hasCurrentResourceEntry(QObject* noteListModel) const;
    Q_INVOKABLE QVariantMap currentResourceEntryFor(QObject* noteListModel) const;
    Q_INVOKABLE bool resourceEditorVisibleFor(QObject* noteListModel) const;

signals:
    void noteListModelChanged();
    void surfaceModeChanged();

private:
    bool connectPropertyNotify(QObject* source, const char* propertyName);

    QPointer<QObject> m_noteListModel;
};
