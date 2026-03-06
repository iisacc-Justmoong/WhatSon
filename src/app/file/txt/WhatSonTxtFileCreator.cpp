#include "WhatSonTxtFileCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace
{
    QString normalizePath(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }
} // namespace

WhatSonTxtFileCreator::WhatSonTxtFileCreator(QString libraryRootPath)
    : m_libraryRootPath(normalizePath(libraryRootPath))
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.creator"),
                              QStringLiteral("ctor"),
                              QStringLiteral("libraryRoot=%1").arg(m_libraryRootPath));
}

WhatSonTxtFileCreator::~WhatSonTxtFileCreator() = default;

void WhatSonTxtFileCreator::setLibraryRootPath(QString value)
{
    m_libraryRootPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.creator"),
                              QStringLiteral("setLibraryRootPath"),
                              QStringLiteral("value=%1").arg(m_libraryRootPath));
}

QString WhatSonTxtFileCreator::libraryRootPath() const
{
    return m_libraryRootPath;
}

bool WhatSonTxtFileCreator::createFile(
    const QString& preferredBaseName,
    const QString& text,
    QString* outFilePath,
    QString* errorMessage) const
{
    if (m_libraryRootPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("libraryRootPath must not be empty.");
        }
        return false;
    }

    QDir directory;
    if (!directory.mkpath(m_libraryRootPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create library directory: %1").arg(m_libraryRootPath);
        }
        return false;
    }

    const QString targetPath = uniqueFilePath(preferredBaseName);
    QFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open txt file for writing: %1 (%2)")
                .arg(targetPath, file.errorString());
        }
        return false;
    }

    const qint64 bytesWritten = file.write(text.toUtf8());
    if (bytesWritten < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write txt file: %1 (%2)")
                .arg(targetPath, file.errorString());
        }
        return false;
    }

    if (outFilePath != nullptr)
    {
        *outFilePath = targetPath;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.creator"),
                              QStringLiteral("createFile.success"),
                              QStringLiteral("path=%1 bytes=%2").arg(targetPath).arg(bytesWritten));
    return true;
}

QString WhatSonTxtFileCreator::sanitizedBaseName(const QString& value) const
{
    QString baseName = QFileInfo(value.trimmed()).completeBaseName().trimmed();
    if (baseName.isEmpty())
    {
        baseName = QFileInfo(value.trimmed()).fileName().trimmed();
    }

    baseName.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
    baseName.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral(""));

    if (baseName.isEmpty())
    {
        return QStringLiteral("Txt");
    }

    return baseName;
}

QString WhatSonTxtFileCreator::uniqueFilePath(const QString& preferredBaseName) const
{
    const QDir libraryDir(m_libraryRootPath);

    if (preferredBaseName.trimmed().isEmpty())
    {
        int sequence = 1;
        while (true)
        {
            const QString candidateName = QStringLiteral("Txt-%1.txt").arg(sequence);
            const QString candidatePath = libraryDir.filePath(candidateName);
            if (!QFileInfo::exists(candidatePath))
            {
                return QDir::cleanPath(candidatePath);
            }
            ++sequence;
        }
    }

    const QString baseName = sanitizedBaseName(preferredBaseName);
    QString candidateName = baseName + QStringLiteral(".txt");
    QString candidatePath = libraryDir.filePath(candidateName);
    if (!QFileInfo::exists(candidatePath))
    {
        return QDir::cleanPath(candidatePath);
    }

    int sequence = 2;
    while (true)
    {
        candidateName = QStringLiteral("%1-%2.txt").arg(baseName).arg(sequence);
        candidatePath = libraryDir.filePath(candidateName);
        if (!QFileInfo::exists(candidatePath))
        {
            return QDir::cleanPath(candidatePath);
        }
        ++sequence;
    }
}
