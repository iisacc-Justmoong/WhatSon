#include "app/models/panel/HierarchyInteractionBridge.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/hierarchy/IHierarchyCapabilities.hpp"

#include <QVariantMap>

HierarchyInteractionBridge::HierarchyInteractionBridge(QObject* parent)
    : QObject(parent)
{
}

HierarchyInteractionBridge::~HierarchyInteractionBridge() = default;

QObject* HierarchyInteractionBridge::hierarchyController() const noexcept
{
    return m_hierarchyController;
}

void HierarchyInteractionBridge::setHierarchyController(QObject* controller)
{
    if (controller != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("HierarchyInteractionBridge::setHierarchyController")))
    {
        return;
    }

    if (controller == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("HierarchyInteractionBridge::setHierarchyController")))
    {
        return;
    }

    IHierarchyController* interfaceController = qobject_cast<IHierarchyController*>(controller);
    if (m_hierarchyController == interfaceController)
    {
        return;
    }

    disconnectHierarchyController();
    m_hierarchyController = interfaceController;

    if (m_hierarchyController != nullptr)
    {
        m_hierarchyControllerDestroyedConnection = connect(
            m_hierarchyController,
            &QObject::destroyed,
            this,
            &HierarchyInteractionBridge::handleHierarchyControllerDestroyed);
        m_hierarchySelectionChangedConnection = connect(
            m_hierarchyController,
            &IHierarchyController::hierarchySelectionChanged,
            this,
            &HierarchyInteractionBridge::handleHierarchyStateChanged);
        m_hierarchyNodesChangedConnection = connect(
            m_hierarchyController,
            &IHierarchyController::hierarchyNodesChanged,
            this,
            &HierarchyInteractionBridge::handleHierarchyStateChanged);
    }

    emit hierarchyControllerChanged();
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
    const auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyController);
    return renameCapability != nullptr && renameCapability->renameEnabled() && renameCapability->canRenameItem(index);
}

bool HierarchyInteractionBridge::renameItem(int index, const QString& displayName)
{
    auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyController);
    if (renameCapability == nullptr || !renameCapability->renameEnabled())
    {
        return false;
    }

    return renameCapability->renameItem(index, displayName);
}

void HierarchyInteractionBridge::createFolder()
{
    auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyController);
    if (crudCapability == nullptr || !crudCapability->createFolderEnabled())
    {
        return;
    }

    crudCapability->createFolder();
}

void HierarchyInteractionBridge::deleteSelectedFolder()
{
    auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyController);
    if (crudCapability == nullptr || !crudCapability->deleteFolderEnabled())
    {
        return;
    }

    crudCapability->deleteSelectedFolder();
}

bool HierarchyInteractionBridge::setItemExpanded(int index, bool expanded)
{
    auto* expansionCapability = qobject_cast<IHierarchyExpansionCapability*>(m_hierarchyController);
    if (expansionCapability == nullptr)
    {
        return false;
    }

    return expansionCapability->setItemExpanded(index, expanded);
}

bool HierarchyInteractionBridge::setAllItemsExpanded(bool expanded)
{
    if (m_hierarchyController == nullptr)
    {
        return false;
    }

    bool handled = false;
    if (QMetaObject::invokeMethod(
            m_hierarchyController.data(),
            "setAllItemsExpanded",
            Qt::DirectConnection,
            Q_RETURN_ARG(bool, handled),
            Q_ARG(bool, expanded)))
    {
        return handled;
    }

    auto* expansionCapability = qobject_cast<IHierarchyExpansionCapability*>(m_hierarchyController);
    if (expansionCapability == nullptr)
    {
        return false;
    }

    const QVariantList hierarchyNodes = m_hierarchyController->hierarchyNodes();
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

void HierarchyInteractionBridge::handleHierarchyControllerDestroyed()
{
    disconnectHierarchyController();
    m_hierarchyController = nullptr;
    emit hierarchyControllerChanged();
    refreshCapabilityState();
}

void HierarchyInteractionBridge::handleHierarchyStateChanged()
{
    refreshCapabilityState();
}

void HierarchyInteractionBridge::disconnectHierarchyController()
{
    if (m_hierarchyControllerDestroyedConnection)
    {
        disconnect(m_hierarchyControllerDestroyedConnection);
        m_hierarchyControllerDestroyedConnection = {};
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
    const auto* renameCapability = qobject_cast<IHierarchyRenameCapability*>(m_hierarchyController);
    const auto* crudCapability = qobject_cast<IHierarchyCrudCapability*>(m_hierarchyController);

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
