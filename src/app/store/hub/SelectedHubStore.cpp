#include "SelectedHubStore.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"

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
    QString normalizedStoredPath = normalizeHubPath(rawStoredPath);

    if (normalizedStoredPath.isEmpty())
    {
        if (!rawStoredPath.trimmed().isEmpty())
        {
            settings.remove(selectedHubSettingsKey());
        }
        settings.sync();
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
    const QString normalizedHubPath = WhatSon::HubPath::normalizePath(hubPath);
    if (normalizedHubPath.trimmed().isEmpty())
        return false;

    if (WhatSon::Android::Storage::isSupportedUri(normalizedHubPath))
        return true;

    const QFileInfo hubInfo(normalizedHubPath);
    return hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive);
}

QString SelectedHubStore::normalizeHubPath(const QString& hubPath) const
{
    const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (normalizedHubPath.isEmpty())
        return {};

    if (WhatSon::Android::Storage::isMountedHubPath(normalizedHubPath))
    {
        const QString sourceUri = WhatSon::Android::Storage::mountedHubSourceUri(normalizedHubPath);
        if (!sourceUri.trimmed().isEmpty())
            return WhatSon::HubPath::normalizePath(sourceUri);
    }

    return normalizedHubPath;
}

QString SelectedHubStore::selectedHubSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubPathSettingsKey);
}
