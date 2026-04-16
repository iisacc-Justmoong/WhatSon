#pragma once

#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/IHierarchyViewModelProvider.hpp"

#include <QPointer>
#include <QVector>

#include <array>

class HierarchyViewModelProvider final : public IHierarchyViewModelProvider
{
    Q_OBJECT

public:
    struct Mapping final
    {
        int hierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
        IHierarchyViewModel* viewModel = nullptr;
    };

    explicit HierarchyViewModelProvider(QObject* parent = nullptr);
    ~HierarchyViewModelProvider() override;

    void setMappings(QVector<Mapping> mappings);
    QVector<Mapping> mappings() const;

    IHierarchyViewModel* hierarchyViewModel(int hierarchyIndex) const override;
    QObject* noteListModel(int hierarchyIndex) const override;

private:
    static constexpr int kMappingCount =
        WhatSon::Sidebar::kHierarchyMaxIndex - WhatSon::Sidebar::kHierarchyMinIndex + 1;

    static int mappingOffsetForIndex(int hierarchyIndex) noexcept;
    static std::array<QPointer<IHierarchyViewModel>, kMappingCount> normalizedMappings(
        const QVector<Mapping>& mappings);
    static QVector<Mapping> exportedMappings(
        const std::array<QPointer<IHierarchyViewModel>, kMappingCount>& mappings);

    std::array<QPointer<IHierarchyViewModel>, kMappingCount> m_mappings{};
};
