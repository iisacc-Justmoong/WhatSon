#include "WhatSonStructuredTagLinter.hpp"

#include <QDate>
#include <QRegularExpression>
#include <QVariantList>

namespace
{
    const QRegularExpression kAgendaOpenPattern(
        QStringLiteral(R"(<agenda\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaClosePattern(
        QStringLiteral(R"(</agenda>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaSelfClosingPattern(
        QStringLiteral(R"(<agenda\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaBlockPattern(
        QStringLiteral(R"(<agenda\b([^>]*)>([\s\S]*?)</agenda>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskOpenPattern(
        QStringLiteral(R"(<task\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskClosePattern(
        QStringLiteral(R"(</task>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskSelfClosingPattern(
        QStringLiteral(R"(<task\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskBlockPattern(
        QStringLiteral(R"(<task\b([^>]*)>([\s\S]*?)</task>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutOpenPattern(
        QStringLiteral(R"(<callout\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutClosePattern(
        QStringLiteral(R"(</callout>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutSelfClosingPattern(
        QStringLiteral(R"(<callout\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutBlockPattern(
        QStringLiteral(R"(<callout\b([^>]*)>([\s\S]*?)</callout>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCanonicalBreakPattern(
        QStringLiteral(R"(</break>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kLegacyBreakPattern(
        QStringLiteral(R"((?:<\s*(?:break|hr)\b[^>]*?/?>|</\s*hr\s*>))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTagAttributePattern(
        QStringLiteral(R"ATTR(\b([A-Za-z_][A-Za-z0-9_.:-]*)\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR"),
        QRegularExpression::CaseInsensitiveOption);

    QString normalizePlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
    }

    QString decodeXmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    QString escapeXmlAttributeValue(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    int regexMatchCount(const QString& text, const QRegularExpression& pattern)
    {
        int count = 0;
        QRegularExpressionMatchIterator iterator = pattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (match.hasMatch())
            {
                ++count;
            }
        }
        return count;
    }

    QVariantMap issueMap(
        const QString& code,
        const QString& message,
        const QVariantMap& context = {})
    {
        QVariantMap issue;
        issue.insert(QStringLiteral("code"), code);
        issue.insert(QStringLiteral("message"), message);
        if (!context.isEmpty())
        {
            issue.insert(QStringLiteral("context"), context);
        }
        return issue;
    }

    QString attributeValue(const QString& rawTagText, const QString& attributeName)
    {
        const QString normalizedAttributeName = attributeName.trimmed();
        if (normalizedAttributeName.isEmpty())
        {
            return {};
        }

        const QRegularExpression attributePattern(
            QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR")
                .arg(QRegularExpression::escape(normalizedAttributeName)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawTagText);
        if (!match.hasMatch())
        {
            return {};
        }

        if (match.capturedStart(1) >= 0)
        {
            return decodeXmlEntities(match.captured(1));
        }
        if (match.capturedStart(2) >= 0)
        {
            return decodeXmlEntities(match.captured(2));
        }
        if (match.capturedStart(3) >= 0)
        {
            return decodeXmlEntities(match.captured(3));
        }
        return {};
    }

    int attributeCount(QString rawTagText)
    {
        int count = 0;
        rawTagText.remove(QLatin1Char('/'));
        QRegularExpressionMatchIterator iterator = kTagAttributePattern.globalMatch(rawTagText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (match.hasMatch())
            {
                ++count;
            }
        }
        return count;
    }

    bool hasCanonicalIsoDate(const QString& rawDate)
    {
        const QString decodedDate = decodeXmlEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        return parsedDate.isValid() && parsedDate.toString(QStringLiteral("yyyy-MM-dd")) == decodedDate;
    }

    QString normalizedAgendaDate(const QString& rawDate)
    {
        const QString decodedDate = decodeXmlEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        if (parsedDate.isValid())
        {
            return parsedDate.toString(QStringLiteral("yyyy-MM-dd"));
        }
        return QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    }

    bool isCanonicalDoneLiteral(const QString& rawDone)
    {
        const QString decoded = decodeXmlEntities(rawDone).trimmed();
        return decoded == QStringLiteral("true") || decoded == QStringLiteral("false");
    }

    bool normalizedTaskDoneValue(const QString& rawDone)
    {
        const QString lowered = decodeXmlEntities(rawDone).trimmed().toCaseFolded();
        return lowered == QStringLiteral("true")
            || lowered == QStringLiteral("1")
            || lowered == QStringLiteral("yes");
    }

    QString canonicalAgendaStartTag(const QString& rawTagText)
    {
        return QStringLiteral("<agenda date=\"%1\">")
            .arg(escapeXmlAttributeValue(normalizedAgendaDate(attributeValue(rawTagText, QStringLiteral("date")))));
    }

    QString canonicalTaskStartTag(const QString& rawTagText)
    {
        const bool done = normalizedTaskDoneValue(attributeValue(rawTagText, QStringLiteral("done")));
        return QStringLiteral("<task done=\"%1\">")
            .arg(done ? QStringLiteral("true") : QStringLiteral("false"));
    }

    QString canonicalCalloutStartTag()
    {
        return QStringLiteral("<callout>");
    }

    QString canonicalEmptyTask()
    {
        return QStringLiteral("<task done=\"false\"> </task>");
    }

    QString normalizeLegacyBreakTags(QString text)
    {
        text.replace(
            QRegularExpression(
                QStringLiteral(R"(</\s*hr\s*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("</break>"));
        text.replace(
            QRegularExpression(
                QStringLiteral(R"(<\s*(?!/)(?:break|hr)\b[^>]*?/?>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("</break>"));
        return text;
    }

    QString normalizeSelfClosingCallouts(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kCalloutSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += QStringLiteral("<callout></callout>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeSelfClosingTasks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kTaskSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalTaskStartTag(match.captured(0));
            output += QLatin1Char(' ');
            output += QStringLiteral("</task>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeSelfClosingAgendas(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kAgendaSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalAgendaStartTag(match.captured(0));
            output += canonicalEmptyTask();
            output += QStringLiteral("</agenda>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeCalloutBlocks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kCalloutBlockPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalCalloutStartTag();
            output += match.captured(2);
            output += QStringLiteral("</callout>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeAgendaInnerTasks(const QString& innerSourceText)
    {
        QString output;
        int cursor = 0;
        int taskCount = 0;
        QRegularExpressionMatchIterator iterator = kTaskBlockPattern.globalMatch(innerSourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += innerSourceText.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalTaskStartTag(match.captured(0));
            output += match.captured(2);
            output += QStringLiteral("</task>");
            cursor = match.capturedEnd(0);
            ++taskCount;
        }
        output += innerSourceText.mid(cursor);

        if (taskCount > 0)
        {
            return output;
        }

        const QString trimmedInnerSourceText = innerSourceText.trimmed();
        if (trimmedInnerSourceText.isEmpty())
        {
            return canonicalEmptyTask();
        }

        if (trimmedInnerSourceText.contains(QLatin1Char('<'))
            || trimmedInnerSourceText.contains(QLatin1Char('>')))
        {
            return innerSourceText;
        }

        return canonicalTaskStartTag(QStringLiteral("<task>"))
            + innerSourceText
            + QStringLiteral("</task>");
    }

    QString normalizeAgendaBlocks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kAgendaBlockPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalAgendaStartTag(match.captured(0));
            output += normalizeAgendaInnerTasks(match.captured(2));
            output += QStringLiteral("</agenda>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QVariantList mergedIssues(
        const QVariantList& first,
        const QVariantList& second,
        const QVariantList& third = {})
    {
        QVariantList issues = first;
        for (const QVariant& issue : second)
        {
            issues.push_back(issue);
        }
        for (const QVariant& issue : third)
        {
            issues.push_back(issue);
        }
        return issues;
    }

    QVariantMap normalizedVerificationFallback(const QVariantMap& verification, const QString& tagName)
    {
        if (!verification.isEmpty())
        {
            return verification;
        }
        return QVariantMap {
            {QStringLiteral("tagName"), tagName},
            {QStringLiteral("wellFormed"), true},
            {QStringLiteral("sourceLength"), 0},
            {QStringLiteral("issues"), QVariantList()}
        };
    }
}

WhatSonStructuredTagLinter::WhatSonStructuredTagLinter() = default;

WhatSonStructuredTagLinter::~WhatSonStructuredTagLinter() = default;

QString WhatSonStructuredTagLinter::normalizeStructuredSourceText(const QString& sourceText) const
{
    QString normalizedSourceText = normalizePlainText(sourceText);
    if (normalizedSourceText.isEmpty())
    {
        return normalizedSourceText;
    }

    normalizedSourceText = normalizeLegacyBreakTags(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingCallouts(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingTasks(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingAgendas(normalizedSourceText);
    normalizedSourceText = normalizeCalloutBlocks(normalizedSourceText);
    normalizedSourceText = normalizeAgendaBlocks(normalizedSourceText);
    return normalizedSourceText;
}

QVariantMap WhatSonStructuredTagLinter::buildBreakVerification(const QString& sourceText) const
{
    const int canonicalBreakCount = regexMatchCount(sourceText, kCanonicalBreakPattern);
    const int legacyBreakCount = regexMatchCount(sourceText, kLegacyBreakPattern);

    QVariantList issues;
    if (legacyBreakCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("break_legacy_variant"),
            QStringLiteral("Break tag uses a legacy or non-canonical variant."),
            {
                {QStringLiteral("legacyBreakCount"), legacyBreakCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("break")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("canonicalBreakCount"), canonicalBreakCount},
        {QStringLiteral("legacyBreakCount"), legacyBreakCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildAgendaVerification(
    const QString& sourceText,
    const int parsedAgendaCount,
    const int parsedTaskCount,
    const int invalidAgendaChildCount) const
{
    const int agendaOpenCount = regexMatchCount(sourceText, kAgendaOpenPattern);
    const int agendaCloseCount = regexMatchCount(sourceText, kAgendaClosePattern);
    const int agendaSelfClosingCount = regexMatchCount(sourceText, kAgendaSelfClosingPattern);
    const int taskOpenCount = regexMatchCount(sourceText, kTaskOpenPattern);
    const int taskCloseCount = regexMatchCount(sourceText, kTaskClosePattern);
    const int taskSelfClosingCount = regexMatchCount(sourceText, kTaskSelfClosingPattern);

    int invalidAgendaDateCount = 0;
    QRegularExpressionMatchIterator agendaIterator = kAgendaOpenPattern.globalMatch(sourceText);
    while (agendaIterator.hasNext())
    {
        const QRegularExpressionMatch match = agendaIterator.next();
        if (!match.hasMatch())
        {
            continue;
        }
        if (!hasCanonicalIsoDate(attributeValue(match.captured(0), QStringLiteral("date"))))
        {
            ++invalidAgendaDateCount;
        }
    }

    int missingTaskDoneCount = 0;
    int nonCanonicalTaskDoneCount = 0;
    QRegularExpressionMatchIterator taskIterator = kTaskOpenPattern.globalMatch(sourceText);
    while (taskIterator.hasNext())
    {
        const QRegularExpressionMatch match = taskIterator.next();
        if (!match.hasMatch())
        {
            continue;
        }

        const QString rawDone = attributeValue(match.captured(0), QStringLiteral("done"));
        if (rawDone.isEmpty())
        {
            ++missingTaskDoneCount;
            continue;
        }
        if (!isCanonicalDoneLiteral(rawDone))
        {
            ++nonCanonicalTaskDoneCount;
        }
    }

    QVariantList issues;
    if (agendaOpenCount != agendaCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_pair_mismatch"),
            QStringLiteral("Agenda open/close tag counts differ.")));
    }
    if (agendaOpenCount != parsedAgendaCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_parse_mismatch"),
            QStringLiteral("Agenda parser did not confirm every agenda tag pair.")));
    }
    if (taskOpenCount != taskCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_pair_mismatch"),
            QStringLiteral("Task open/close tag counts differ.")));
    }
    if (taskOpenCount != parsedTaskCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_parse_mismatch"),
            QStringLiteral("Agenda parser did not confirm every task tag pair.")));
    }
    if (invalidAgendaChildCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_invalid_children"),
            QStringLiteral("Agenda contains non-task child content."),
            {
                {QStringLiteral("invalidAgendaChildCount"), invalidAgendaChildCount}
            }));
    }
    if (invalidAgendaDateCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_invalid_date"),
            QStringLiteral("Agenda date is missing or not canonical YYYY-MM-DD."),
            {
                {QStringLiteral("invalidAgendaDateCount"), invalidAgendaDateCount}
            }));
    }
    if (agendaSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_self_closing"),
            QStringLiteral("Self-closing agenda tags must be expanded into explicit agenda/task blocks."),
            {
                {QStringLiteral("agendaSelfClosingCount"), agendaSelfClosingCount}
            }));
    }
    if (taskSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_self_closing"),
            QStringLiteral("Self-closing task tags must be expanded into explicit task blocks."),
            {
                {QStringLiteral("taskSelfClosingCount"), taskSelfClosingCount}
            }));
    }
    if (missingTaskDoneCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_missing_done"),
            QStringLiteral("Task tag is missing the mandatory done attribute."),
            {
                {QStringLiteral("missingTaskDoneCount"), missingTaskDoneCount}
            }));
    }
    if (nonCanonicalTaskDoneCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_noncanonical_done"),
            QStringLiteral("Task done attribute is not canonical true|false."),
            {
                {QStringLiteral("nonCanonicalTaskDoneCount"), nonCanonicalTaskDoneCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("agenda")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("agendaOpenCount"), agendaOpenCount},
        {QStringLiteral("agendaCloseCount"), agendaCloseCount},
        {QStringLiteral("agendaParsedCount"), parsedAgendaCount},
        {QStringLiteral("agendaSelfClosingCount"), agendaSelfClosingCount},
        {QStringLiteral("agendaInvalidDateCount"), invalidAgendaDateCount},
        {QStringLiteral("taskOpenCount"), taskOpenCount},
        {QStringLiteral("taskCloseCount"), taskCloseCount},
        {QStringLiteral("taskParsedCount"), parsedTaskCount},
        {QStringLiteral("taskSelfClosingCount"), taskSelfClosingCount},
        {QStringLiteral("taskMissingDoneCount"), missingTaskDoneCount},
        {QStringLiteral("taskNonCanonicalDoneCount"), nonCanonicalTaskDoneCount},
        {QStringLiteral("invalidAgendaChildCount"), invalidAgendaChildCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildCalloutVerification(
    const QString& sourceText,
    const int parsedCalloutCount) const
{
    const int calloutOpenCount = regexMatchCount(sourceText, kCalloutOpenPattern);
    const int calloutCloseCount = regexMatchCount(sourceText, kCalloutClosePattern);
    const int calloutSelfClosingCount = regexMatchCount(sourceText, kCalloutSelfClosingPattern);

    int calloutAttributeCount = 0;
    QRegularExpressionMatchIterator iterator = kCalloutOpenPattern.globalMatch(sourceText);
    while (iterator.hasNext())
    {
        const QRegularExpressionMatch match = iterator.next();
        if (!match.hasMatch())
        {
            continue;
        }
        if (attributeCount(match.captured(0)) > 0)
        {
            ++calloutAttributeCount;
        }
    }

    QVariantList issues;
    if (calloutOpenCount != calloutCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_pair_mismatch"),
            QStringLiteral("Callout open/close tag counts differ.")));
    }
    if (calloutOpenCount != parsedCalloutCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_parse_mismatch"),
            QStringLiteral("Callout parser did not confirm every callout tag pair.")));
    }
    if (calloutSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_self_closing"),
            QStringLiteral("Self-closing callout tags must be expanded into explicit callout blocks."),
            {
                {QStringLiteral("calloutSelfClosingCount"), calloutSelfClosingCount}
            }));
    }
    if (calloutAttributeCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_noncanonical_attributes"),
            QStringLiteral("Callout start tags must not carry custom attributes."),
            {
                {QStringLiteral("calloutAttributeCount"), calloutAttributeCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("callout")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("calloutOpenCount"), calloutOpenCount},
        {QStringLiteral("calloutCloseCount"), calloutCloseCount},
        {QStringLiteral("calloutParsedCount"), parsedCalloutCount},
        {QStringLiteral("calloutSelfClosingCount"), calloutSelfClosingCount},
        {QStringLiteral("calloutAttributeCount"), calloutAttributeCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildStructuredVerification(
    const QVariantMap& agendaVerification,
    const QVariantMap& calloutVerification,
    const QString& sourceText) const
{
    const QVariantMap normalizedAgendaVerification =
        normalizedVerificationFallback(agendaVerification, QStringLiteral("agenda"));
    const QVariantMap normalizedCalloutVerification =
        normalizedVerificationFallback(calloutVerification, QStringLiteral("callout"));
    const QVariantMap breakVerification = buildBreakVerification(sourceText);
    const QVariantList issues = mergedIssues(
        normalizedAgendaVerification.value(QStringLiteral("issues")).toList(),
        normalizedCalloutVerification.value(QStringLiteral("issues")).toList(),
        breakVerification.value(QStringLiteral("issues")).toList());

    const QString normalizedSourceText = normalizeStructuredSourceText(sourceText);
    return QVariantMap {
        {QStringLiteral("agenda"), normalizedAgendaVerification},
        {QStringLiteral("callout"), normalizedCalloutVerification},
        {QStringLiteral("break"), breakVerification},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("canonicalizationSuggested"), normalizedSourceText != normalizePlainText(sourceText)},
        {QStringLiteral("wellFormed"),
         normalizedAgendaVerification.value(QStringLiteral("wellFormed")).toBool()
             && normalizedCalloutVerification.value(QStringLiteral("wellFormed")).toBool()
             && breakVerification.value(QStringLiteral("wellFormed")).toBool()},
        {QStringLiteral("issues"), issues}
    };
}
