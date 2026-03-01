#pragma once

#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/FlatHierarchyModel.hpp"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class ProjectsHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FlatHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ProjectsHierarchyViewModel(QObject* parent = nullptr);
    ~ProjectsHierarchyViewModel() override;

    FlatHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);
    int itemCount() const noexcept;
    bool loadSucceeded() const noexcept;
    QString lastLoadError() const;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setProjectNames(QStringList projectNames);
    QStringList projectNames() const;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :


    void selectedIndexChanged();
    void itemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncModel();
    void syncDomainStoreFromItems();

    QStringList m_projectNames;
    QVector<FlatHierarchyItem> m_items;
    WhatSonProjectsHierarchyStore m_store;
    FlatHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
};
