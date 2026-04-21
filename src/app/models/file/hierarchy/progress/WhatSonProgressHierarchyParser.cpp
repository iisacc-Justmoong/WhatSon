#include "WhatSonProgressHierarchyParser.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "WhatSonProgressHierarchyStore.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

namespace
{
    QStringList defaultStates()
    {
        return {
            QStringLiteral("Ready"),
            QStringLiteral("Pending"),
            QStringLiteral("InProgress"),
            QStringLiteral("Done")
        };
    }

    QStringList parseStatesArray(const QJsonArray& array)
    {
        QStringList states;
        states.reserve(array.size());

        for (const QJsonValue& value : array)
        {
            if (!value.isString())
            {
                continue;
            }

            const QString state = value.toString().trimmed();
            if (!state.isEmpty())
            {
                states.push_back(state);
            }
        }

        return states;
    }

    int parseFirstInteger(const QString& text)
    {
        const QRegularExpression integerRegex(QStringLiteral(R"((-?\d+))"));
        const QRegularExpressionMatch match = integerRegex.match(text);
        if (!match.hasMatch())
        {
            return 0;
        }

        bool ok = false;
        const int value = match.captured(1).toInt(&ok);
        return ok ? value : 0;
    }
} // namespace

WhatSonProgressHierarchyParser::WhatSonProgressHierarchyParser() = default;

WhatSonProgressHierarchyParser::~WhatSonProgressHierarchyParser() = default;

bool WhatSonProgressHierarchyParser::parse(
    const QString& rawText,
    WhatSonProgressHierarchyStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.progress.parser"),
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

    const QString trimmedText = rawText.trimmed();
    if (trimmedText.isEmpty() || trimmedText.startsWith(QStringLiteral("<DOCTYPE-XML"), Qt::CaseInsensitive))
    {
        outStore->setProgressValue(0);
        outStore->setProgressStates(defaultStates());
        return true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(trimmedText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject())
    {
        const QJsonObject object = document.object();

        QStringList states;
        const QJsonValue statesValue = object.value(QStringLiteral("states"));
        if (statesValue.isArray())
        {
            states = parseStatesArray(statesValue.toArray());
        }
        else
        {
            const QJsonValue enumsValue = object.value(QStringLiteral("enums"));
            if (enumsValue.isArray())
            {
                states = parseStatesArray(enumsValue.toArray());
            }
        }
        outStore->setProgressStates(states);

        const QJsonValue progressValue = object.value(QStringLiteral("progress"));
        if (progressValue.isDouble())
        {
            outStore->setProgressValue(static_cast<int>(progressValue.toDouble()));
            return true;
        }

        const QJsonValue valueAlias = object.value(QStringLiteral("value"));
        if (valueAlias.isDouble())
        {
            outStore->setProgressValue(static_cast<int>(valueAlias.toDouble()));
            return true;
        }
    }

    outStore->setProgressStates(defaultStates());
    outStore->setProgressValue(parseFirstInteger(trimmedText));

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.progress.parser"),
                              QStringLiteral("parse.fallback"),
                              QStringLiteral("value=%1").arg(outStore->progressValue()));
    return true;
}
