#pragma once

#include <QMetaObject>
#include <QPointer>
#include <QVariantMap>

class QObject;

class ResourceDetailPanelController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* resourceListModel READ resourceListModel NOTIFY resourceListModelChanged)
    Q_PROPERTY(bool resourceContextLinked READ resourceContextLinked NOTIFY resourceContextLinkedChanged)
    Q_PROPERTY(QVariantMap currentResourceEntry READ currentResourceEntry NOTIFY currentResourceEntryChanged)

public:
    explicit ResourceDetailPanelController(QObject* parent = nullptr);
    ~ResourceDetailPanelController() override;

    QObject* resourceListModel() const noexcept;
    bool resourceContextLinked() const noexcept;
    QVariantMap currentResourceEntry() const;

    void setCurrentResourceListModel(QObject* resourceListModel);

signals:
    void resourceListModelChanged();
    void resourceContextLinkedChanged();
    void currentResourceEntryChanged();

private slots:
    void clearCurrentResourceListModel();

private:
    void disconnectResourceListModelSignals();

    QPointer<QObject> m_resourceListModel;
    QMetaObject::Connection m_resourceListModelDestroyedConnection;
    QMetaObject::Connection m_currentResourceEntryChangedConnection;
    QMetaObject::Connection m_currentIndexChangedConnection;
};
