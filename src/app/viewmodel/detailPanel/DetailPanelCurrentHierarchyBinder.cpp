#include "DetailPanelCurrentHierarchyBinder.hpp"

#include "NoteDetailPanelViewModel.hpp"
#include "ResourceDetailPanelViewModel.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/IActiveHierarchyContextSource.hpp"

DetailPanelCurrentHierarchyBinder::DetailPanelCurrentHierarchyBinder(QObject* parent)
    : QObject(parent)
{
}

DetailPanelCurrentHierarchyBinder::~DetailPanelCurrentHierarchyBinder() = default;

NoteDetailPanelViewModel* DetailPanelCurrentHierarchyBinder::noteDetailPanelViewModel() const noexcept
{
    return m_noteDetailPanelViewModel.data();
}

void DetailPanelCurrentHierarchyBinder::setNoteDetailPanelViewModel(NoteDetailPanelViewModel* detailPanelViewModel)
{
    if (m_noteDetailPanelViewModel == detailPanelViewModel)
    {
        return;
    }

    m_noteDetailPanelViewModel = detailPanelViewModel;
    emit noteDetailPanelViewModelChanged();
    synchronize();
}

ResourceDetailPanelViewModel* DetailPanelCurrentHierarchyBinder::resourceDetailPanelViewModel() const noexcept
{
    return m_resourceDetailPanelViewModel.data();
}

void DetailPanelCurrentHierarchyBinder::setResourceDetailPanelViewModel(
    ResourceDetailPanelViewModel* detailPanelViewModel)
{
    if (m_resourceDetailPanelViewModel == detailPanelViewModel)
    {
        return;
    }

    m_resourceDetailPanelViewModel = detailPanelViewModel;
    emit resourceDetailPanelViewModelChanged();
    synchronize();
}

IActiveHierarchyContextSource* DetailPanelCurrentHierarchyBinder::hierarchyContextSource() const noexcept
{
    return m_hierarchyContextSource.data();
}

void DetailPanelCurrentHierarchyBinder::setHierarchyContextSource(
    IActiveHierarchyContextSource* hierarchyContextSource)
{
    if (m_hierarchyContextSource == hierarchyContextSource)
    {
        return;
    }

    if (m_hierarchyBindingsConnection)
    {
        QObject::disconnect(m_hierarchyBindingsConnection);
    }
    if (m_hierarchyDestroyedConnection)
    {
        QObject::disconnect(m_hierarchyDestroyedConnection);
    }

    m_hierarchyContextSource = hierarchyContextSource;
    if (m_hierarchyContextSource != nullptr)
    {
        m_hierarchyBindingsConnection = QObject::connect(
            m_hierarchyContextSource,
            &IActiveHierarchyContextSource::activeBindingsChanged,
            this,
            &DetailPanelCurrentHierarchyBinder::synchronize);
        m_hierarchyDestroyedConnection = QObject::connect(
            m_hierarchyContextSource,
            &QObject::destroyed,
            this,
            &DetailPanelCurrentHierarchyBinder::clearHierarchyContextSource);
    }
    else
    {
        m_hierarchyBindingsConnection = QMetaObject::Connection();
        m_hierarchyDestroyedConnection = QMetaObject::Connection();
    }

    emit hierarchyContextSourceChanged();
    synchronize();
}

void DetailPanelCurrentHierarchyBinder::clearHierarchyContextSource()
{
    if (m_hierarchyBindingsConnection)
    {
        QObject::disconnect(m_hierarchyBindingsConnection);
    }
    if (m_hierarchyDestroyedConnection)
    {
        QObject::disconnect(m_hierarchyDestroyedConnection);
    }

    m_hierarchyBindingsConnection = QMetaObject::Connection();
    m_hierarchyDestroyedConnection = QMetaObject::Connection();
    m_hierarchyContextSource = nullptr;
    emit hierarchyContextSourceChanged();
    synchronize();
}

void DetailPanelCurrentHierarchyBinder::synchronize()
{
    if (m_noteDetailPanelViewModel == nullptr && m_resourceDetailPanelViewModel == nullptr)
    {
        return;
    }

    int activeHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
    QObject* activeNoteListModel = nullptr;
    QObject* activeHierarchyViewModel = nullptr;
    if (m_hierarchyContextSource != nullptr)
    {
        activeHierarchyIndex = WhatSon::Sidebar::normalizeHierarchyIndex(m_hierarchyContextSource->activeHierarchyIndex());
        activeNoteListModel = m_hierarchyContextSource->activeNoteListModel();
        activeHierarchyViewModel = m_hierarchyContextSource->activeHierarchyViewModel();
    }

    if (m_noteDetailPanelViewModel != nullptr)
    {
        m_noteDetailPanelViewModel->setCurrentNoteListModel(activeNoteListModel);
        m_noteDetailPanelViewModel->setCurrentNoteDirectorySourceViewModel(activeHierarchyViewModel);
    }

    if (m_resourceDetailPanelViewModel != nullptr)
    {
        const bool resourceHierarchyActive =
            activeHierarchyIndex == static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources);
        m_resourceDetailPanelViewModel->setCurrentResourceListModel(resourceHierarchyActive ? activeNoteListModel : nullptr);
    }
}
