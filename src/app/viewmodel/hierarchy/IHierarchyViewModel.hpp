#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class IHierarchyViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* hierarchyItemModel READ hierarchyItemModel CONSTANT)
    Q_PROPERTY(QObject* hierarchyNoteListModel READ hierarchyNoteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyNodes READ hierarchyNodes NOTIFY hierarchyNodesChanged)
    Q_PROPERTY(int hierarchySelectedIndex READ hierarchySelectedIndex WRITE setHierarchySelectedIndex NOTIFY hierarchySelectionChanged)
    Q_PROPERTY(int hierarchyItemCount READ hierarchyItemCount NOTIFY hierarchyItemCountChanged)
    Q_PROPERTY(bool hierarchyLoadSucceeded READ hierarchyLoadSucceeded NOTIFY hierarchyLoadStateChanged)
    Q_PROPERTY(QString hierarchyLastLoadError READ hierarchyLastLoadError NOTIFY hierarchyLoadStateChanged)
    Q_PROPERTY(bool hierarchyRenameEnabled READ hierarchyRenameEnabled NOTIFY hierarchyCapabilityStateChanged)
    Q_PROPERTY(bool hierarchyCreateEnabled READ hierarchyCreateEnabled NOTIFY hierarchyCapabilityStateChanged)
    Q_PROPERTY(bool hierarchyDeleteEnabled READ hierarchyDeleteEnabled NOTIFY hierarchyCapabilityStateChanged)
    Q_PROPERTY(bool hierarchyViewOptionsEnabled READ hierarchyViewOptionsEnabled NOTIFY hierarchyCapabilityStateChanged)

public:
    explicit IHierarchyViewModel(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IHierarchyViewModel() override = default;

    QObject* hierarchyItemModel() noexcept
    {
        return itemModel();
    }

    QObject* hierarchyNoteListModel() noexcept
    {
        return noteListModel();
    }

    QVariantList hierarchyNodes() const
    {
        return hierarchyModel();
    }

    int hierarchySelectedIndex() const noexcept
    {
        return selectedIndex();
    }

    Q_INVOKABLE void setHierarchySelectedIndex(int index)
    {
        setSelectedIndex(index);
    }

    int hierarchyItemCount() const noexcept
    {
        return itemCount();
    }

    bool hierarchyLoadSucceeded() const noexcept
    {
        return loadSucceeded();
    }

    QString hierarchyLastLoadError() const
    {
        return lastLoadError();
    }

    bool hierarchyRenameEnabled() const noexcept
    {
        return renameEnabled();
    }

    bool hierarchyCreateEnabled() const noexcept
    {
        return createFolderEnabled();
    }

    bool hierarchyDeleteEnabled() const noexcept
    {
        return deleteFolderEnabled();
    }

    bool hierarchyViewOptionsEnabled() const noexcept
    {
        return viewOptionsEnabled();
    }

    Q_INVOKABLE QString hierarchyItemLabelAt(int index) const
    {
        return itemLabel(index);
    }

    Q_INVOKABLE bool canRenameHierarchyItemAt(int index) const
    {
        return canRenameItem(index);
    }

    Q_INVOKABLE bool renameHierarchyItemAt(int index, const QString& displayName)
    {
        return renameItem(index, displayName);
    }

    Q_INVOKABLE void createHierarchyItem()
    {
        createFolder();
    }

    Q_INVOKABLE void deleteSelectedHierarchyItem()
    {
        deleteSelectedFolder();
    }

    Q_INVOKABLE bool setHierarchyItemExpandedState(int index, bool expanded)
    {
        return setItemExpanded(index, expanded);
    }

    Q_INVOKABLE bool applyHierarchyNodeReorder(const QVariantList& hierarchyNodes,
                                               const QString& activeItemKey = QString())
    {
        return applyHierarchyNodes(hierarchyNodes, activeItemKey);
    }

    Q_INVOKABLE bool canAcceptHierarchyNoteDropAt(int index, const QString& noteId) const
    {
        return canAcceptNoteDrop(index, noteId);
    }

    Q_INVOKABLE bool assignHierarchyNoteToFolderAt(int index, const QString& noteId)
    {
        return assignNoteToFolder(index, noteId);
    }

    virtual QObject* itemModel() noexcept = 0;
    virtual QObject* noteListModel() noexcept
    {
        return nullptr;
    }
    virtual int selectedIndex() const noexcept = 0;
    virtual void setSelectedIndex(int index) = 0;
    virtual int itemCount() const noexcept = 0;
    virtual bool loadSucceeded() const noexcept = 0;
    virtual QString lastLoadError() const = 0;
    virtual QVariantList hierarchyModel() const = 0;
    virtual QString itemLabel(int index) const = 0;
    virtual bool canRenameItem(int index) const = 0;
    virtual bool renameItem(int index, const QString& displayName) = 0;
    virtual void createFolder() = 0;
    virtual void deleteSelectedFolder() = 0;
    virtual bool renameEnabled() const noexcept = 0;
    virtual bool createFolderEnabled() const noexcept = 0;
    virtual bool deleteFolderEnabled() const noexcept = 0;
    virtual bool viewOptionsEnabled() const noexcept
    {
        return true;
    }
    virtual bool setItemExpanded(int index, bool expanded)
    {
        Q_UNUSED(index);
        Q_UNUSED(expanded);
        return false;
    }
    virtual bool applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey = QString())
    {
        Q_UNUSED(hierarchyNodes);
        Q_UNUSED(activeItemKey);
        return false;
    }
    virtual bool canAcceptNoteDrop(int index, const QString& noteId) const
    {
        Q_UNUSED(index);
        Q_UNUSED(noteId);
        return false;
    }
    virtual bool assignNoteToFolder(int index, const QString& noteId)
    {
        Q_UNUSED(index);
        Q_UNUSED(noteId);
        return false;
    }
    virtual bool supportsHierarchyNodeReorder() const noexcept
    {
        return false;
    }
    virtual bool supportsHierarchyNoteDrop() const noexcept
    {
        return false;
    }

signals:
    void hierarchyNodesChanged();
    void hierarchySelectionChanged();
    void hierarchyItemCountChanged();
    void hierarchyLoadStateChanged();
    void hierarchyCapabilityStateChanged();

protected:
    void initializeHierarchyInterfaceSignalBridge()
    {
        QObject::connect(this, SIGNAL(selectedIndexChanged()), this, SIGNAL(hierarchySelectionChanged()));
        QObject::connect(this, SIGNAL(selectedIndexChanged()), this, SIGNAL(hierarchyCapabilityStateChanged()));
        QObject::connect(this, SIGNAL(hierarchyModelChanged()), this, SIGNAL(hierarchyNodesChanged()));
        QObject::connect(this, SIGNAL(hierarchyModelChanged()), this, SIGNAL(hierarchyCapabilityStateChanged()));
        QObject::connect(this, SIGNAL(itemCountChanged()), this, SIGNAL(hierarchyItemCountChanged()));
        QObject::connect(this, SIGNAL(loadStateChanged()), this, SIGNAL(hierarchyLoadStateChanged()));
    }
};
