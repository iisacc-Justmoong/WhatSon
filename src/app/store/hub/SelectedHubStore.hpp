#pragma once

#include "ISelectedHubStore.hpp"

class SelectedHubStore final : public ISelectedHubStore
{
public:
    SelectedHubStore() = default;
    ~SelectedHubStore() override = default;

    [[nodiscard]] QString selectedHubPath() override;
    [[nodiscard]] QString selectedHubUrl() override;
    [[nodiscard]] QByteArray selectedHubAccessBookmark() override;
    [[nodiscard]] QString startupHubPath() override;
    [[nodiscard]] QString startupHubUrl() override;
    void clearSelectedHubPath() override;
    void setSelectedHubPath(const QString& hubPath) override;
    void setSelectedHubSelection(
        const QString& hubPath,
        const QByteArray& accessBookmark,
        const QString& selectionUrl = QString()) override;

private:
    [[nodiscard]] bool isStoredHubPathValid(const QString& hubPath) const;
    [[nodiscard]] QString normalizeHubPath(const QString& hubPath) const;
    [[nodiscard]] QString normalizeHubSelectionUrl(const QString& selectionUrl) const;
    [[nodiscard]] QString selectedHubSettingsKey() const;
    [[nodiscard]] QString selectedHubUrlSettingsKey() const;
    [[nodiscard]] QString selectedHubBookmarkSettingsKey() const;
};
