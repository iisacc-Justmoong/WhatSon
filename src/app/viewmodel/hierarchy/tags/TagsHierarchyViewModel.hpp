#pragma once

#include "TagsHierarchyModel.hpp"
#include "file/hierarchy/tags/WhatSonTagDepthEntry.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"

#include <QObject>
#include <QVariantList>
#include <QVector>

class TagsHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(TagsHierarchyModel* itemModel READ itemModel CONSTANT)
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

    TagsHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);
    int itemCount() const noexcept;
    bool loadSucceeded() const noexcept;
    QString lastLoadError() const;

    Q_INVOKABLE void setDepthItems(const QVariantList& depthItems);
    Q_INVOKABLE QVariantList depthItems() const;
    Q_INVOKABLE QString itemLabel(int index) const;
    Q_INVOKABLE bool canRenameItem(int index) const;
    Q_INVOKABLE bool renameItem(int index, const QString& displayName);
    Q_INVOKABLE void createFolder();
    Q_INVOKABLE void deleteSelectedFolder();

    void setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries);
    QVector<WhatSonTagDepthEntry> tagDepthEntries() const;
    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    bool renameEnabled() const noexcept;
    bool createFolderEnabled() const noexcept;
    bool deleteFolderEnabled() const noexcept;

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
    static int extractDepth(const QVariantMap& entryMap);
    static QString fallbackLabel(int ordinal);
    static QVector<TagsHierarchyItem> buildItems(const QVector<WhatSonTagDepthEntry>& entries);
    static int nextFolderSequence(const QVector<WhatSonTagDepthEntry>& entries);
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
