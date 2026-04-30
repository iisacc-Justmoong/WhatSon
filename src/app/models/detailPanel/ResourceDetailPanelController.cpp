#include "app/models/detailPanel/ResourceDetailPanelController.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/hierarchy/resources/ResourcesListModel.hpp"

ResourceDetailPanelController::ResourceDetailPanelController(QObject* parent)
    : QObject(parent)
{
}

ResourceDetailPanelController::~ResourceDetailPanelController() = default;

QObject* ResourceDetailPanelController::resourceListModel() const noexcept
{
    return m_resourceListModel.data();
}

bool ResourceDetailPanelController::resourceContextLinked() const noexcept
{
    return qobject_cast<ResourcesListModel*>(m_resourceListModel.data()) != nullptr;
}

QVariantMap ResourceDetailPanelController::currentResourceEntry() const
{
    const auto* resourcesListModel = qobject_cast<ResourcesListModel*>(m_resourceListModel.data());
    return resourcesListModel != nullptr ? resourcesListModel->currentResourceEntry() : QVariantMap{};
}

void ResourceDetailPanelController::setCurrentResourceListModel(QObject* resourceListModel)
{
    if (resourceListModel != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("ResourceDetailPanelController::setCurrentResourceListModel")))
    {
        return;
    }

    if (resourceListModel == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("ResourceDetailPanelController::setCurrentResourceListModel")))
    {
        return;
    }

    if (m_resourceListModel == resourceListModel)
    {
        return;
    }

    disconnectResourceListModelSignals();
    m_resourceListModel = resourceListModel;

    if (m_resourceListModel != nullptr)
    {
        m_resourceListModelDestroyedConnection = connect(
            m_resourceListModel,
            &QObject::destroyed,
            this,
            &ResourceDetailPanelController::clearCurrentResourceListModel);

        if (auto* resourcesListModel = qobject_cast<ResourcesListModel*>(m_resourceListModel.data()))
        {
            m_currentResourceEntryChangedConnection = connect(
                resourcesListModel,
                &ResourcesListModel::currentResourceEntryChanged,
                this,
                &ResourceDetailPanelController::currentResourceEntryChanged);
            m_currentIndexChangedConnection = connect(
                resourcesListModel,
                &ResourcesListModel::currentIndexChanged,
                this,
                &ResourceDetailPanelController::currentResourceEntryChanged);
        }
    }

    emit resourceListModelChanged();
    emit resourceContextLinkedChanged();
    emit currentResourceEntryChanged();
}

void ResourceDetailPanelController::clearCurrentResourceListModel()
{
    if (m_resourceListModel == nullptr)
    {
        return;
    }

    disconnectResourceListModelSignals();
    m_resourceListModel = nullptr;
    emit resourceListModelChanged();
    emit resourceContextLinkedChanged();
    emit currentResourceEntryChanged();
}

void ResourceDetailPanelController::disconnectResourceListModelSignals()
{
    if (m_resourceListModelDestroyedConnection)
    {
        disconnect(m_resourceListModelDestroyedConnection);
        m_resourceListModelDestroyedConnection = QMetaObject::Connection();
    }

    if (m_currentResourceEntryChangedConnection)
    {
        disconnect(m_currentResourceEntryChangedConnection);
        m_currentResourceEntryChangedConnection = QMetaObject::Connection();
    }

    if (m_currentIndexChangedConnection)
    {
        disconnect(m_currentIndexChangedConnection);
        m_currentIndexChangedConnection = QMetaObject::Connection();
    }
}
