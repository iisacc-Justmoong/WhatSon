#include "SelectedHubStore.hpp"
#include "models/file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace
{
    constexpr auto kSelectedHubPathSettingsKey = "workspace/selectedHubPath";
    constexpr auto kSelectedHubBookmarkSettingsKey = "workspace/selectedHubBookmark";
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
            settings.remove(selectedHubBookmarkSettingsKey());
        }
        settings.sync();
        return QString();
    }

    if (!isStoredHubPathValid(normalizedStoredPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.remove(selectedHubBookmarkSettingsKey());
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

QByteArray SelectedHubStore::selectedHubAccessBookmark()
{
    if (selectedHubPath().isEmpty())
    {
        return {};
    }

    QSettings settings;
    return settings.value(selectedHubBookmarkSettingsKey()).toByteArray();
}

void SelectedHubStore::clearSelectedHubPath()
{
    QSettings settings;
    settings.remove(selectedHubSettingsKey());
    settings.remove(selectedHubBookmarkSettingsKey());
    settings.sync();
}

void SelectedHubStore::setSelectedHubPath(const QString& hubPath)
{
    setSelectedHubSelection(hubPath, {});
}

void SelectedHubStore::setSelectedHubSelection(const QString& hubPath, const QByteArray& accessBookmark)
{
    const QString normalizedHubPath = normalizeHubPath(hubPath);
    QSettings settings;
    if (!isStoredHubPathValid(normalizedHubPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.remove(selectedHubBookmarkSettingsKey());
        settings.sync();
        return;
    }

    settings.setValue(selectedHubSettingsKey(), normalizedHubPath);
    if (accessBookmark.isEmpty())
    {
        settings.remove(selectedHubBookmarkSettingsKey());
    }
    else
    {
        settings.setValue(selectedHubBookmarkSettingsKey(), accessBookmark);
    }
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

QString SelectedHubStore::selectedHubBookmarkSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubBookmarkSettingsKey);
}
