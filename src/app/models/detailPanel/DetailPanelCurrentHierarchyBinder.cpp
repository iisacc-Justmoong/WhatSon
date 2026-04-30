#include "app/models/detailPanel/DetailPanelCurrentHierarchyBinder.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/detailPanel/NoteDetailPanelController.hpp"
#include "app/models/detailPanel/ResourceDetailPanelController.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/sidebar/IActiveHierarchyContextSource.hpp"

DetailPanelCurrentHierarchyBinder::DetailPanelCurrentHierarchyBinder(QObject* parent)
    : QObject(parent)
{
}

DetailPanelCurrentHierarchyBinder::~DetailPanelCurrentHierarchyBinder() = default;

NoteDetailPanelController* DetailPanelCurrentHierarchyBinder::noteDetailPanelController() const noexcept
{
    return m_noteDetailPanelController.data();
}

void DetailPanelCurrentHierarchyBinder::setNoteDetailPanelController(NoteDetailPanelController* detailPanelController)
{
    if (detailPanelController != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setNoteDetailPanelController")))
    {
        return;
    }

    if (detailPanelController == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setNoteDetailPanelController")))
    {
        return;
    }

    if (m_noteDetailPanelController == detailPanelController)
    {
        return;
    }

    m_noteDetailPanelController = detailPanelController;
    emit noteDetailPanelControllerChanged();
    synchronize();
}

ResourceDetailPanelController* DetailPanelCurrentHierarchyBinder::resourceDetailPanelController() const noexcept
{
    return m_resourceDetailPanelController.data();
}

void DetailPanelCurrentHierarchyBinder::setResourceDetailPanelController(
    ResourceDetailPanelController* detailPanelController)
{
    if (detailPanelController != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setResourceDetailPanelController")))
    {
        return;
    }

    if (detailPanelController == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setResourceDetailPanelController")))
    {
        return;
    }

    if (m_resourceDetailPanelController == detailPanelController)
    {
        return;
    }

    m_resourceDetailPanelController = detailPanelController;
    emit resourceDetailPanelControllerChanged();
    synchronize();
}

IActiveHierarchyContextSource* DetailPanelCurrentHierarchyBinder::hierarchyContextSource() const noexcept
{
    return m_hierarchyContextSource.data();
}

void DetailPanelCurrentHierarchyBinder::setHierarchyContextSource(
    IActiveHierarchyContextSource* hierarchyContextSource)
{
    if (hierarchyContextSource != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setHierarchyContextSource")))
    {
        return;
    }

    if (hierarchyContextSource == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("DetailPanelCurrentHierarchyBinder::setHierarchyContextSource")))
    {
        return;
    }

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
    if (m_noteDetailPanelController == nullptr && m_resourceDetailPanelController == nullptr)
    {
        return;
    }

    int activeHierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
    QObject* activeNoteListModel = nullptr;
    QObject* activeHierarchyController = nullptr;
    if (m_hierarchyContextSource != nullptr)
    {
        activeHierarchyIndex = WhatSon::Sidebar::normalizeHierarchyIndex(m_hierarchyContextSource->activeHierarchyIndex());
        activeNoteListModel = m_hierarchyContextSource->activeNoteListModel();
        activeHierarchyController = m_hierarchyContextSource->activeHierarchyController();
    }

    if (m_noteDetailPanelController != nullptr)
    {
        m_noteDetailPanelController->setCurrentNoteListModel(activeNoteListModel);
        m_noteDetailPanelController->setCurrentNoteDirectorySourceController(activeHierarchyController);
    }

    if (m_resourceDetailPanelController != nullptr)
    {
        const bool resourceHierarchyActive =
            activeHierarchyIndex == static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources);
        m_resourceDetailPanelController->setCurrentResourceListModel(resourceHierarchyActive ? activeNoteListModel : nullptr);
    }
}
