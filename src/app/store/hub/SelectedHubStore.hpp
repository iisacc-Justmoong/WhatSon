#pragma once

#include <QByteArray>
#include <QString>

class SelectedHubStore final
{
public:
    SelectedHubStore() = default;

    [[nodiscard]] QString selectedHubPath();
    [[nodiscard]] QByteArray selectedHubAccessBookmark();
    [[nodiscard]] QString startupHubPath(const QString& blueprintFallbackHubPath);
    void clearSelectedHubPath();
    void setSelectedHubPath(const QString& hubPath);
    void setSelectedHubSelection(const QString& hubPath, const QByteArray& accessBookmark);

private:
    [[nodiscard]] bool isStoredHubPathValid(const QString& hubPath) const;
    [[nodiscard]] QString normalizeHubPath(const QString& hubPath) const;
    [[nodiscard]] QString selectedHubSettingsKey() const;
    [[nodiscard]] QString selectedHubBookmarkSettingsKey() const;
};
