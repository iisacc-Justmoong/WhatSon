#pragma once

#include "TagsHierarchyModel.hpp"
#include "file/hierarchy/tags/WhatSonTagDepthEntry.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QVariantList>
#include <QVector>

class TagsHierarchyViewModel final : public IHierarchyViewModel,
                                     public IHierarchyRenameCapability,
                                     public IHierarchyCrudCapability,
                                     public IHierarchyExpansionCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability IHierarchyExpansionCapability)

    Q_PROPERTY(TagsHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyModel READ hierarchyModel NOTIFY hierarchyModelChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    Q_PROPERTY(bool loadSucceeded READ loadSucceeded NOTIFY loadStateChanged)
    Q_PROPERTY(QString lastLoadError READ lastLoadError NOTIFY loadStateChanged)
    Q_PROPERTY(bool renameEnabled READ renameEnabled CONSTANT)
    Q_PROPERTY(bool createFolderEnabled READ createFolderEnabled CONSTANT)
    Q_PROPERTY(bool deleteFolderEnabled READ deleteFolderEnabled NOTIFY selectedIndexChanged)

public:
    explicit TagsHierarchyViewModel(QObject* parent = nullptr);
    ~TagsHierarchyViewModel() override;

    TagsHierarchyModel* itemModel() noexcept override;

    int selectedIndex() const noexcept override;
    Q_INVOKABLE void setSelectedIndex(int index) override;
    int itemCount() const noexcept override;
    bool loadSucceeded() const noexcept override;
    QString lastLoadError() const override;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    QVariantList hierarchyModel() const override;
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const override;
    Q_INVOKABLE bool canRenameItem(int index) const override;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName) override;
    Q_INVOKABLE bool setItemExpanded(int index, bool expanded) override;
    Q_INVOKABLE void createFolder() override;
    Q_INVOKABLE void deleteSelectedFolder() override;

    void setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries);
    QVector<WhatSonTagDepthEntry> tagDepthEntries() const;
    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applyRuntimeSnapshot(
        QVector<WhatSonTagDepthEntry> entries,
        QString tagsFilePath,
        bool loadSucceeded,
        QString errorMessage = QString());
    bool renameEnabled() const noexcept override;
    bool createFolderEnabled() const noexcept override;
    bool deleteFolderEnabled() const noexcept override;

public
    slots  :




    void requestViewModelHook();

    signals  :



    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();
    void viewModelHookRequested();

private:
    static int extractDepth(const QVariantMap& entryMap);
    static QVector<TagsHierarchyItem> buildItems(const QVector<WhatSonTagDepthEntry>& entries);
    static int nextFolderSequence(const QVector<WhatSonTagDepthEntry>& entries);
    bool reloadFromTagsFilePath(QString* errorMessage = nullptr);
    void updateItemCount();
    void updateLoadState(bool succeeded, QString errorMessage = QString());
    void syncStore();
    void syncModel();

    QVector<WhatSonTagDepthEntry> m_entries;
    QVector<TagsHierarchyItem> m_items;
    WhatSonTagsHierarchyStore m_store;
    TagsHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
    int m_createdFolderSequence = 1;
    int m_itemCount = 0;
    bool m_loadSucceeded = false;
    QString m_lastLoadError;
    QString m_tagsFilePath;
};
