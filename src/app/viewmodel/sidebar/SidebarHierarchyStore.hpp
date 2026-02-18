#pragma once

#include "HierarchyItemListModel.hpp"

#include <QObject>
#include <QStringList>

#include <memory>
#include <vector>

class HierarchySectionModel;

class SidebarHierarchyStore final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)
    Q_PROPERTY(HierarchyItemListModel* itemModel READ itemModel CONSTANT)
    Q_PROPERTY(QStringList sectionNames READ sectionNames CONSTANT)
    Q_PROPERTY(QStringList toolbarIconNames READ toolbarIconNames CONSTANT)

public:
    explicit SidebarHierarchyStore(QObject* parent = nullptr);
    ~SidebarHierarchyStore() override;

    int activeIndex() const noexcept;
    void setActiveIndex(int index);

    HierarchyItemListModel* itemModel() noexcept;
    QStringList sectionNames() const;
    QStringList toolbarIconNames() const;

    Q_INVOKABLE int sectionCount() const noexcept;

    signals  :


    void activeIndexChanged();

private:
    void syncActiveSection();

    std::vector<std::unique_ptr<HierarchySectionModel>> m_sections;
    HierarchyItemListModel m_itemModel;
    int m_activeIndex = 0;
};
