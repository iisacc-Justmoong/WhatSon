#include "DetailPanelCurrentHierarchyBinder.hpp"

#include "DetailPanelViewModel.hpp"
#include "viewmodel/sidebar/IActiveHierarchyContextSource.hpp"

DetailPanelCurrentHierarchyBinder::DetailPanelCurrentHierarchyBinder(QObject* parent)
    : QObject(parent)
{
}

DetailPanelCurrentHierarchyBinder::~DetailPanelCurrentHierarchyBinder() = default;

DetailPanelViewModel* DetailPanelCurrentHierarchyBinder::detailPanelViewModel() const noexcept
{
    return m_detailPanelViewModel.data();
}

void DetailPanelCurrentHierarchyBinder::setDetailPanelViewModel(DetailPanelViewModel* detailPanelViewModel)
{
    if (m_detailPanelViewModel == detailPanelViewModel)
    {
        return;
    }

    m_detailPanelViewModel = detailPanelViewModel;
    emit detailPanelViewModelChanged();
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
    if (m_detailPanelViewModel == nullptr)
    {
        return;
    }

    QObject* activeNoteListModel = nullptr;
    QObject* activeHierarchyViewModel = nullptr;
    if (m_hierarchyContextSource != nullptr)
    {
        activeNoteListModel = m_hierarchyContextSource->activeNoteListModel();
        activeHierarchyViewModel = m_hierarchyContextSource->activeHierarchyViewModel();
    }

    m_detailPanelViewModel->setCurrentNoteListModel(activeNoteListModel);
    m_detailPanelViewModel->setCurrentNoteDirectorySourceViewModel(activeHierarchyViewModel);
}
