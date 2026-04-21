#pragma once

#include <QRandomGenerator>
#include <QString>

namespace WhatSon::FolderIdentity
{
    inline constexpr int kUuidLength = 64;

    inline QString normalizeFolderUuid(QString value)
    {
        value = value.trimmed();
        if (value.size() != kUuidLength)
        {
            return {};
        }

        for (const QChar& character : value)
        {
            if (!character.isDigit() && !character.isLetter())
            {
                return {};
            }
        }

        return value;
    }

    inline bool isValidFolderUuid(const QString& value)
    {
        return !normalizeFolderUuid(value).isEmpty();
    }

    inline QString createFolderUuid()
    {
        static const QString kAlphabet =
            QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

        QString uuid;
        uuid.reserve(kUuidLength);
        for (int index = 0; index < kUuidLength; ++index)
        {
            uuid.push_back(kAlphabet.at(QRandomGenerator::global()->bounded(kAlphabet.size())));
        }
        return uuid;
    }
} // namespace WhatSon::FolderIdentity
