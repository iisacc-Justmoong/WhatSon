#pragma once

#include "viewmodel/sidebar/IActiveHierarchySource.hpp"

#include <QObject>

class IActiveHierarchyContextSource : public IActiveHierarchySource
{
    Q_OBJECT

public:
    explicit IActiveHierarchyContextSource(QObject* parent = nullptr)
        : IActiveHierarchySource(parent)
    {
    }

    ~IActiveHierarchyContextSource() override = default;

    virtual QObject* activeHierarchyViewModel() const = 0;
    virtual QObject* activeNoteListModel() const = 0;

signals:
    void activeBindingsChanged();
    void activeHierarchyViewModelChanged();
    void activeNoteListModelChanged();
};
