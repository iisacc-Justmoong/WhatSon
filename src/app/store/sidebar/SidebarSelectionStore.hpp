#pragma once

#include "store/sidebar/ISidebarSelectionStore.hpp"

class SidebarSelectionStore final : public ISidebarSelectionStore
{
    Q_OBJECT

public:
    explicit SidebarSelectionStore(QObject* parent = nullptr);
    ~SidebarSelectionStore() override;

    int selectedHierarchyIndex() const noexcept override;
    Q_INVOKABLE void setSelectedHierarchyIndex(int index) override;

private:
    int m_selectedHierarchyIndex = 0;
};
