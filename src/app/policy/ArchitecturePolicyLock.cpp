#include "ArchitecturePolicyLock.hpp"

#include <array>
#include <atomic>
#include <QDebug>

namespace
{
    using Layer = WhatSon::Policy::Layer;
    using Row = std::array<bool, 7>;

    constexpr int layerIndex(Layer layer) noexcept
    {
        return static_cast<int>(layer);
    }

    // Fixed architecture contract:
    // View -> ViewModel
    // ViewModel -> DataModel, Store, Parser, Creator
    // DataModel -> none
    // Store -> DataModel, Parser, Creator, FileSystem
    // Parser -> DataModel
    // Creator -> DataModel
    // FileSystem -> none
    constexpr std::array<Row, 7> kDependencyMatrix = {
        {
            /* View */ Row{false, true, false, false, false, false, false},
            /* ViewModel */ Row{false, false, true, true, true, true, false},
            /* DataModel */ Row{false, false, false, false, false, false, false},
            /* Store */ Row{false, false, true, false, true, true, true},
            /* Parser */ Row{false, false, true, false, false, false, false},
            /* Creator */ Row{false, false, true, false, false, false, false},
            /* FileSystem */ Row{false, false, false, false, false, false, false}
        }
    };

    std::atomic_bool g_architecturePolicyLocked{false};
}

namespace WhatSon::Policy
{
    bool ArchitecturePolicyLock::isLocked() noexcept
    {
        return g_architecturePolicyLocked.load(std::memory_order_acquire);
    }

    void ArchitecturePolicyLock::lock() noexcept
    {
        g_architecturePolicyLocked.store(true, std::memory_order_release);
    }

    const char* layerName(Layer layer) noexcept
    {
        switch (layer)
        {
        case Layer::View:
            return "View";
        case Layer::ViewModel:
            return "ViewModel";
        case Layer::DataModel:
            return "DataModel";
        case Layer::Store:
            return "Store";
        case Layer::Parser:
            return "Parser";
        case Layer::Creator:
            return "Creator";
        case Layer::FileSystem:
            return "FileSystem";
        default:
            return "Unknown";
        }
    }

    bool isDependencyAllowed(Layer from, Layer to) noexcept
    {
        const int fromIndex = layerIndex(from);
        const int toIndex = layerIndex(to);
        if (fromIndex < 0 || fromIndex >= static_cast<int>(kDependencyMatrix.size()))
        {
            return false;
        }
        const Row& row = kDependencyMatrix[fromIndex];
        if (toIndex < 0 || toIndex >= static_cast<int>(row.size()))
        {
            return false;
        }
        return row[toIndex];
    }

    bool assertDependencyAllowed(Layer from, Layer to, QString* errorMessage)
    {
        if (isDependencyAllowed(from, to))
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("architecture policy violation: %1 -> %2 is not allowed")
                .arg(QString::fromLatin1(layerName(from)),
                     QString::fromLatin1(layerName(to)));
        }
        return false;
    }

    bool verifyDependencyAllowed(
        Layer from,
        Layer to,
        const QString& context,
        QString* errorMessage)
    {
        QString violationMessage;
        if (assertDependencyAllowed(from, to, &violationMessage))
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        }

        const QString formattedMessage = context.trimmed().isEmpty()
            ? violationMessage
            : QStringLiteral("%1 (%2)").arg(violationMessage, context.trimmed());
        qWarning().noquote() << QStringLiteral("[whatson:policy][dependency] %1").arg(formattedMessage);
        if (errorMessage != nullptr)
        {
            *errorMessage = formattedMessage;
        }
        return false;
    }
} // namespace WhatSon::Policy
