#pragma once

#include "app/models/file/hierarchy/IHierarchyController.hpp"

#include <QObject>

class IHierarchyControllerProvider : public QObject
{
    Q_OBJECT

public:
    explicit IHierarchyControllerProvider(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IHierarchyControllerProvider() override = default;

    Q_INVOKABLE virtual IHierarchyController* hierarchyController(int hierarchyIndex) const = 0;
    Q_INVOKABLE virtual QObject* noteListModel(int hierarchyIndex) const = 0;

    signals  :


    void mappingsChanged();
};
