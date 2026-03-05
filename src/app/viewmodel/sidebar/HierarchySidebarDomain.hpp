#pragma once

namespace WhatSon::Sidebar
{
    enum class HierarchyDomain : int
    {
        Library = 0,
        Projects = 1,
        Bookmarks = 2,
        Tags = 3,
        Resources = 4,
        Progress = 5,
        Event = 6,
        Preset = 7
    };

    inline constexpr int kHierarchyMinIndex = static_cast<int>(HierarchyDomain::Library);
    inline constexpr int kHierarchyMaxIndex = static_cast<int>(HierarchyDomain::Preset);
    inline constexpr int kHierarchyDefaultIndex = static_cast<int>(HierarchyDomain::Library);

    inline int normalizeHierarchyIndex(int index) noexcept
    {
        if (index < kHierarchyMinIndex || index > kHierarchyMaxIndex)
        {
            return kHierarchyDefaultIndex;
        }
        return index;
    }
} // namespace WhatSon::Sidebar
