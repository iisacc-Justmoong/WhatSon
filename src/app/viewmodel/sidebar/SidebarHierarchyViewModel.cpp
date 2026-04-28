#include "app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/viewmodel/sidebar/HierarchySidebarDomain.hpp"
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

SidebarHierarchyViewModel::SidebarHierarchyViewModel(QObject* parent)
{
    setParent(parent);
    m_fallbackHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
}

SidebarHierarchyViewModel::~SidebarHierarchyViewModel() = default;

int SidebarHierarchyViewModel::activeHierarchyIndex() const noexcept
{
    if (m_selectionStore != nullptr)
    {
        return WhatSon::Sidebar::normalizeHierarchyIndex(m_selectionStore->selectedHierarchyIndex());
    }
    return WhatSon::Sidebar::normalizeHierarchyIndex(m_fallbackHierarchyIndex);
}

void SidebarHierarchyViewModel::setActiveHierarchyIndex(int index)
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

QObject* SidebarHierarchyViewModel::activeHierarchyViewModel() const
{
    return hierarchyViewModelForIndex(activeHierarchyIndex());
}

QObject* SidebarHierarchyViewModel::activeNoteListModel() const
{
    QObject* model = noteListModelForIndex(activeHierarchyIndex());
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("sidebar.hierarchy.viewmodel"),
                              QStringLiteral("activeNoteListModel"),
                              QStringLiteral("activeHierarchyIndex=%1 noteListModel=0x%2")
                                  .arg(activeHierarchyIndex())
                                  .arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    return model;
}

QObject* SidebarHierarchyViewModel::hierarchyViewModelForIndex(int hierarchyIndex) const
{
    if (m_viewModelProvider == nullptr)
    {
        return nullptr;
    }
    return stabilizeQmlBindingOwnership(
        m_viewModelProvider->hierarchyViewModel(WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex)));
}

QObject* SidebarHierarchyViewModel::noteListModelForIndex(int hierarchyIndex) const
{
    if (m_viewModelProvider == nullptr)
    {
        return nullptr;
    }
    return stabilizeQmlBindingOwnership(
        m_viewModelProvider->noteListModel(WhatSon::Sidebar::normalizeHierarchyIndex(hierarchyIndex)));
}

ISidebarSelectionStore* SidebarHierarchyViewModel::selectionStore() const noexcept
{
    return m_selectionStore;
}

void SidebarHierarchyViewModel::setSelectionStore(ISidebarSelectionStore* store)
{
    if (store != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::ViewModel,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("SidebarHierarchyViewModel::setSelectionStore")))
    {
        return;
    }

    if (store == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("SidebarHierarchyViewModel::setSelectionStore")))
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

IHierarchyViewModelProvider* SidebarHierarchyViewModel::viewModelProvider() const noexcept
{
    return m_viewModelProvider;
}

void SidebarHierarchyViewModel::setViewModelProvider(IHierarchyViewModelProvider* provider)
{
    if (!WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("SidebarHierarchyViewModel::setViewModelProvider")))
    {
        return;
    }

    if (m_viewModelProvider == provider)
    {
        return;
    }

    if (m_providerConnection)
    {
        QObject::disconnect(m_providerConnection);
    }

    m_viewModelProvider = provider;
    if (m_viewModelProvider != nullptr)
    {
        m_providerConnection = QObject::connect(
            m_viewModelProvider,
            &IHierarchyViewModelProvider::mappingsChanged,
            this,
            [this]()
            {
                emit activeHierarchyViewModelChanged();
                emit activeNoteListModelChanged();
                emit activeBindingsChanged();
            });
    }
    else
    {
        m_providerConnection = QMetaObject::Connection();
    }

    emit viewModelProviderChanged();
    emit activeHierarchyViewModelChanged();
    emit activeNoteListModelChanged();
    emit activeBindingsChanged();
}

void SidebarHierarchyViewModel::emitActiveBindingsChanged()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("sidebar.hierarchy.viewmodel"),
                              QStringLiteral("emitActiveBindingsChanged"),
                              QStringLiteral("activeHierarchyIndex=%1 hierarchyViewModel=0x%2 noteListModel=0x%3")
                                  .arg(activeHierarchyIndex())
                                  .arg(QString::number(reinterpret_cast<quintptr>(activeHierarchyViewModel()), 16))
                                  .arg(QString::number(reinterpret_cast<quintptr>(activeNoteListModel()), 16)));
    emit activeHierarchyIndexChanged();
    emit activeHierarchyViewModelChanged();
    emit activeNoteListModelChanged();
    emit activeBindingsChanged();
}
