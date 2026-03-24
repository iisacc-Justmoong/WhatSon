#pragma once

#include <QString>
#include <QVariantList>
#include <QtPlugin>

class IHierarchyRenameCapability
{
public:
    virtual ~IHierarchyRenameCapability() = default;

    virtual bool canRenameItem(int index) const = 0;
    virtual bool renameItem(int index, const QString& displayName) = 0;
    virtual bool renameEnabled() const noexcept = 0;
};

#define IHierarchyRenameCapability_iid "WhatSon.IHierarchyRenameCapability/1.0"
Q_DECLARE_INTERFACE(IHierarchyRenameCapability, IHierarchyRenameCapability_iid)

class IHierarchyCrudCapability
{
public:
    virtual ~IHierarchyCrudCapability() = default;

    virtual void createFolder() = 0;
    virtual void deleteSelectedFolder() = 0;
    virtual bool createFolderEnabled() const noexcept = 0;
    virtual bool deleteFolderEnabled() const noexcept = 0;
    virtual bool viewOptionsEnabled() const noexcept
    {
        return true;
    }
};

#define IHierarchyCrudCapability_iid "WhatSon.IHierarchyCrudCapability/1.0"
Q_DECLARE_INTERFACE(IHierarchyCrudCapability, IHierarchyCrudCapability_iid)

class IHierarchyExpansionCapability
{
public:
    virtual ~IHierarchyExpansionCapability() = default;

    virtual bool setItemExpanded(int index, bool expanded) = 0;
};

#define IHierarchyExpansionCapability_iid "WhatSon.IHierarchyExpansionCapability/1.0"
Q_DECLARE_INTERFACE(IHierarchyExpansionCapability, IHierarchyExpansionCapability_iid)

class IHierarchyReorderCapability
{
public:
    virtual ~IHierarchyReorderCapability() = default;

    virtual bool applyHierarchyNodes(const QVariantList& hierarchyNodes,
                                     const QString& activeItemKey = QString()) = 0;
    virtual bool supportsHierarchyNodeReorder() const noexcept = 0;
};

#define IHierarchyReorderCapability_iid "WhatSon.IHierarchyReorderCapability/1.0"
Q_DECLARE_INTERFACE(IHierarchyReorderCapability, IHierarchyReorderCapability_iid)

class IHierarchyNoteDropCapability
{
public:
    virtual ~IHierarchyNoteDropCapability() = default;

    virtual bool canAcceptNoteDrop(int index, const QString& noteId) const = 0;
    virtual bool assignNoteToFolder(int index, const QString& noteId) = 0;
    virtual bool supportsHierarchyNoteDrop() const noexcept = 0;
};

#define IHierarchyNoteDropCapability_iid "WhatSon.IHierarchyNoteDropCapability/1.0"
Q_DECLARE_INTERFACE(IHierarchyNoteDropCapability, IHierarchyNoteDropCapability_iid)
