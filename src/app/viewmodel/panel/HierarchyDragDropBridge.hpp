#pragma once

#include "app/viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

class HierarchyDragDropBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QObject
        * hierarchyViewModel READ hierarchyViewModel WRITE setHierarchyViewModel NOTIFY hierarchyViewModelChanged)
    Q_PROPERTY(bool reorderContractAvailable READ reorderContractAvailable NOTIFY reorderContractAvailableChanged)
    Q_PROPERTY(bool noteDropContractAvailable READ noteDropContractAvailable NOTIFY noteDropContractAvailableChanged)
    Q_PROPERTY(QString selectedItemKey READ selectedItemKey NOTIFY selectedItemKeyChanged)

public:
    explicit HierarchyDragDropBridge(QObject* parent = nullptr);
    ~HierarchyDragDropBridge() override;

    QObject* hierarchyViewModel() const noexcept;
    void setHierarchyViewModel(QObject* viewModel);

    bool reorderContractAvailable() const noexcept;
    bool noteDropContractAvailable() const noexcept;
    QString selectedItemKey() const;

    Q_INVOKABLE bool applyHierarchyReorder(const QVariantList& hierarchyNodes,
                                           const QString& activeItemKey = QString());
    Q_INVOKABLE bool canAcceptNoteDrop(int index, const QString& noteId) const;
    Q_INVOKABLE bool canAcceptNoteDropList(int index, const QVariantList& noteIds) const;
    Q_INVOKABLE bool assignNoteToFolder(int index, const QString& noteId);
    Q_INVOKABLE bool assignNotesToFolder(int index, const QVariantList& noteIds);

    signals  :


    void hierarchyViewModelChanged();
    void reorderContractAvailableChanged();
    void noteDropContractAvailableChanged();
    void selectedItemKeyChanged();

private
    slots  :


    void handleHierarchyModelChanged();
    void handleSelectedIndexChanged();
    void handleHierarchyViewModelDestroyed();

private:
    QVariantList readHierarchyModel() const;
    QString resolvedActiveItemKey(const QString& preferredActiveItemKey) const;
    void refreshContractState();
    void refreshSelectedItemKey();
    void disconnectHierarchyViewModel();

    QPointer<IHierarchyViewModel> m_hierarchyViewModel;
    QString m_selectedItemKey;
    bool m_reorderContractAvailable = false;
    bool m_noteDropContractAvailable = false;
    QMetaObject::Connection m_hierarchyViewModelDestroyedConnection;
    QMetaObject::Connection m_hierarchyModelChangedConnection;
    QMetaObject::Connection m_selectedIndexChangedConnection;
};
