#pragma once

#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "viewmodel/hierarchy/common/FlatHierarchyModel.hpp"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class ProgressHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(FlatHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit ProgressHierarchyViewModel(QObject* parent = nullptr);
    ~ProgressHierarchyViewModel() override;

    FlatHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setProgressState(int progressValue, QStringList progressStates);
    int progressValue() const noexcept;
    QStringList progressStates() const;
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    signals  :



    void selectedIndexChanged();

private:
    void rebuildItems();
    void syncProgressStore();
    void syncProgressStatesFromItems();
    void syncModel();

    int m_progressValue = 0;
    QStringList m_progressStates;
    QVector<FlatHierarchyItem> m_items;
    WhatSonProgressHierarchyStore m_store;
    FlatHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
};
