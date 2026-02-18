#pragma once

#include <QString>
#include <QVector>

struct SidebarHierarchyItem
{
    int indentLevel = 0;
    bool accent = false;
    QString label;
    bool expanded = false;
    bool showChevron = true;
};

class HierarchySectionModel
{
public:
    virtual ~HierarchySectionModel();

    virtual QString sectionName() const = 0;
    virtual QString toolbarIconName() const = 0;
    virtual QVector<SidebarHierarchyItem> items() const = 0;
};

class LibraryModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class ProjectsModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class BookmarksModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class TagsModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class ResourcesModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class ProgressModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class EventModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};

class PresetModel final : public HierarchySectionModel
{
public:
    QString sectionName() const override;
    QString toolbarIconName() const override;
    QVector<SidebarHierarchyItem> items() const override;
};
