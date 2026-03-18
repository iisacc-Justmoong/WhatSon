#include "SelectedHubStore.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace
{
    constexpr auto kSelectedHubPathSettingsKey = "workspace/selectedHubPath";
}

QString SelectedHubStore::selectedHubPath()
{
    QSettings settings;
    const QString rawStoredPath = settings.value(selectedHubSettingsKey()).toString();
    const QString normalizedStoredPath = normalizeHubPath(rawStoredPath);
    if (normalizedStoredPath.isEmpty())
    {
        if (!rawStoredPath.trimmed().isEmpty())
        {
            settings.remove(selectedHubSettingsKey());
            settings.sync();
        }
        return QString();
    }

    if (!isStoredHubPathValid(normalizedStoredPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.sync();
        return QString();
    }

    if (normalizedStoredPath != rawStoredPath)
    {
        settings.setValue(selectedHubSettingsKey(), normalizedStoredPath);
        settings.sync();
    }

    return normalizedStoredPath;
}

QString SelectedHubStore::startupHubPath(const QString& blueprintFallbackHubPath)
{
    const QString storedHubPath = selectedHubPath();
    if (!storedHubPath.isEmpty())
        return storedHubPath;

    const QString normalizedFallbackPath = normalizeHubPath(blueprintFallbackHubPath);
    if (!isStoredHubPathValid(normalizedFallbackPath))
        return QString();
    return normalizedFallbackPath;
}

void SelectedHubStore::clearSelectedHubPath()
{
    QSettings settings;
    settings.remove(selectedHubSettingsKey());
    settings.sync();
}

void SelectedHubStore::setSelectedHubPath(const QString& hubPath)
{
    const QString normalizedHubPath = normalizeHubPath(hubPath);
    QSettings settings;
    if (!isStoredHubPathValid(normalizedHubPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.sync();
        return;
    }

    settings.setValue(selectedHubSettingsKey(), normalizedHubPath);
    settings.sync();
}

bool SelectedHubStore::isStoredHubPathValid(const QString& hubPath) const
{
    if (hubPath.trimmed().isEmpty())
        return false;

    const QFileInfo hubInfo(hubPath);
    return hubInfo.exists()
        && hubInfo.isDir()
        && hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive);
}

QString SelectedHubStore::normalizeHubPath(const QString& hubPath) const
{
    if (hubPath.trimmed().isEmpty())
        return QString();
    return QDir::cleanPath(QFileInfo(hubPath).absoluteFilePath());
}

QString SelectedHubStore::selectedHubSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubPathSettingsKey);
}
