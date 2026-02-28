#include "WhatSonTagsFileReader.hpp"

#include <QFile>

WhatSonTagsFileReader::WhatSonTagsFileReader() = default;

WhatSonTagsFileReader::~WhatSonTagsFileReader() = default;

bool WhatSonTagsFileReader::readTextFile(
    const QString& filePath,
    QString* outText,
    QString* errorMessage) const
{
    if (outText == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outText must not be null.");
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open tags file: %1").arg(filePath);
        }
        return false;
    }

    *outText = QString::fromUtf8(file.readAll());
    return true;
}
