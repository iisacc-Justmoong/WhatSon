#pragma once

#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace WhatSon::Hierarchy::TagsSupport
{
    inline QString normalizePath(const QString& input)
    {
        return WhatSon::HubPath::normalizePath(input);
    }

    inline bool resolveContentsDirectories(
        const QString& wshubPath,
        QStringList* outContentsDirectories,
        QString* errorMessage = nullptr)
    {
        if (outContentsDirectories == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outContentsDirectories must not be null.");
            }
            return false;
        }

        outContentsDirectories->clear();

        const QString hubRootPath = normalizePath(wshubPath);
        if (hubRootPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must not be empty.");
            }
            return false;
        }

        const QFileInfo hubInfo(hubRootPath);
        if (!hubInfo.exists())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(hubRootPath);
            }
            return false;
        }

        if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")) || !hubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(hubRootPath);
            }
            return false;
        }

        const QDir hubDir(hubRootPath);

        const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents"));
        if (QFileInfo(fixedInternalPath).isDir())
        {
            outContentsDirectories->push_back(WhatSon::HubPath::normalizePath(fixedInternalPath));
        }

        const QStringList dynamicContentsDirectories = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicContentsDirectories)
        {
            outContentsDirectories->push_back(WhatSon::HubPath::joinPath(hubDir.path(), directoryName));
        }

        outContentsDirectories->removeDuplicates();

        if (outContentsDirectories->isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("No *.wscontents directory was found inside .wshub: %1").
                    arg(hubRootPath);
            }
            return false;
        }

        return true;
    }

    inline bool readUtf8File(const QString& filePath, QString* outText, QString* errorMessage = nullptr)
    {
        WhatSon::Debug::trace(
            QStringLiteral("hierarchy.io.support"),
            QStringLiteral("readUtf8File.begin"),
            QStringLiteral("path=%1").arg(filePath));

        if (outText == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outText must not be null.");
            }
            WhatSon::Debug::trace(
                QStringLiteral("hierarchy.io.support"),
                QStringLiteral("readUtf8File.failed"),
                QStringLiteral("path=%1 reason=outText is null").arg(filePath));
            return false;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file: %1").arg(filePath);
            }
            WhatSon::Debug::trace(
                QStringLiteral("hierarchy.io.support"),
                QStringLiteral("readUtf8File.failed"),
                QStringLiteral("path=%1 reason=open failed").arg(filePath));
            return false;
        }

        const QByteArray bytes = file.readAll();
        *outText = QString::fromUtf8(bytes);
        WhatSon::Debug::trace(
            QStringLiteral("hierarchy.io.support"),
            QStringLiteral("readUtf8File.success"),
            QStringLiteral("path=%1 bytes=%2").arg(filePath).arg(bytes.size()));
        return true;
    }
} // namespace WhatSon::Hierarchy::TagsSupport
