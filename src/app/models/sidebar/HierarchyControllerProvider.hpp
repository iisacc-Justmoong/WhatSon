#pragma once

#include "app/models/file/hierarchy/IHierarchyController.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/sidebar/IHierarchyControllerProvider.hpp"

#include <QPointer>
#include <QVector>

#include <array>

class HierarchyControllerProvider final : public IHierarchyControllerProvider
{
    Q_OBJECT

public:
    struct Mapping final
    {
        int hierarchyIndex = WhatSon::Sidebar::kHierarchyDefaultIndex;
        IHierarchyController* controller = nullptr;
    };

    explicit HierarchyControllerProvider(QObject* parent = nullptr);
    ~HierarchyControllerProvider() override;

    void setMappings(QVector<Mapping> mappings);
    QVector<Mapping> mappings() const;

    IHierarchyController* hierarchyController(int hierarchyIndex) const override;
    QObject* noteListModel(int hierarchyIndex) const override;

private:
    static constexpr int kMappingCount =
        WhatSon::Sidebar::kHierarchyMaxIndex - WhatSon::Sidebar::kHierarchyMinIndex + 1;

    static int mappingOffsetForIndex(int hierarchyIndex) noexcept;
    static std::array<QPointer<IHierarchyController>, kMappingCount> normalizedMappings(
        const QVector<Mapping>& mappings);
    static QVector<Mapping> exportedMappings(
        const std::array<QPointer<IHierarchyController>, kMappingCount>& mappings);

    std::array<QPointer<IHierarchyController>, kMappingCount> m_mappings{};
};
