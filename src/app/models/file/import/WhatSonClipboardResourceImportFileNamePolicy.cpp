#include "app/models/file/import/WhatSonClipboardResourceImportFileNamePolicy.hpp"

#include <QRandomGenerator>

namespace
{
    constexpr int kClipboardImportAssetKeyLength = 32;
    constexpr char kClipboardImportAssetAlphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    QString generateRandomAlphaNumericKey(const int length)
    {
        QString key;
        key.reserve(length);

        constexpr int alphabetLength = static_cast<int>(sizeof(kClipboardImportAssetAlphabet)) - 1;
        for (int index = 0; index < length; ++index)
        {
            const int randomIndex = QRandomGenerator::global()->bounded(alphabetLength);
            key.append(QLatin1Char(kClipboardImportAssetAlphabet[randomIndex]));
        }

        return key;
    }
}

QString WhatSon::Resources::generateClipboardImportAssetFileName()
{
    return QStringLiteral("%1.png").arg(generateRandomAlphaNumericKey(kClipboardImportAssetKeyLength));
}
