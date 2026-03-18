#pragma once

#include <QString>

class SelectedHubStore final
{
public:
    SelectedHubStore() = default;

    [[nodiscard]] QString selectedHubPath();
    [[nodiscard]] QString startupHubPath(const QString& blueprintFallbackHubPath);
    void clearSelectedHubPath();
    void setSelectedHubPath(const QString& hubPath);

private:
    [[nodiscard]] bool isStoredHubPathValid(const QString& hubPath) const;
    [[nodiscard]] QString normalizeHubPath(const QString& hubPath) const;
    [[nodiscard]] QString selectedHubSettingsKey() const;
};
