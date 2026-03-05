#pragma once

#include "viewmodel/sidebar/IHierarchyViewModelProvider.hpp"

class HierarchyViewModelProvider final : public IHierarchyViewModelProvider
{
    Q_OBJECT

public:
    struct Targets final
    {
        QObject* libraryViewModel = nullptr;
        QObject* projectsViewModel = nullptr;
        QObject* bookmarksViewModel = nullptr;
        QObject* tagsViewModel = nullptr;
        QObject* resourcesViewModel = nullptr;
        QObject* progressViewModel = nullptr;
        QObject* eventViewModel = nullptr;
        QObject* presetViewModel = nullptr;
    };

    explicit HierarchyViewModelProvider(QObject* parent = nullptr);
    ~HierarchyViewModelProvider() override;

    void setTargets(Targets targets);
    Targets targets() const noexcept;

    QObject* hierarchyViewModel(int hierarchyIndex) const override;
    QObject* noteListModel(int hierarchyIndex) const override;

private:
    static bool sameTargets(const Targets& lhs, const Targets& rhs) noexcept;
    static QObject* noteListModelFromViewModel(QObject* viewModel);

    Targets m_targets;
};
