#pragma once

#include "app/store/sidebar/ISidebarSelectionStore.hpp"
#include "app/models/sidebar/IActiveHierarchyContextSource.hpp"
#include "app/models/sidebar/IHierarchyControllerProvider.hpp"

#include <QMetaObject>
#include <QPointer>

class SidebarHierarchyController final : public IActiveHierarchyContextSource
{
    Q_OBJECT

    Q_PROPERTY(
        int
 activeHierarchyIndex READ activeHierarchyIndex WRITE setActiveHierarchyIndex NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(int resolvedActiveHierarchyIndex READ activeHierarchyIndex NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(QObject* activeHierarchyController READ activeHierarchyController NOTIFY activeHierarchyControllerChanged)
    Q_PROPERTY(QObject* resolvedHierarchyController READ activeHierarchyController NOTIFY activeHierarchyControllerChanged)
    Q_PROPERTY(QObject* activeNoteListModel READ activeNoteListModel NOTIFY activeNoteListModelChanged)
    Q_PROPERTY(QObject* resolvedNoteListModel READ activeNoteListModel NOTIFY activeNoteListModelChanged)

public:
    explicit SidebarHierarchyController(QObject* parent = nullptr);
    ~SidebarHierarchyController() override;

    int activeHierarchyIndex() const noexcept override;
    Q_INVOKABLE void setActiveHierarchyIndex(int index);

    QObject* activeHierarchyController() const override;
    QObject* activeNoteListModel() const override;

    Q_INVOKABLE QObject* hierarchyControllerForIndex(int hierarchyIndex) const;
    Q_INVOKABLE QObject* noteListModelForIndex(int hierarchyIndex) const;

    ISidebarSelectionStore* selectionStore() const noexcept;
    void setSelectionStore(ISidebarSelectionStore* store);

    IHierarchyControllerProvider* controllerProvider() const noexcept;
    void setControllerProvider(IHierarchyControllerProvider* provider);

signals:
    void selectionStoreChanged();
    void controllerProviderChanged();

private:
    void emitActiveBindingsChanged();

    int m_fallbackHierarchyIndex = 0;
    QPointer<ISidebarSelectionStore> m_selectionStore;
    QPointer<IHierarchyControllerProvider> m_controllerProvider;
    QMetaObject::Connection m_selectionStoreConnection;
    QMetaObject::Connection m_providerConnection;
};
