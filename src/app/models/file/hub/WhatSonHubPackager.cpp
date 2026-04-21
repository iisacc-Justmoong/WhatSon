#include "WhatSonHubPackager.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonHubPathUtils.hpp"

#include <QDir>
#include <QFileInfo>

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include "platform/Apple/WhatSonApplePackageAppearance.hpp"
#endif

QString WhatSonHubPackager::packageExtension() const
{
    return QStringLiteral(".wshub");
}

QString WhatSonHubPackager::normalizePackagePath(const QString& hubPackagePath) const
{
    QString normalizedHubPackagePath = WhatSon::HubPath::normalizeAbsolutePath(hubPackagePath.trimmed());
    if (!normalizedHubPackagePath.endsWith(packageExtension(), Qt::CaseInsensitive))
    {
        normalizedHubPackagePath += packageExtension();
    }
    return normalizedHubPackagePath;
}

bool WhatSonHubPackager::createPackageRoot(
    const QString& hubPackagePath,
    QString* outPackagePath,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.packager"),
        QStringLiteral("createPackageRoot.begin"),
        QStringLiteral("requestedPath=%1").arg(hubPackagePath));

    const QString trimmedHubPackagePath = hubPackagePath.trimmed();
    if (trimmedHubPackagePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub package path must not be empty.");
        }
        return false;
    }

    const QString absoluteHubPackagePath = normalizePackagePath(trimmedHubPackagePath);
    if (WhatSon::HubPath::isNonLocalUrl(absoluteHubPackagePath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub package path must resolve to a local directory.");
        }
        return false;
    }

    const QString hubParentDirectoryPath = WhatSon::HubPath::parentPath(absoluteHubPackagePath);
    if (hubParentDirectoryPath.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub parent directory must not be empty.");
        }
        return false;
    }

    if (!ensureDirectory(hubParentDirectoryPath, errorMessage))
    {
        return false;
    }

    const QFileInfo absoluteHubPackageInfo(absoluteHubPackagePath);
    if (absoluteHubPackageInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub already exists: %1").arg(absoluteHubPackagePath);
        }
        return false;
    }

    if (!ensureDirectory(absoluteHubPackagePath, errorMessage))
    {
        return false;
    }

    QString presentationWarning;
    if (!applyPackagePresentation(absoluteHubPackagePath, &presentationWarning))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.packager"),
            QStringLiteral("createPackageRoot.warning.packagePresentation"),
            presentationWarning);
    }

    if (outPackagePath != nullptr)
    {
        *outPackagePath = absoluteHubPackagePath;
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.packager"),
        QStringLiteral("createPackageRoot.success"),
        QStringLiteral("path=%1").arg(absoluteHubPackagePath));
    return true;
}

bool WhatSonHubPackager::ensureDirectory(const QString& absolutePath, QString* errorMessage) const
{
    QDir dir;
    if (dir.exists(absolutePath))
    {
        return true;
    }

    if (dir.mkpath(absolutePath))
    {
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to create directory: %1").arg(absolutePath);
    }
    return false;
}

bool WhatSonHubPackager::applyPackagePresentation(
    const QString& absolutePackagePath,
    QString* errorMessage) const
{
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    return WhatSon::Apple::PackageAppearance::applyPackageDirectoryPresentation(
        absolutePackagePath,
        errorMessage);
#else
    Q_UNUSED(absolutePackagePath);
    Q_UNUSED(errorMessage);
    return true;
#endif
}
