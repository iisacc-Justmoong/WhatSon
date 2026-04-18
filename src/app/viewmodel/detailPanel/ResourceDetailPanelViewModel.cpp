#include "ResourceDetailPanelViewModel.hpp"

#include "viewmodel/hierarchy/resources/ResourcesListModel.hpp"

ResourceDetailPanelViewModel::ResourceDetailPanelViewModel(QObject* parent)
    : QObject(parent)
{
}

ResourceDetailPanelViewModel::~ResourceDetailPanelViewModel() = default;

QObject* ResourceDetailPanelViewModel::resourceListModel() const noexcept
{
    return m_resourceListModel.data();
}

bool ResourceDetailPanelViewModel::resourceContextLinked() const noexcept
{
    return qobject_cast<ResourcesListModel*>(m_resourceListModel.data()) != nullptr;
}

QVariantMap ResourceDetailPanelViewModel::currentResourceEntry() const
{
    const auto* resourcesListModel = qobject_cast<ResourcesListModel*>(m_resourceListModel.data());
    return resourcesListModel != nullptr ? resourcesListModel->currentResourceEntry() : QVariantMap{};
}

void ResourceDetailPanelViewModel::setCurrentResourceListModel(QObject* resourceListModel)
{
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
            &ResourceDetailPanelViewModel::clearCurrentResourceListModel);

        if (auto* resourcesListModel = qobject_cast<ResourcesListModel*>(m_resourceListModel.data()))
        {
            m_currentResourceEntryChangedConnection = connect(
                resourcesListModel,
                &ResourcesListModel::currentResourceEntryChanged,
                this,
                &ResourceDetailPanelViewModel::currentResourceEntryChanged);
            m_currentIndexChangedConnection = connect(
                resourcesListModel,
                &ResourcesListModel::currentIndexChanged,
                this,
                &ResourceDetailPanelViewModel::currentResourceEntryChanged);
        }
    }

    emit resourceListModelChanged();
    emit resourceContextLinkedChanged();
    emit currentResourceEntryChanged();
}

void ResourceDetailPanelViewModel::clearCurrentResourceListModel()
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

void ResourceDetailPanelViewModel::disconnectResourceListModelSignals()
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
