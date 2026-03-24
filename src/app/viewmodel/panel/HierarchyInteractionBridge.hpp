#pragma once

#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

class HierarchyInteractionBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QObject
        * hierarchyViewModel READ hierarchyViewModel WRITE setHierarchyViewModel NOTIFY hierarchyViewModelChanged)
    Q_PROPERTY(bool renameContractAvailable READ renameContractAvailable NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool viewOptionsEnabled READ viewOptionsEnabled NOTIFY capabilityStateChanged)

public:
    explicit HierarchyInteractionBridge(QObject* parent = nullptr);
    ~HierarchyInteractionBridge() override;

    QObject* hierarchyViewModel() const noexcept;
    void setHierarchyViewModel(QObject* viewModel);

    bool renameContractAvailable() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;
    bool viewOptionsEnabled() const noexcept;

    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();
    Q_INVOKABLE bool setItemExpanded(int index, bool expanded);

signals:
    void hierarchyViewModelChanged();
    void capabilityStateChanged();

private slots:
    void handleHierarchyViewModelDestroyed();
    void handleHierarchyStateChanged();

private:
    void disconnectHierarchyViewModel();
    bool refreshCapabilityState();

    QPointer<IHierarchyViewModel> m_hierarchyViewModel;
    bool m_renameContractAvailable = false;
    bool m_createFolderEnabled = false;
    bool m_deleteFolderEnabled = false;
    bool m_viewOptionsEnabled = true;
    QMetaObject::Connection m_hierarchyViewModelDestroyedConnection;
    QMetaObject::Connection m_hierarchySelectionChangedConnection;
    QMetaObject::Connection m_hierarchyNodesChangedConnection;
};
