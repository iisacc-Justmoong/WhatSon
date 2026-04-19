#include "SelectedHubStore.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace
{
    constexpr auto kSelectedHubPathSettingsKey = "workspace/selectedHubPath";
    constexpr auto kSelectedHubUrlSettingsKey = "workspace/selectedHubUrl";
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
            settings.remove(selectedHubUrlSettingsKey());
            settings.remove(selectedHubBookmarkSettingsKey());
        }
        settings.sync();
        return QString();
    }

    if (!isStoredHubPathValid(normalizedStoredPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.remove(selectedHubUrlSettingsKey());
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

QString SelectedHubStore::selectedHubUrl()
{
    QSettings settings;
    const QString rawStoredUrl = settings.value(selectedHubUrlSettingsKey()).toString();
    const QString normalizedStoredUrl = normalizeHubSelectionUrl(rawStoredUrl);
    if (normalizedStoredUrl.isEmpty())
    {
        if (!rawStoredUrl.trimmed().isEmpty())
        {
            settings.remove(selectedHubUrlSettingsKey());
            settings.sync();
        }
        return {};
    }

    if (normalizedStoredUrl != rawStoredUrl)
    {
        settings.setValue(selectedHubUrlSettingsKey(), normalizedStoredUrl);
        settings.sync();
    }

    return normalizedStoredUrl;
}

QByteArray SelectedHubStore::selectedHubAccessBookmark()
{
    if (selectedHubPath().isEmpty() && selectedHubUrl().isEmpty())
    {
        return {};
    }

    QSettings settings;
    return settings.value(selectedHubBookmarkSettingsKey()).toByteArray();
}

QString SelectedHubStore::startupHubPath()
{
    return selectedHubPath();
}

QString SelectedHubStore::startupHubUrl()
{
    return selectedHubUrl();
}

void SelectedHubStore::clearSelectedHubPath()
{
    QSettings settings;
    settings.remove(selectedHubSettingsKey());
    settings.remove(selectedHubUrlSettingsKey());
    settings.remove(selectedHubBookmarkSettingsKey());
    settings.sync();
}

void SelectedHubStore::setSelectedHubPath(const QString& hubPath)
{
    setSelectedHubSelection(hubPath, {}, {});
}

void SelectedHubStore::setSelectedHubSelection(
    const QString& hubPath,
    const QByteArray& accessBookmark,
    const QString& selectionUrl)
{
    const QString normalizedHubPath = normalizeHubPath(hubPath);
    QString normalizedSelectionUrl = normalizeHubSelectionUrl(selectionUrl);
    QSettings settings;
    if (!isStoredHubPathValid(normalizedHubPath))
    {
        settings.remove(selectedHubSettingsKey());
        settings.remove(selectedHubUrlSettingsKey());
        settings.remove(selectedHubBookmarkSettingsKey());
        settings.sync();
        return;
    }

    if (normalizedSelectionUrl.isEmpty())
    {
        const QUrl fallbackSelectionUrl = WhatSon::HubPath::urlFromPath(normalizedHubPath);
        if (fallbackSelectionUrl.isValid())
        {
            normalizedSelectionUrl = fallbackSelectionUrl.toString(QUrl::FullyEncoded).trimmed();
        }
    }

    settings.setValue(selectedHubSettingsKey(), normalizedHubPath);
    if (normalizedSelectionUrl.isEmpty())
    {
        settings.remove(selectedHubUrlSettingsKey());
    }
    else
    {
        settings.setValue(selectedHubUrlSettingsKey(), normalizedSelectionUrl);
    }
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

QString SelectedHubStore::normalizeHubSelectionUrl(const QString& selectionUrl) const
{
    const QString trimmedSelectionUrl = selectionUrl.trimmed();
    if (trimmedSelectionUrl.isEmpty())
    {
        return {};
    }

    const QUrl parsedUrl(trimmedSelectionUrl);
    if (!parsedUrl.isValid() || parsedUrl.scheme().trimmed().isEmpty())
    {
        return {};
    }

    if (parsedUrl.isLocalFile())
    {
        const QString normalizedLocalPath = WhatSon::HubPath::normalizeAbsolutePath(parsedUrl.toLocalFile());
        if (normalizedLocalPath.isEmpty())
        {
            return {};
        }
        return QUrl::fromLocalFile(normalizedLocalPath).toString(QUrl::FullyEncoded).trimmed();
    }

    return parsedUrl.toString(QUrl::FullyEncoded).trimmed();
}

QString SelectedHubStore::selectedHubSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubPathSettingsKey);
}

QString SelectedHubStore::selectedHubUrlSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubUrlSettingsKey);
}

QString SelectedHubStore::selectedHubBookmarkSettingsKey() const
{
    return QString::fromLatin1(kSelectedHubBookmarkSettingsKey);
}
