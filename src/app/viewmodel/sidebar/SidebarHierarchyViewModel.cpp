#include "app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/viewmodel/sidebar/HierarchySidebarDomain.hpp"

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
    return noteListModelForIndex(activeHierarchyIndex());
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
        && !WhatSon::Policy::verifyDependencyAllowed(
            WhatSon::Policy::Layer::ViewModel,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("SidebarHierarchyViewModel::setSelectionStore")))
    {
        return;
    }

    if (WhatSon::Policy::ArchitecturePolicyLock::isLocked())
    {
        qWarning().noquote()
            << QStringLiteral(
                "[whatson:policy][lock] SidebarHierarchyViewModel::setSelectionStore rejected because architecture policy is locked");
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
    if (WhatSon::Policy::ArchitecturePolicyLock::isLocked())
    {
        qWarning().noquote()
            << QStringLiteral(
                "[whatson:policy][lock] SidebarHierarchyViewModel::setViewModelProvider rejected because architecture policy is locked");
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
    emit activeHierarchyIndexChanged();
    emit activeHierarchyViewModelChanged();
    emit activeNoteListModelChanged();
    emit activeBindingsChanged();
}
