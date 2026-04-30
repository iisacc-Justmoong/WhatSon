#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class IHierarchyController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* hierarchyItemModel READ hierarchyItemModel CONSTANT)
    Q_PROPERTY(QObject* hierarchyNoteListModel READ hierarchyNoteListModel CONSTANT)
    Q_PROPERTY(QVariantList hierarchyNodes READ hierarchyNodes NOTIFY hierarchyNodesChanged)
    Q_PROPERTY(int hierarchySelectedIndex READ hierarchySelectedIndex WRITE setHierarchySelectedIndex NOTIFY hierarchySelectionChanged)
    Q_PROPERTY(int hierarchyItemCount READ hierarchyItemCount NOTIFY hierarchyItemCountChanged)
    Q_PROPERTY(bool hierarchyLoadSucceeded READ hierarchyLoadSucceeded NOTIFY hierarchyLoadStateChanged)
    Q_PROPERTY(QString hierarchyLastLoadError READ hierarchyLastLoadError NOTIFY hierarchyLoadStateChanged)

public:
    explicit IHierarchyController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IHierarchyController() override = default;

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

    Q_INVOKABLE QString hierarchyItemLabelAt(int index) const
    {
        return itemLabel(index);
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

signals:
    void hierarchyNodesChanged();
    void hierarchySelectionChanged();
    void hierarchyItemCountChanged();
    void hierarchyLoadStateChanged();

protected:
    void initializeHierarchyInterfaceSignalBridge()
    {
        QObject::connect(this, SIGNAL(selectedIndexChanged()), this, SIGNAL(hierarchySelectionChanged()));
        QObject::connect(this, SIGNAL(hierarchyModelChanged()), this, SIGNAL(hierarchyNodesChanged()));
        QObject::connect(this, SIGNAL(itemCountChanged()), this, SIGNAL(hierarchyItemCountChanged()));
        QObject::connect(this, SIGNAL(loadStateChanged()), this, SIGNAL(hierarchyLoadStateChanged()));
    }
};
