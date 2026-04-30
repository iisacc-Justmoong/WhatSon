#pragma once

#include "app/models/file/hierarchy/IHierarchyController.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

class HierarchyInteractionBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QObject
        * hierarchyController READ hierarchyController WRITE setHierarchyController NOTIFY hierarchyControllerChanged)
    Q_PROPERTY(bool renameContractAvailable READ renameContractAvailable NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY capabilityStateChanged)
    Q_PROPERTY(bool viewOptionsEnabled READ viewOptionsEnabled NOTIFY capabilityStateChanged)

public:
    explicit HierarchyInteractionBridge(QObject* parent = nullptr);
    ~HierarchyInteractionBridge() override;

    QObject* hierarchyController() const noexcept;
    void setHierarchyController(QObject* controller);

    bool renameContractAvailable() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;
    bool viewOptionsEnabled() const noexcept;

    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();
    Q_INVOKABLE bool setItemExpanded(int index, bool expanded);
    Q_INVOKABLE bool setAllItemsExpanded(bool expanded);

signals:
    void hierarchyControllerChanged();
    void capabilityStateChanged();

private slots:
    void handleHierarchyControllerDestroyed();
    void handleHierarchyStateChanged();

private:
    void disconnectHierarchyController();
    bool refreshCapabilityState();

    QPointer<IHierarchyController> m_hierarchyController;
    bool m_renameContractAvailable = false;
    bool m_createFolderEnabled = false;
    bool m_deleteFolderEnabled = false;
    bool m_viewOptionsEnabled = true;
    QMetaObject::Connection m_hierarchyControllerDestroyedConnection;
    QMetaObject::Connection m_hierarchySelectionChangedConnection;
    QMetaObject::Connection m_hierarchyNodesChangedConnection;
};
