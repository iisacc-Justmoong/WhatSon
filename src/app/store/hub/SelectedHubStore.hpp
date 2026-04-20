#pragma once

#include "ISelectedHubStore.hpp"

class SelectedHubStore final : public ISelectedHubStore
{
public:
    SelectedHubStore() = default;
    ~SelectedHubStore() override = default;

    [[nodiscard]] QString selectedHubPath() override;
    [[nodiscard]] QByteArray selectedHubAccessBookmark() override;
    void clearSelectedHubPath() override;
    void setSelectedHubPath(const QString& hubPath) override;
    void setSelectedHubSelection(const QString& hubPath, const QByteArray& accessBookmark) override;

private:
    [[nodiscard]] bool isStoredHubPathValid(const QString& hubPath) const;
    [[nodiscard]] QString normalizeHubPath(const QString& hubPath) const;
    [[nodiscard]] QString selectedHubSettingsKey() const;
    [[nodiscard]] QString selectedHubBookmarkSettingsKey() const;
};
