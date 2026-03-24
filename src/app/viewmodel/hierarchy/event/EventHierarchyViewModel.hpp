#pragma once

#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyModel.hpp"

#include <QStringList>
#include <QVariantList>
#include <QVector>

class EventHierarchyViewModel final : public IHierarchyViewModel,
                                      public IHierarchyRenameCapability,
                                      public IHierarchyCrudCapability,
                                      public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

    Q_PROPERTY(EventHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit EventHierarchyViewModel(QObject* parent = nullptr);
    ~EventHierarchyViewModel() override;

    EventHierarchyModel* itemModel() noexcept override;

    int selectedIndex() const noexcept override;
    Q_INVOKABLE void setSelectedIndex(int index) override;
    int itemCount() const noexcept override;
    bool loadSucceeded() const noexcept override;
    QString lastLoadError() const override;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    QVariantList hierarchyModel() const override;
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const override;
    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE bool setItemExpanded(int index, bool expanded);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setEventNames(QStringList eventNames);
    QStringList eventNames() const;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QStringList eventNames,
        QString eventFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncModel();
    void syncDomainStoreFromItems();

    QStringList m_eventNames;
    QVector<EventHierarchyItem> m_items;
    WhatSonEventHierarchyStore m_store;
    EventHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_eventFilePath;
};
