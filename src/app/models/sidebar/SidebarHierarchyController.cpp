#include "app/models/sidebar/SidebarHierarchyController.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDebug>
#include <QQmlEngine>

namespace
{
    QObject* stabilizeQmlBindingOwnership(QObject* object)
    {
        if (object != nullptr)
        {
            QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
        }
        return object;
    }
}

SidebarHierarchyController::SidebarHierarchyController(QObject* parent)
{
    setParent(parent);
    m_fallbackHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
}

SidebarHierarchyController::~SidebarHierarchyController() = default;

int SidebarHierarchyController::activeHierarchyIndex() const noexcept
{
    if (m_selectionStore != nullptr)
    {
        return WhatSon::Sidebar::normalizeHierarchyIndex(m_selectionStore->selectedHierarchyIndex());
    }
    return WhatSon::Sidebar::normalizeHierarchyIndex(m_fallbackHierarchyIndex);
}

void SidebarHierarchyController::setActiveHierarchyIndex(int index)
{
    const int normalizedIndex = WhatSon::Sidebar::normalizeHierarchyIndex(index);
    if (m_selectionStore != nullptr)
    {
        m_selectionStore->setSelectedHierarchyIndex(normalizedIndex);
        return;
    }

    if (m_fallbackHierarchyIndex == normalizedIndex)
    {
        return;
    }
    m_fallbackHierarchyIndex = normalizedIndex;
    emitActiveBindingsChanged();
}

QObject* SidebarHierarchyController::activeHierarchyController() const
{
    return hierarchyControllerForIndex(activeHierarchyIndex());
}

QObject* SidebarHierarchyController::activeNoteListModel() const
{
    QObject* model = noteListModelForIndex(activeHierarchyIndex());
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("sidebar.hierarchy.controller"),
                              QStringLiteral("activeNoteListModel"),
                              QStringLiteral("activeHierarchyIndex=%1 noteListModel=0x%2")
                                  .arg(activeHierarchyIndex())
                                  .arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    return model;
}

QObject* SidebarHierarchyController::hierarchyControllerForIndex(int hierarchyIndex) const
{
    if (m_controllerProvider == nullptr)
    {
        return nullptr;
    }
    return stabilizeQmlBindingOwnership(
        m_controllerProvider->hierarchyController(WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex)));
}

QObject* SidebarHierarchyController::noteListModelForIndex(int hierarchyIndex) const
{
    if (m_controllerProvider == nullptr)
    {
        return nullptr;
    }
    return stabilizeQmlBindingOwnership(
        m_controllerProvider->noteListModel(WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex)));
}

ISidebarSelectionStore* SidebarHierarchyController::selectionStore() const noexcept
{
    return m_selectionStore;
}

void SidebarHierarchyController::setSelectionStore(ISidebarSelectionStore* store)
{
    if (store != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::Controller,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("SidebarHierarchyController::setSelectionStore")))
    {
        return;
    }

    if (store == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("SidebarHierarchyController::setSelectionStore")))
    {
        return;
    }

    if (m_selectionStore == store)
    {
        return;
    }

    const int preservedIndex = activeHierarchyIndex();
    if (m_selectionStoreConnection)
    {
        QObject::disconnect(m_selectionStoreConnection);
    }

    m_selectionStore = store;
    if (m_selectionStore != nullptr)
    {
        m_selectionStore->setSelectedHierarchyIndex(preservedIndex);
        m_selectionStoreConnection = QObject::connect(
            m_selectionStore,
            &ISidebarSelectionStore::selectedHierarchyIndexChanged,
            this,
            [this]()
            {
                emitActiveBindingsChanged();
            });
    }
    else
    {
        m_fallbackHierarchyIndex = preservedIndex;
        m_selectionStoreConnection = QMetaObject::Connection();
    }

    emit selectionStoreChanged();
    emitActiveBindingsChanged();
}

IHierarchyControllerProvider* SidebarHierarchyController::controllerProvider() const noexcept
{
    return m_controllerProvider;
}

void SidebarHierarchyController::setControllerProvider(IHierarchyControllerProvider* provider)
{
    if (!WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("SidebarHierarchyController::setControllerProvider")))
    {
        return;
    }

    if (m_controllerProvider == provider)
    {
        return;
    }

    if (m_providerConnection)
    {
        QObject::disconnect(m_providerConnection);
    }

    m_controllerProvider = provider;
    if (m_controllerProvider != nullptr)
    {
        m_providerConnection = QObject::connect(
            m_controllerProvider,
            &IHierarchyControllerProvider::mappingsChanged,
            this,
            [this]()
            {
                emit activeHierarchyControllerChanged();
                emit activeNoteListModelChanged();
                emit activeBindingsChanged();
            });
    }
    else
    {
        m_providerConnection = QMetaObject::Connection();
    }

    emit controllerProviderChanged();
    emit activeHierarchyControllerChanged();
    emit activeNoteListModelChanged();
    emit activeBindingsChanged();
}

void SidebarHierarchyController::emitActiveBindingsChanged()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("sidebar.hierarchy.controller"),
                              QStringLiteral("emitActiveBindingsChanged"),
                              QStringLiteral("activeHierarchyIndex=%1 hierarchyController=0x%2 noteListModel=0x%3")
                                  .arg(activeHierarchyIndex())
                                  .arg(QString::number(reinterpret_cast<quintptr>(activeHierarchyController()), 16))
                                  .arg(QString::number(reinterpret_cast<quintptr>(activeNoteListModel()), 16)));
    emit activeHierarchyIndexChanged();
    emit activeHierarchyControllerChanged();
    emit activeNoteListModelChanged();
    emit activeBindingsChanged();
}
