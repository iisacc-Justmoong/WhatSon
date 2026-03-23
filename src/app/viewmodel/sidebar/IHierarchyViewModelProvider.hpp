#pragma once

#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"

#include <QObject>

class IHierarchyViewModelProvider : public QObject
{
    Q_OBJECT

public:
    explicit IHierarchyViewModelProvider(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IHierarchyViewModelProvider() override = default;

    Q_INVOKABLE virtual IHierarchyViewModel* hierarchyViewModel(int hierarchyIndex) const = 0;
    Q_INVOKABLE virtual QObject* noteListModel(int hierarchyIndex) const = 0;

    signals  :


    void mappingsChanged();
};
