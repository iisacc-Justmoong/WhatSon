#pragma once

#include <QObject>

class IActiveHierarchySource : public QObject
{
    Q_OBJECT

public:
    explicit IActiveHierarchySource(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~IActiveHierarchySource() override = default;

    virtual int activeHierarchyIndex() const noexcept = 0;

signals:
    void activeHierarchyIndexChanged();
};
