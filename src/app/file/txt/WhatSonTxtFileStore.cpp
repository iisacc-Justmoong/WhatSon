#include "WhatSonTxtFileStore.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>

#include <utility>

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

    QString sanitizeText(QString value)
    {
        value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return value.trimmed();
    }
} // namespace

WhatSonTxtFileStore::WhatSonTxtFileStore() = default;

WhatSonTxtFileStore::~WhatSonTxtFileStore() = default;

void WhatSonTxtFileStore::clear()
{
    m_filePath.clear();
    m_bodyPlainText.clear();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.store"),
                              QStringLiteral("clear"));
}

QString WhatSonTxtFileStore::filePath() const
{
    return m_filePath;
}

void WhatSonTxtFileStore::setFilePath(QString value)
{
    m_filePath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.store"),
                              QStringLiteral("setFilePath"),
                              QStringLiteral("value=%1").arg(m_filePath));
}

QString WhatSonTxtFileStore::bodyPlainText() const
{
    return m_bodyPlainText;
}

void WhatSonTxtFileStore::setBodyPlainText(QString value)
{
    m_bodyPlainText = sanitizeText(std::move(value));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.store"),
                              QStringLiteral("setBodyPlainText"),
                              QStringLiteral("length=%1").arg(m_bodyPlainText.size()));
}
