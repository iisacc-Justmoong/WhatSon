#include "HierarchyInteractionBridge.hpp"

#include "policy/ArchitecturePolicyLock.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"

#include <QVariantMap>

HierarchyInteractionBridge::HierarchyInteractionBridge(QObject* parent)
    : QObject(parent)
{
}

HierarchyInteractionBridge::~HierarchyInteractionBridge() = default;

QObject* HierarchyInteractionBridge::hierarchyViewModel() const noexcept
{
    return m_hierarchyViewModel;
}

void HierarchyInteractionBridge::setHierarchyViewModel(QObject* viewModel)
{
    if (viewModel != nullptr
        && !WhatSon::Policy::verifyDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::ViewModel,
            QStringLiteral("HierarchyInteractionBridge::setHierarchyViewModel")))
    {
        return;
    }

    IHierarchyViewModel* interfaceViewModel = qobject_cast<IHierarchyViewModel*>(viewModel);
    if (m_hierarchyViewModel == interfaceViewModel)
    {
        return;
    }

    disconnectHierarchyViewModel();
    m_hierarchyViewModel = interfaceViewModel;

    if (m_hierarchyViewModel != nullptr)
    {
        m_hierarchyViewModelDestroyedConnection = connect(
            m_hierarchyViewModel,
            &QObject::destroyed,
            this,
            &HierarchyInteractionBridge::handleHierarchyViewModelDestroyed);
        m_hierarchySelectionChangedConnection = connect(
            m_hierarchyViewModel,
            &IHierarchyViewModel::hierarchySelectionChanged,
            this,
            &HierarchyInteractionBridge::handleHierarchyStateChanged);
        m_hierarchyNodesChangedConnection = connect(
            m_hierarchyViewModel,
            &IHierarchyViewModel::hierarchyNodesChanged,
            this,
            &HierarchyInteractionBridge::handleHierarchyStateChanged);
    }

    emit hierarchyViewModelChanged();
    refreshCapabilityState();
}

bool HierarchyInteractionBridge::renameContractAvailable() const noexcept
{
    return m_renameContractAvailable;
}

bool HierarchyInteractionBridge::createFolderEnabled() const noexcept
{
    return m_createFolderEnabled;
}

bool HierarchyInteractionBridge::deleteFolderEnabled() const noexcept
{
    return m_deleteFolderEnabled;
}

bool HierarchyInteractionBridge::viewOptionsEnabled() const noexcept
{
    return m_viewOptionsEnabled;
}

bool HierarchyInteractionBridge::canRenameItem(int index) const
{
    const auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyViewModel);
    return renameCapability != nullptr && renameCapability->renameEnabled() && renameCapability->canRenameItem(index);
}

bool HierarchyInteractionBridge::renameItem(int index, const QString& displayName)
{
    auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyViewModel);
    if (renameCapability == nullptr || !renameCapability->renameEnabled())
    {
        return false;
    }

    return renameCapability->renameItem(index, displayName);
}

void HierarchyInteractionBridge::createFolder()
{
    auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyViewModel);
    if (crudCapability == nullptr || !crudCapability->createFolderEnabled())
    {
        return;
    }

    crudCapability->createFolder();
}

void HierarchyInteractionBridge::deleteSelectedFolder()
{
    auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyViewModel);
    if (crudCapability == nullptr || !crudCapability->deleteFolderEnabled())
    {
        return;
    }

    crudCapability->deleteSelectedFolder();
}

bool HierarchyInteractionBridge::setItemExpanded(int index, bool expanded)
{
    auto* expansionCapability = qobject_cast<IHierarchyExpansionCapability*>(m_hierarchyViewModel);
    if (expansionCapability == nullptr)
    {
        return false;
    }

    return expansionCapability->setItemExpanded(index, expanded);
}

bool HierarchyInteractionBridge::setAllItemsExpanded(bool expanded)
{
    if (m_hierarchyViewModel == nullptr)
    {
        return false;
    }

    bool handled = false;
    if (QMetaObject::invokeMethod(
            m_hierarchyViewModel.data(),
            "setAllItemsExpanded",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, handled),
            Q_ARG(bool, expanded)))
    {
        return handled;
    }

    auto* expansionCapability = qobject_cast<IHierarchyExpansionCapability*>(m_hierarchyViewModel);
    if (expansionCapability == nullptr)
    {
        return false;
    }

    const QVariantList hierarchyNodes = m_hierarchyViewModel->hierarchyNodes();
    bool hasExpandableItems = false;
    for (int index = 0; index < hierarchyNodes.size(); ++index)
    {
        const QVariantMap entryMap = hierarchyNodes.at(index).toMap();
        if (!entryMap.value(QStringLiteral("showChevron")).toBool())
        {
            continue;
        }

        hasExpandableItems = true;
        if (entryMap.value(QStringLiteral("expanded")).toBool() == expanded)
        {
            continue;
        }

        expansionCapability->setItemExpanded(index, expanded);
    }

    return hasExpandableItems || hierarchyNodes.isEmpty();
}

void HierarchyInteractionBridge::handleHierarchyViewModelDestroyed()
{
    disconnectHierarchyViewModel();
    m_hierarchyViewModel = nullptr;
    emit hierarchyViewModelChanged();
    refreshCapabilityState();
}

void HierarchyInteractionBridge::handleHierarchyStateChanged()
{
    refreshCapabilityState();
}

void HierarchyInteractionBridge::disconnectHierarchyViewModel()
{
    if (m_hierarchyViewModelDestroyedConnection)
    {
        disconnect(m_hierarchyViewModelDestroyedConnection);
        m_hierarchyViewModelDestroyedConnection = {};
    }
    if (m_hierarchySelectionChangedConnection)
    {
        disconnect(m_hierarchySelectionChangedConnection);
        m_hierarchySelectionChangedConnection = {};
    }
    if (m_hierarchyNodesChangedConnection)
    {
        disconnect(m_hierarchyNodesChangedConnection);
        m_hierarchyNodesChangedConnection = {};
    }
}

bool HierarchyInteractionBridge::refreshCapabilityState()
{
    const auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyViewModel);
    const auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyViewModel);

    const bool nextRenameContractAvailable =
        renameCapability != nullptr && renameCapability->renameEnabled();
    const bool nextCreateFolderEnabled =
        crudCapability != nullptr && crudCapability->createFolderEnabled();
    const bool nextDeleteFolderEnabled =
        crudCapability != nullptr && crudCapability->deleteFolderEnabled();
    const bool nextViewOptionsEnabled =
        crudCapability == nullptr || crudCapability->viewOptionsEnabled();

    if (m_renameContractAvailable == nextRenameContractAvailable
        && m_createFolderEnabled == nextCreateFolderEnabled
        && m_deleteFolderEnabled == nextDeleteFolderEnabled
        && m_viewOptionsEnabled == nextViewOptionsEnabled)
    {
        return false;
    }

    m_renameContractAvailable = nextRenameContractAvailable;
    m_createFolderEnabled = nextCreateFolderEnabled;
    m_deleteFolderEnabled = nextDeleteFolderEnabled;
    m_viewOptionsEnabled = nextViewOptionsEnabled;
    emit capabilityStateChanged();
    return true;
}
