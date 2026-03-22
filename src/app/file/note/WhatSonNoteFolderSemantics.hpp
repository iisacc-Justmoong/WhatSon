#pragma once

#include <QString>
#include <QStringList>
#include <QRegularExpression>

namespace WhatSon::NoteFolders
{
    inline QString normalizeFolderPath(QString value)
    {
        value = value.trimmed();
        value.replace(QLatin1Char('\\'), QLatin1Char('/'));
        while (value.contains(QStringLiteral("//")))
        {
            value.replace(QStringLiteral("//"), QStringLiteral("/"));
        }
        while (value.startsWith(QLatin1Char('/')))
        {
            value.remove(0, 1);
        }
        while (value.endsWith(QLatin1Char('/')))
        {
            value.chop(1);
        }
        return value;
    }

    inline bool usesReservedTodayFolderSegment(const QString& value)
    {
        const QString normalizedPath = normalizeFolderPath(value);
        if (normalizedPath.isEmpty())
        {
            return false;
        }

        const QStringList segments = normalizedPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (const QString& segment : segments)
        {
            if (segment.trimmed().toCaseFolded() == QStringLiteral("today"))
            {
                return true;
            }
        }

        return false;
    }

    struct RawFoldersBlockState final
    {
        bool blockPresent = false;
        bool hasConcreteEntry = false;
    };

    inline RawFoldersBlockState inspectRawFoldersBlock(const QString& wsnHeadText)
    {
        static const QRegularExpression foldersBlockRegex(
            QStringLiteral(R"(<\s*folders\b[^>]*>([\s\S]*?)<\s*/\s*folders\s*>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression folderTagRegex(
            QStringLiteral(R"(<\s*folder\b[^>]*>([\s\S]*?)<\s*/\s*folder\s*>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression xmlTagRegex(QStringLiteral(R"(<[^>]+>)"));

        RawFoldersBlockState state;
        const QRegularExpressionMatch foldersMatch = foldersBlockRegex.match(wsnHeadText);
        if (!foldersMatch.hasMatch())
        {
            return state;
        }

        state.blockPresent = true;
        QString foldersInnerText = foldersMatch.captured(1);

        QRegularExpressionMatchIterator folderIt = folderTagRegex.globalMatch(foldersInnerText);
        while (folderIt.hasNext())
        {
            const QRegularExpressionMatch folderMatch = folderIt.next();
            if (!folderMatch.captured(1).trimmed().isEmpty())
            {
                state.hasConcreteEntry = true;
                return state;
            }
        }

        foldersInnerText.remove(folderTagRegex);
        foldersInnerText.remove(xmlTagRegex);
        state.hasConcreteEntry = !foldersInnerText.trimmed().isEmpty();
        return state;
    }
} // namespace WhatSon::NoteFolders
