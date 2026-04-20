#include "WhatSonApplePackageAppearance.hpp"

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)

#import <Foundation/Foundation.h>

#include <QDir>
#include <QFileInfo>

namespace WhatSon::Apple::PackageAppearance
{
    bool applyPackageDirectoryPresentation(const QString& directoryPath, QString* errorMessage)
    {
        const QString normalizedDirectoryPath = QDir::cleanPath(directoryPath.trimmed());
        if (normalizedDirectoryPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Package directory path must not be empty.");
            }
            return false;
        }

        if (!QFileInfo(normalizedDirectoryPath).isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Package directory does not exist: %1").arg(normalizedDirectoryPath);
            }
            return false;
        }

        const QByteArray utf8Path = normalizedDirectoryPath.toUtf8();
        NSString* packagePath = [NSString stringWithUTF8String:utf8Path.constData()];
        if (packagePath == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to translate the package directory path for Apple packaging.");
            }
            return false;
        }

        NSURL* packageUrl = [NSURL fileURLWithPath:packagePath isDirectory:YES];
        if (packageUrl == nil)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to create an Apple package URL for: %1").arg(normalizedDirectoryPath);
            }
            return false;
        }

        NSError* resourceError = nil;
        const BOOL applied = [packageUrl setResourceValue:@YES forKey:NSURLIsPackageKey error:&resourceError];
        if (!applied)
        {
            if (errorMessage != nullptr)
            {
                const QString nativeMessage = resourceError == nil
                                                  ? QString()
                                                  : QString::fromUtf8(resourceError.localizedDescription.UTF8String).trimmed();
                *errorMessage = nativeMessage.isEmpty()
                                    ? QStringLiteral("Failed to apply Apple package presentation to: %1").arg(
                                        normalizedDirectoryPath)
                                    : QStringLiteral("Failed to apply Apple package presentation to %1: %2").arg(
                                        normalizedDirectoryPath,
                                        nativeMessage);
            }
            return false;
        }

        return true;
    }
}

#endif
