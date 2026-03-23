#pragma once

#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/sidebar/IHierarchyViewModelProvider.hpp"

class HierarchyViewModelProvider final : public IHierarchyViewModelProvider
{
    Q_OBJECT

public:
    struct Targets final
    {
        IHierarchyViewModel* libraryViewModel = nullptr;
        IHierarchyViewModel* projectsViewModel = nullptr;
        IHierarchyViewModel* bookmarksViewModel = nullptr;
        IHierarchyViewModel* tagsViewModel = nullptr;
        IHierarchyViewModel* resourcesViewModel = nullptr;
        IHierarchyViewModel* progressViewModel = nullptr;
        IHierarchyViewModel* eventViewModel = nullptr;
        IHierarchyViewModel* presetViewModel = nullptr;
    };

    explicit HierarchyViewModelProvider(QObject* parent = nullptr);
    ~HierarchyViewModelProvider() override;

    void setTargets(Targets targets);
    Targets targets() const noexcept;

    IHierarchyViewModel* hierarchyViewModel(int hierarchyIndex) const override;
    QObject* noteListModel(int hierarchyIndex) const override;

private:
    static bool sameTargets(const Targets& lhs, const Targets& rhs) noexcept;
    Targets m_targets;
};
