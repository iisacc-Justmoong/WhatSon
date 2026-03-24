#pragma once

#include <QString>

namespace WhatSon::Policy
{
    enum class Layer : int
    {
        View = 0,
        ViewModel,
        DataModel,
        Store,
        Parser,
        Creator,
        FileSystem
    };

    class ArchitecturePolicyLock final
    {
    public:
        static bool isLocked() noexcept;
        static void lock() noexcept;

    private:
        ArchitecturePolicyLock() = delete;
    };

    const char* layerName(Layer layer) noexcept;
    bool isDependencyAllowed(Layer from, Layer to) noexcept;
    bool assertDependencyAllowed(Layer from, Layer to, QString* errorMessage = nullptr);
    bool verifyDependencyAllowed(Layer from,
                                 Layer to,
                                 const QString& context = QString(),
                                 QString* errorMessage = nullptr);
} // namespace WhatSon::Policy
