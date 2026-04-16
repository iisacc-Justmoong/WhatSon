#pragma once

#include "store/sidebar/ISidebarSelectionStore.hpp"
#include "viewmodel/sidebar/IActiveHierarchyContextSource.hpp"
#include "viewmodel/sidebar/IHierarchyViewModelProvider.hpp"

#include <QMetaObject>
#include <QPointer>

class SidebarHierarchyViewModel final : public IActiveHierarchyContextSource
{
    Q_OBJECT

    Q_PROPERTY(
        int
 activeHierarchyIndex READ activeHierarchyIndex WRITE setActiveHierarchyIndex NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(int resolvedActiveHierarchyIndex READ activeHierarchyIndex NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(QObject* activeHierarchyViewModel READ activeHierarchyViewModel NOTIFY activeHierarchyViewModelChanged)
    Q_PROPERTY(QObject* resolvedHierarchyViewModel READ activeHierarchyViewModel NOTIFY activeHierarchyViewModelChanged)
    Q_PROPERTY(QObject* activeNoteListModel READ activeNoteListModel NOTIFY activeNoteListModelChanged)
    Q_PROPERTY(QObject* resolvedNoteListModel READ activeNoteListModel NOTIFY activeNoteListModelChanged)

public:
    explicit SidebarHierarchyViewModel(QObject* parent = nullptr);
    ~SidebarHierarchyViewModel() override;

    int activeHierarchyIndex() const noexcept override;
    Q_INVOKABLE void setActiveHierarchyIndex(int index);

    QObject* activeHierarchyViewModel() const override;
    QObject* activeNoteListModel() const override;

    Q_INVOKABLE QObject* hierarchyViewModelForIndex(int hierarchyIndex) const;
    Q_INVOKABLE QObject* noteListModelForIndex(int hierarchyIndex) const;

    ISidebarSelectionStore* selectionStore() const noexcept;
    void setSelectionStore(ISidebarSelectionStore* store);

    IHierarchyViewModelProvider* viewModelProvider() const noexcept;
    void setViewModelProvider(IHierarchyViewModelProvider* provider);

signals:
    void selectionStoreChanged();
    void viewModelProviderChanged();

private:
    void emitActiveBindingsChanged();

    int m_fallbackHierarchyIndex = 0;
    QPointer<ISidebarSelectionStore> m_selectionStore;
    QPointer<IHierarchyViewModelProvider> m_viewModelProvider;
    QMetaObject::Connection m_selectionStoreConnection;
    QMetaObject::Connection m_providerConnection;
};
