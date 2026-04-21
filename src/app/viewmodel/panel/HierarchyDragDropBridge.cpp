#include "app/viewmodel/panel/HierarchyDragDropBridge.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/viewmodel/hierarchy/IHierarchyCapabilities.hpp"

#include <QStringList>

namespace
{
    QStringList normalizedUniqueNoteIds(const QVariantList& noteIds)
    {
        QStringList normalized;
        normalized.reserve(noteIds.size());
        for (const QVariant& noteIdValue : noteIds)
        {
            const QString normalizedNoteId = noteIdValue.toString().trimmed();
            if (normalizedNoteId.isEmpty() || normalized.contains(normalizedNoteId))
            {
                continue;
            }

            normalized.push_back(normalizedNoteId);
        }

        return normalized;
    }
}

HierarchyDragDropBridge::HierarchyDragDropBridge(QObject* parent)
    : QObject(parent)
{
}

HierarchyDragDropBridge::~HierarchyDragDropBridge() = default;

QObject* HierarchyDragDropBridge::hierarchyViewModel() const noexcept
{
    return m_hierarchyViewModel;
}

void HierarchyDragDropBridge::setHierarchyViewModel(QObject* viewModel)
{
    if (viewModel != nullptr
        && !WhatSon::Policy::verifyDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::ViewModel,
            QStringLiteral("HierarchyDragDropBridge::setHierarchyViewModel")))
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
            &HierarchyDragDropBridge::handleHierarchyViewModelDestroyed);
        m_hierarchyModelChangedConnection = connect(
            m_hierarchyViewModel,
            &IHierarchyViewModel::hierarchyNodesChanged,
            this,
            &HierarchyDragDropBridge::handleHierarchyModelChanged);
        m_selectedIndexChangedConnection = connect(
            m_hierarchyViewModel,
            &IHierarchyViewModel::hierarchySelectionChanged,
            this,
            &HierarchyDragDropBridge::handleSelectedIndexChanged);
    }

    emit hierarchyViewModelChanged();
    refreshContractState();
    refreshSelectedItemKey();
}

bool HierarchyDragDropBridge::reorderContractAvailable() const noexcept
{
    return m_reorderContractAvailable;
}

bool HierarchyDragDropBridge::noteDropContractAvailable() const noexcept
{
    return m_noteDropContractAvailable;
}

QString HierarchyDragDropBridge::selectedItemKey() const
{
    return m_selectedItemKey;
}

bool HierarchyDragDropBridge::applyHierarchyReorder(
    const QVariantList& hierarchyNodes,
    const QString& activeItemKey)
{
    auto* reorderCapability = qobject_cast<IHierarchyReorderCapability*>(m_hierarchyViewModel);
    if (reorderCapability == nullptr || !m_reorderContractAvailable || hierarchyNodes.isEmpty())
    {
        return false;
    }

    return reorderCapability->applyHierarchyNodes(hierarchyNodes, resolvedActiveItemKey(activeItemKey));
}

bool HierarchyDragDropBridge::canAcceptNoteDrop(int index, const QString& noteId) const
{
    const auto* noteDropCapability = qobject_cast<IHierarchyNoteDropCapability*>(m_hierarchyViewModel);
    if (noteDropCapability == nullptr || !m_noteDropContractAvailable)
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (index < 0 || normalizedNoteId.isEmpty())
    {
        return false;
    }

    return noteDropCapability->canAcceptNoteDrop(index, normalizedNoteId);
}

bool HierarchyDragDropBridge::canAcceptNoteDropList(int index, const QVariantList& noteIds) const
{
    const auto* noteDropCapability = qobject_cast<IHierarchyNoteDropCapability*>(m_hierarchyViewModel);
    if (noteDropCapability == nullptr || !m_noteDropContractAvailable || index < 0)
    {
        return false;
    }

    const QStringList normalizedNoteIds = normalizedUniqueNoteIds(noteIds);
    for (const QString& noteId : normalizedNoteIds)
    {
        if (noteDropCapability->canAcceptNoteDrop(index, noteId))
        {
            return true;
        }
    }

    return false;
}

bool HierarchyDragDropBridge::assignNoteToFolder(int index, const QString& noteId)
{
    auto* noteDropCapability = qobject_cast<IHierarchyNoteDropCapability*>(m_hierarchyViewModel);
    if (noteDropCapability == nullptr || !m_noteDropContractAvailable)
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (index < 0 || normalizedNoteId.isEmpty())
    {
        return false;
    }

    return noteDropCapability->assignNoteToFolder(index, normalizedNoteId);
}

bool HierarchyDragDropBridge::assignNotesToFolder(int index, const QVariantList& noteIds)
{
    auto* noteDropCapability = qobject_cast<IHierarchyNoteDropCapability*>(m_hierarchyViewModel);
    if (noteDropCapability == nullptr || !m_noteDropContractAvailable || index < 0)
    {
        return false;
    }

    const QStringList normalizedNoteIds = normalizedUniqueNoteIds(noteIds);
    bool assignedAny = false;
    for (const QString& noteId : normalizedNoteIds)
    {
        if (!noteDropCapability->canAcceptNoteDrop(index, noteId))
        {
            continue;
        }

        if (noteDropCapability->assignNoteToFolder(index, noteId))
        {
            assignedAny = true;
        }
    }

    return assignedAny;
}

void HierarchyDragDropBridge::handleHierarchyModelChanged()
{
    refreshSelectedItemKey();
}

void HierarchyDragDropBridge::handleSelectedIndexChanged()
{
    refreshSelectedItemKey();
}

void HierarchyDragDropBridge::handleHierarchyViewModelDestroyed()
{
    disconnectHierarchyViewModel();
    m_hierarchyViewModel = nullptr;
    emit hierarchyViewModelChanged();
    refreshContractState();
    refreshSelectedItemKey();
}

QVariantList HierarchyDragDropBridge::readHierarchyModel() const
{
    return m_hierarchyViewModel ? m_hierarchyViewModel->hierarchyNodes() : QVariantList{};
}

QString HierarchyDragDropBridge::resolvedActiveItemKey(const QString& preferredActiveItemKey) const
{
    const QString normalizedPreferred = preferredActiveItemKey.trimmed();
    if (!normalizedPreferred.isEmpty())
    {
        return normalizedPreferred;
    }

    return m_selectedItemKey;
}

void HierarchyDragDropBridge::refreshContractState()
{
    const auto* reorderCapability = qobject_cast<IHierarchyReorderCapability*>(m_hierarchyViewModel);
    const bool nextReorderContractAvailable =
        reorderCapability != nullptr && reorderCapability->supportsHierarchyNodeReorder();
    if (m_reorderContractAvailable != nextReorderContractAvailable)
    {
        m_reorderContractAvailable = nextReorderContractAvailable;
        emit reorderContractAvailableChanged();
    }

    const auto* noteDropCapability = qobject_cast<IHierarchyNoteDropCapability*>(m_hierarchyViewModel);
    const bool nextNoteDropContractAvailable =
        noteDropCapability != nullptr && noteDropCapability->supportsHierarchyNoteDrop();
    if (m_noteDropContractAvailable != nextNoteDropContractAvailable)
    {
        m_noteDropContractAvailable = nextNoteDropContractAvailable;
        emit noteDropContractAvailableChanged();
    }
}

void HierarchyDragDropBridge::refreshSelectedItemKey()
{
    QString nextSelectedItemKey;
    if (m_hierarchyViewModel != nullptr)
    {
        const int selectedIndex = m_hierarchyViewModel->hierarchySelectedIndex();
        const QVariantList hierarchyModel = readHierarchyModel();
        if (selectedIndex >= 0 && selectedIndex < hierarchyModel.size())
        {
            nextSelectedItemKey = hierarchyModel.at(selectedIndex)
                                      .toMap()
                                      .value(QStringLiteral("key"))
                                      .toString()
                                      .trimmed();
        }
    }

    if (m_selectedItemKey == nextSelectedItemKey)
    {
        return;
    }

    m_selectedItemKey = nextSelectedItemKey;
    emit selectedItemKeyChanged();
}

void HierarchyDragDropBridge::disconnectHierarchyViewModel()
{
    if (m_hierarchyViewModelDestroyedConnection)
    {
        disconnect(m_hierarchyViewModelDestroyedConnection);
        m_hierarchyViewModelDestroyedConnection = QMetaObject::Connection();
    }
    if (m_hierarchyModelChangedConnection)
    {
        disconnect(m_hierarchyModelChangedConnection);
        m_hierarchyModelChangedConnection = QMetaObject::Connection();
    }
    if (m_selectedIndexChangedConnection)
    {
        disconnect(m_selectedIndexChangedConnection);
        m_selectedIndexChangedConnection = QMetaObject::Connection();
    }
}
