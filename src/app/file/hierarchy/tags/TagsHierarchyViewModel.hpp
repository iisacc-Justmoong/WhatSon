#pragma once

#include "TagsHierarchyModel.hpp"
#include "WhatSonTagDepthEntry.hpp"

#include <QObject>
#include <QVector>

class TagsHierarchyViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(TagsHierarchyModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    explicit TagsHierarchyViewModel(QObject* parent = nullptr);
    ~TagsHierarchyViewModel() override;

    TagsHierarchyModel* itemModel() noexcept;

    int selectedIndex() const noexcept;
    Q_INVOKABLE void setSelectedIndex(int index);

    void setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries);
    QVector<WhatSonTagDepthEntry> tagDepthEntries() const;

    signals  :


    void selectedIndexChanged();

private:
    static QString fallbackLabel(int ordinal);
    static QVector<TagsHierarchyItem> buildItems(const QVector<WhatSonTagDepthEntry>& entries);
    void syncModel();

    QVector<WhatSonTagDepthEntry> m_entries;
    QVector<TagsHierarchyItem> m_items;
    TagsHierarchyModel m_itemModel;
    int m_selectedIndex = -1;
};
