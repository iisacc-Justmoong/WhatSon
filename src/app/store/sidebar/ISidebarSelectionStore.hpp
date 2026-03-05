#pragma once

#include <QObject>

class ISidebarSelectionStore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        int
 selectedHierarchyIndex READ selectedHierarchyIndex WRITE setSelectedHierarchyIndex NOTIFY selectedHierarchyIndexChanged)


public:
    explicit ISidebarSelectionStore(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~ISidebarSelectionStore() override = default;

    virtual int selectedHierarchyIndex() const noexcept = 0;
    Q_INVOKABLE virtual void setSelectedHierarchyIndex(int index) = 0;

    signals  :


    void selectedHierarchyIndexChanged();
};
