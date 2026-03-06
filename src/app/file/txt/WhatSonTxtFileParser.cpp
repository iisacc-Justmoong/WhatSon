#include "WhatSonTxtFileParser.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonTxtFileStore.hpp"

namespace
{
    QString normalizeTxt(QString text)
    {
        if (!text.isEmpty() && text.front() == QChar(0xFEFF))
        {
            text.remove(0, 1);
        }

        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        return text.trimmed();
    }
} // namespace

WhatSonTxtFileParser::WhatSonTxtFileParser() = default;

WhatSonTxtFileParser::~WhatSonTxtFileParser() = default;

bool WhatSonTxtFileParser::parse(
    const QString& rawText,
    WhatSonTxtFileStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.parser"),
                              QStringLiteral("parse.begin"),
                              QStringLiteral("bytes=%1").arg(rawText.toUtf8().size()));

    if (outStore == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outStore must not be null.");
        }
        return false;
    }

    outStore->clear();

    const QString normalizedText = normalizeTxt(rawText);
    outStore->setBodyPlainText(normalizedText);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("txt.parser"),
                              QStringLiteral("parse.success"),
                              QStringLiteral("length=%1").arg(outStore->bodyPlainText().size()));
    return true;
}
